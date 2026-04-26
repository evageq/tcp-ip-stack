#include "arp.h"
#include "tuntap.h"
#include "util.h"
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

static arp_cache_t g_arp_cache = ARP_CACHE_INIT();

extern tap_t g_tap;

inline int
arp_hdr_len(const netdev_t *dev)
{
    return sizeof(arphdr_t);
}

static inline void
arp_send(skb_t *skb, const mac_t dst)
{
    netdev_send(skb, dst, ETH_P_ARP);
}

static skb_t *
arp_make(netdev_t *dev, const unsigned char *spa, const unsigned char *tpa,
         const unsigned char *sha, const unsigned char *tha, int opcode,
         int htype, int ptype)
{
    skb_t *skb = skb_alloc(arp_hdr_len(dev) + mac_hdr_len(dev));
    skb_reserve(skb, arp_hdr_len(dev) + mac_hdr_len(dev));
    skb->out_dev = dev;
    skb->protocol = htons(ETH_P_ARP); // FIXME ntohs or htons
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

void
arp_request(netdev_t *dev, uint32_t addr)
{
    skb_t *skb = arp_make(dev, (unsigned char *)&dev->dev_addr,
                          (unsigned char *)&addr, (unsigned char *)&dev->mac,
                          NULL, ARP_REQUEST, 1, 0x0800);
    arp_send(skb, dev->bcast_addr);
    skb_free(skb);
}

const mac_t *
arp_get_hw_addr(uint32_t addr)
{
    tll_foreach(g_arp_cache, it)
    {
        if (it->item.addr == addr)
        {
            return &it->item.hwa;
        }
    }

    return NULL;
}

inline arphdr_t *
arp_hdr(const skb_t *skb)
{
    return (arphdr_t *)skb->network_head;
}

arp_record_t *
arp_cache_hit(const arphdr_t *frame)
{
    tll_foreach(g_arp_cache, it)
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
arp_cache_update(arp_record_t *arp_record, netdev_t *dev, const arphdr_t *hdr)
{
    arp_record->hwt = ntohs(hdr->htype);
    arp_record->ptype = ntohs(hdr->ptype);
    arp_record->addr = hdr->spa;
    arp_record->dev = dev;
    memcpy(&arp_record->hwa, hdr->sha, sizeof(arp_record->hwa));

    return 0;
}

static int
arp_cache_add(netdev_t *dev, const arphdr_t *hdr)
{
    arp_record_t arp_record;

    arp_cache_update(&arp_record, dev, hdr);
    tll_push_back(g_arp_cache, arp_record);

    return 0;
}

int
arp_cache_merge(netdev_t *dev, const arphdr_t *frame)
{
    arp_record_t arp_record = { 0 };
    arp_record_t *cache_hit = arp_cache_hit(frame);
    int ret = 0;

    if (cache_hit != NULL)
    {
        ret = arp_cache_update(cache_hit, dev, frame);
    }
    else
    {
        ret = arp_cache_add(dev, frame);
    }

    return ret;
}

int
arp_process(skb_t *skb)
{
    netdev_t *host = skb->in_dev;
    if (skb->len < (int)sizeof(arphdr_t))
    {
        debug("Short ARP packet, len %d", skb->len);
        return -1;
    }

    const arphdr_t *arp_head = arp_hdr(skb);

    if (ntohs(arp_head->htype) == 1)
    {
        if (ntohs(arp_head->ptype) == 0x0800 && arp_head->plen == 4)
        {
            arp_cache_merge(skb->in_dev, arp_head);

            if (arp_head->tpa == host->dev_addr)
            {
                if (ntohs(arp_head->oper) == ARP_REQUEST)
                {
                    skb_t *skb;
                    skb = arp_make(host, (unsigned char *)&host->dev_addr,
                                   (unsigned char *)&arp_head->spa,
                                   (unsigned char *)&host->mac, arp_head->sha,
                                   ARP_REPLY, ntohs(arp_head->htype),
                                   ntohs(arp_head->ptype));
                    arp_send(skb, arp_head->sha);
                    skb_free(skb);
                }
            }
        }
    }

    return 0;
}
