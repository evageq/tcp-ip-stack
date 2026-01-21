#ifndef __ARP__
#define __ARP__

#include "net/ethernet.h"
#include "defs.h"
#include "tll.h"
#include "eth.h"
#include "tuntap.h"

typedef enum arp_codes_e
{
    ARP_REQUEST = 1,
    ARP_REPLY = 2,

} arp_codes_t;

typedef struct arp_hdr_s
{
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[ETH_ALEN];
    uint8_t spa[4];
    uint8_t tha[ETH_ALEN];
    uint8_t tpa[4];

} __attribute__((packed)) arp_hdr_t;

typedef struct arp_record_s
{
    struct sockaddr saddr;
    uint16_t ptype;
    uint16_t hwt;
    uint8_t hwa[ETH_ALEN];
    uint32_t flags;
    uint32_t mask;
    char tif[IF_NAMESIZE];
} arp_record_t;

typedef tll_type(arp_record_t, arp_cache_s) arp_cache_t;

int arp_cache_merge(arp_cache_t cache, const arp_hdr_t *frame, const tap_t *tap);
arp_record_t *arp_cache_hit(arp_cache_t cache, const arp_hdr_t *arp_hdr);
int arp_process(const tap_t *tap, const arp_hdr_t *arp_hdr);

#endif // __ARP__
