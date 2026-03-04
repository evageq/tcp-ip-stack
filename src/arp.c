#include "arp.h"
#include "tuntap.h"
#include "util.h"
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

arp_cache_t g_arp_cache;

extern tap_t g_tap;

inline arphdr_t *
arp_hdr(const skb_t *skb)
{
    return (arphdr_t *)(skb->mac_head + skb->dev->mac_head_len);
}

arp_record_t *
arp_cache_hit(const arp_cache_t *cache, const arphdr_t *frame)
{
    tll_foreach(*cache, it)
    {
        arp_record_t *arp_record = &it->item;
        if (arp_record->ptype == ntohs(frame->ptype)
            && frame->spa == arp_record->addr)
        {
            return arp_record;
        }
    }

    return NULL;
}

int
arp_cache_update(arp_cache_t *cache, arp_record_t *arp_record,
                 const arphdr_t *hdr)
{
    arp_record->hwt = ntohs(hdr->htype);
    arp_record->ptype = ntohs(hdr->ptype);
    arp_record->addr = hdr->spa;
    memcpy(&arp_record->hwa, hdr->sha, sizeof(arp_record->hwa));

    return 0;
}

static int
arp_cache_add(arp_cache_t *cache, const arphdr_t *hdr)
{
    arp_record_t arp_record;

    arp_cache_update(cache, &arp_record, hdr);
    tll_push_back(*cache, arp_record);

    return 0;
}

int
arp_cache_merge(arp_cache_t *cache, const arphdr_t *frame)
{
    arp_record_t arp_record = { 0 };
    arp_record_t *cache_hit = arp_cache_hit(cache, frame);
    int ret = 0;

    if (cache_hit != NULL)
    {
        ret = arp_cache_update(cache, cache_hit, frame);
    }
    else
    {
        ret = arp_cache_add(cache, frame);
    }

    return ret;
}

int
arp_hdr_len(const netdev_t *dev)
{
    return sizeof(arphdr_t);
}

static skb_t *
arp_create(const netdev_t *dev, const unsigned char *spa,
           const unsigned char *tpa, const unsigned char *sha,
           const unsigned char *tha, int opcode, int htype, int ptype)
{
    skb_t *skb = skb_alloc(arp_hdr_len(dev) + mac_hdr_len(dev));
    skb_reserve(skb, arp_hdr_len(dev) + mac_hdr_len(dev));
    skb->dev = (netdev_t *)dev;
    skb->protocol = ntohs(ETH_P_ARP);
    arphdr_t *arp = skb_push(skb, arp_hdr_len(dev));
    if (spa == NULL)
    {
        memcpy(&arp->spa, &dev->dev_addr, sizeof(dev->dev_addr));
    }
    else
    {
        memcpy(&arp->spa, spa, sizeof(dev->dev_addr));
    }
    if (tha == NULL)
    {
        memcpy(&arp->tha, dev->bcast_addr, dev->mac_len);
    }
    else
    {
        memcpy(&arp->tha, tha, dev->mac_len);
    }

    memcpy(&arp->tpa, tpa, sizeof(dev->dev_addr));
    memcpy(&arp->sha, &dev->mac, dev->mac_len);

    arp->oper = htons(opcode);
    arp->htype = htons(htype);
    arp->ptype = htons(ptype);

    arp->hlen = dev->mac_len;
    arp->plen = sizeof(dev->dev_addr);

    return skb;
}

static void
arp_send(skb_t *skb, const mac_t dst)
{
    skb_push(skb, skb->dev->mac_head_len);
    netdev_send(skb, dst, ETH_P_ARP);
}

int
arp_process(const netdev_t *host, skb_t *skb)
{
    bool is_request = false;
    const arphdr_t *arp_head = arp_hdr(skb);

    if (ntohs(arp_head->htype) == 1)
    {
        if (ntohs(arp_head->ptype) == 0x0800 && arp_head->plen == 4)
        {
            arp_cache_merge(&g_arp_cache, arp_head);

            if (arp_head->tpa == host->dev_addr)
            {
                if (ntohs(arp_head->oper) == ARP_REQUEST)
                {
                    skb_t *skb;
                    skb = arp_create(
                        host, (unsigned char *)&host->dev_addr,
                        (unsigned char *)&arp_head->spa,
                        (unsigned char *)&host->mac, arp_head->sha, ARP_REPLY,
                        ntohs(arp_head->htype), ntohs(arp_head->ptype));
                    arp_send(skb, arp_head->sha);
                    skb_free(skb);
                }
            }
        }
    }

    return 0;
}
