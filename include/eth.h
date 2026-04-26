#ifndef __ETH__
#define __ETH__

#include "skb.h"
#include <net/ethernet.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mac_t[ETH_ALEN];

typedef struct ethhdr_s
{
    mac_t dmac;
    mac_t smac;
    uint16_t ether_type;
    uint8_t payload[];

} __attribute__((packed)) ethhdr_t;

int eth_type(const skb_t *skb);
char *mac2str(const mac_t mac, size_t n, char buf[n]);
ethhdr_t *mac_hdr(skb_t *skb);
int mac_hdr_len(const netdev_t *dev);
int mac_equal(const mac_t *a, const mac_t *b);
bool mac_recv_check_host(skb_t *skb);

#endif // __ETH__
