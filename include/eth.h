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
void *mac_hdr(skb_t *skb);
int mac_hdr_len(const netdev_t *dev);

#endif // __ETH__
