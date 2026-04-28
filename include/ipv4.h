#ifndef __IPV4__
#define __IPV4__

#include "netdev.h"
#include <stdint.h>

#define ICMP_PROTO 1

typedef struct iphdr_s
{
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t flags_foffset;
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
int mask2prefix_len(uint32_t mask);

#endif // __IPV4__
