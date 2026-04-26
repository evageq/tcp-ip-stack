#ifndef __ARP__
#define __ARP__

#include "eth.h"
#include "netdev.h"
#include "tll.h"

#define ARP_CACHE_INIT() tll_init()

typedef enum arp_codes_e
{
    ARP_REQUEST = 1,
    ARP_REPLY = 2,

} arp_codes_t;

typedef struct arphdr_s
{
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    mac_t sha;
    uint32_t spa;
    mac_t tha;
    uint32_t tpa;

} __attribute__((packed)) arphdr_t;

typedef struct arp_record_s
{
    uint32_t addr;
    uint16_t ptype;
    uint16_t hwt;
    mac_t hwa;
    uint32_t flags;
    uint32_t mask;
    netdev_t *dev;
} arp_record_t;

typedef tll_type(arp_record_t, arp_cache_s) arp_cache_t;

int arp_cache_update(arp_record_t *arp_record, netdev_t *dev,
                     const arphdr_t *frame);
int arp_cache_merge(netdev_t *dev, const arphdr_t *frame);
arp_record_t *arp_cache_hit(const arphdr_t *arp_hdr);
int arp_process(skb_t *skb);
arphdr_t *arp_hdr(const skb_t *skb);
const mac_t *arp_get_hw_addr(uint32_t addr);
void arp_request(netdev_t *dev, uint32_t addr);
int arp_hdr_len(const netdev_t *dev);

#endif // __ARP__
