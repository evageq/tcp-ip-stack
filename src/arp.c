#include "arp.h"
#include "tuntap.h"
#include "util.h"
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

arp_cache_t g_arp_cache;

extern tap_t g_tap;

arp_record_t *
arp_cache_hit(const arp_cache_t *cache, const arp_hdr_t *frame)
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
                 const arp_hdr_t *hdr)
{
    arp_record->hwt = ntohs(hdr->htype);
    arp_record->ptype = ntohs(hdr->ptype);
    arp_record->addr = hdr->spa;
    memcpy(&arp_record->hwa, hdr->sha, sizeof(arp_record->hwa));

    return 0;
}

static int
arp_cache_add(arp_cache_t *cache, const arp_hdr_t *hdr)
{
    arp_record_t arp_record;

    arp_cache_update(cache, &arp_record, hdr);
    tll_push_back(*cache, arp_record);

    return 0;
}

int
arp_cache_merge(arp_cache_t *cache, const arp_hdr_t *frame)
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
arp_process(const netdev_t *host, const arp_hdr_t *arp_hdr)
{
    bool is_request = false;
    arp_hdr_t arp_send = { 0 };

    if (ntohs(arp_hdr->htype) == 1)
    {
        if (ntohs(arp_hdr->ptype) == 0x0800 && arp_hdr->plen == 4)
        {
            arp_cache_merge(&g_arp_cache, arp_hdr);

            if (arp_hdr->tpa == host->dev_addr)
            {
                if (ntohs(arp_hdr->oper) == ARP_REQUEST)
                {
                    size_t frame_sz = sizeof(eth_frame_t) + sizeof(arp_hdr_t);
                    eth_frame_t *frame = malloc(frame_sz);

                    memcpy(frame->dmac, arp_hdr->sha, sizeof(mac_t));
                    memcpy(frame->smac, host->mac, sizeof(mac_t));
                    frame->ether_type = ntohs(ETH_P_ARP);

                    arp_hdr_t *arp_ans = (arp_hdr_t *)frame->payload;

                    assert(sizeof(*arp_ans) + sizeof(*frame) == frame_sz);
                    memcpy(arp_ans, arp_hdr, sizeof(*arp_hdr));

                    SWAP(&arp_ans->spa, &arp_ans->tpa);
                    SWAP(&arp_ans->sha, &arp_ans->tha);
                    memcpy((uint8_t*)&arp_ans->sha, (uint8_t*)host->mac, sizeof(host->mac));
                    arp_ans->oper = htons(ARP_REPLY);

                    tap_write(&g_tap, frame_sz, (uint8_t *)frame);
                    free(frame);
                }
            }
        }
    }

    return 0;
}
