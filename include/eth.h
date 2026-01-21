#ifndef __ETH__
#define __ETH__

#include <stdint.h>
#include <net/ethernet.h>
#include <stdbool.h>

typedef struct eth_frame_s
{
    uint8_t dmac[ETH_ALEN];
    uint8_t smac[ETH_ALEN];
    uint16_t ether_type;
    uint8_t payload[];

} __attribute__((packed)) eth_frame_t;

bool is_eth_arp(eth_frame_t *eth);

#endif // __ETH__
