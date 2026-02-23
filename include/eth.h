#ifndef __ETH__
#define __ETH__

#include <net/ethernet.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mac_t[ETH_ALEN];

typedef struct eth_frame_s
{
    mac_t dmac;
    mac_t smac;
    uint16_t ether_type;
    uint8_t payload[];

} __attribute__((packed)) eth_frame_t;

int eth_type(const eth_frame_t *frame);
char *mac2str(const mac_t mac, size_t n, char buf[n]);

#endif // __ETH__
