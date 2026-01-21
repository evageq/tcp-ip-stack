#include <netinet/in.h>
#include <string.h>
#include "arp.h"
#include "tuntap.h"

arp_cache_t g_arp_cache;

arp_record_t *
arp_cache_hit(arp_cache_t cache, const arp_hdr_t *frame)
{
    tll_foreach(cache, it)
    {
        arp_record_t *arp_record = &it->item;
        if (arp_record->ptype == ntohs(frame->ptype)
            && ADDR(frame->spa) == SOCK_ADDR(arp_record->saddr))
        {
            return arp_record;
        }
    }

    return NULL;
}

int arp_cache_merge(arp_cache_t cache, const arp_hdr_t *frame, const tap_t *tap)
{
    arp_record_t arp_record = { 0 };
    arp_record_t *cache_hit = arp_cache_hit(cache, frame);
    if (cache_hit != NULL)
    {
        memcpy(&arp_record, cache_hit, sizeof(arp_record));
    }
    else
    {
        arp_record.hwt = ntohs(frame->htype);
        arp_record.ptype = ntohs(frame->ptype);
        struct sockaddr saddr;
        if (arp_record.hwt == 1)
        {
            struct sockaddr_in sin = {0};
            memcpy(&sin.sin_addr, frame->spa, sizeof(sin.sin_addr));
            memcpy(&saddr, &sin, sizeof(saddr));
        }
        memcpy(&arp_record.saddr, &saddr, sizeof(arp_record.saddr));
    }

    strncpy(arp_record.tif, tap->name, sizeof(arp_record.tif));

    memcpy(&arp_record.hwa, frame->sha, sizeof(arp_record.hwa));

    return 0;
}

int
arp_process(const tap_t *tap, const arp_hdr_t *arp_hdr)
{
    bool is_request = false;
    arp_hdr_t arp_send = {0};
    bool merge_flag;
    if (ntohs(arp_hdr->htype) == 1)
    {
        if (ntohs(arp_hdr->ptype) == 0x0800 && arp_hdr->plen == 4)
        {
            merge_flag = false;
            if (arp_cache_hit(g_arp_cache, arp_hdr))
            {
                merge_flag = true;
                arp_cache_merge(g_arp_cache, arp_hdr, tap);
            }

            // if (ADDR(arp_hdr->tpa) == TAP_ADDR(tap))
            {
                if (merge_flag == false)
                {
                    arp_cache_merge(g_arp_cache, arp_hdr, tap);
                }

                if (ntohs(arp_hdr->oper) == ARP_REQUEST)
                {
                    is_request = true;

                    uint8_t tmp_spa[arp_hdr->plen];
                    uint8_t tmp_sha[arp_hdr->hlen];
                    memcpy(&tmp_spa, arp_hdr->spa, sizeof(tmp_spa));
                    memcpy(&tmp_sha, arp_hdr->sha, sizeof(tmp_sha));
                        

                    memcpy(arp_send.spa, arp_hdr->tpa, sizeof(arp_hdr->spa));
                    memcpy(arp_send.sha, tap->hwaddr.sa_data, sizeof(arp_hdr->sha));
                    memcpy(arp_send.tpa, tmp_spa, sizeof(arp_hdr->tpa));
                    memcpy(arp_send.tha, tmp_sha, sizeof(arp_hdr->tha));

                    arp_send.oper = ntohs(ARP_REPLY);
                }
            }
        }
    }

    if (is_request)
    {
        size_t frame_len = sizeof(eth_frame_t) + sizeof(arp_hdr_t);
        eth_frame_t *frame = malloc(frame_len);
        memcpy(&frame->payload, &arp_send, sizeof(arp_send));
        memcpy(frame->dmac, &arp_send.tha, sizeof(frame->dmac));
        memcpy(frame->smac, &arp_send.sha, sizeof(frame->smac));
        tap_write(tap, frame_len, (uint8_t*)frame);
    }

    return 0;
}
