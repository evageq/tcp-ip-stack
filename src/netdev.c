#include "netdev.h"
#include "util.h"
#include "arp.h"
#include "ipv4.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>

extern netdev_t host;

netdev_t
netdev_init(const char *addr, const char *hwaddr)
{
    netdev_t dev = { .valid = false };
    if (inet_pton(AF_INET, addr, &dev.dev_addr) != 1)
    {
        error("Parsing inet address failed\n");
        return dev;
    }
    int write = sscanf(hwaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dev.mac[0],
                       &dev.mac[1], &dev.mac[2], &dev.mac[3], &dev.mac[4],
                       &dev.mac[5]);
    if (write != ETH_ALEN)
    {
        error("Failed to parse mac addr %s", hwaddr);
        return dev;
    }
    return (dev.valid = true, dev);
}

void
netdev_receive(const eth_frame_t* frame)
{
    eth_frame_t *eth_hdr = (eth_frame_t *)frame;
    int e_type = eth_type(eth_hdr);
    switch (e_type)
    {
        case ETH_P_ARP:
        {
            arp_process(&host, (arp_hdr_t *)eth_hdr->payload);
            break;
        }
        case ETH_P_IP:
        {
            ip_process(&host, (ipv4_hdr_t *)eth_hdr->payload);
            break;
        }
        default:
        {
            debug("Unknown eth_type %d", e_type);
            break;
        }
    }
}
