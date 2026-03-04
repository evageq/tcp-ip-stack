#ifndef __IPV4__
#define __IPV4__

#include "netdev.h"
#include <stdint.h>

#define ICMP_PROTO 1

typedef struct ipv4_hdr_s
{
    uint8_t version : 4;
    uint8_t ihl : 4;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint8_t flags : 3;
    uint16_t frag_offset : 13;
    uint8_t ttl;
    uint8_t proto;
    uint16_t csum;
    uint32_t saddr;
    uint32_t daddr;

} __attribute__((packed)) ipv4_hdr_t;

uint16_t ip_checksum();
int ip_process(const netdev_t *dev, skb_t *skb);
ipv4_hdr_t *ip_hdr(const skb_t *skb);

#endif // __IPV4__
