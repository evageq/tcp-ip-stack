#include "netdev.h"
#include "arp.h"
#include "ipv4.h"
#include "skb.h"
#include "tuntap.h"
#include "util.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

extern tap_t g_tap;
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
    sscanf("ff:ff:ff:ff:ff:ff", "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &dev.bcast_addr[0], &dev.bcast_addr[1], &dev.bcast_addr[2],
           &dev.bcast_addr[3], &dev.bcast_addr[4], &dev.bcast_addr[5]);

    if (write != ETH_ALEN)
    {
        error("Failed to parse mac addr %s", hwaddr);
        return dev;
    }

    dev.mac_len = ETH_ALEN;
    dev.mac_head_len = sizeof(ethhdr_t);

    return (dev.valid = true, dev);
}

void
netdev_receive(skb_t *skb, netdev_t *host)
{
    skb->dev = host;
    skb->protocol = eth_type(skb);
    skb->mac_head = skb->data;

    switch (skb->protocol)
    {
        case ETH_P_ARP:
        {
            arp_process(host, skb);
            break;
        }
        case ETH_P_IP:
        {
            ip_process(host, skb);
            break;
        }
        default:
        {
            debug("Unknown packet type %d", skb->protocol);
            break;
        }
    }
}

void
netdev_send(skb_t *skb, const mac_t dst, int proto)
{
    const netdev_t *host = skb->dev;
    skb->mac_head = skb->data;

    ethhdr_t *frame = mac_hdr(skb);
    memcpy(frame->dmac, dst, host->mac_len);
    memcpy(frame->smac, host->mac, host->mac_len);
    frame->ether_type = htons(proto);

    tap_write(&g_tap, skb->len, skb->data);
}
