#ifndef __IPV4__
#define __IPV4__

#include "netdev.h"
#include <stdint.h>

#define ICMP_PROTO 1

typedef struct iphdr_s
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

} __attribute__((packed)) iphdr_t;

uint16_t ip_checksum();
int ip_process(skb_t *skb);
iphdr_t *ip_hdr(const skb_t *skb);
uint16_t checksum(void *addr, int count);
int ip_headroom(const netdev_t *dev);
int icmp_process(skb_t *skb);
int ip_send(struct sock *sk, skb_t *skb);

#endif // __IPV4__
