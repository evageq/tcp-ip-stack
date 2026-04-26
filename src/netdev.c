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
netdev_init(const char *addr, uint32_t netmask, const char *hwaddr)
{
    netdev_t dev = { .valid = false };
    if (inet_pton(AF_INET, addr, &dev.dev_addr) != 1)
    {
        error("Parsing inet address failed\n");
        return dev;
    }
    dev.netmask = netmask;

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

int
netdev_receive(skb_t *skb, netdev_t *host)
{
    skb->in_dev = host;
    skb->mac_head = skb->data;

    if (skb->len < host->mac_head_len)
    {
        debug("Short ethernet frame, len %d", skb->len);
        return -1;
    }

    // check is packet mine
    if (mac_recv_check_host(skb) == true)
    {
        print_hex_packet(SKB_CAP(skb), skb->head, SKB_CAP(skb));
    }
    else
    {
        // drop packet
        return -1;
    }

    skb->protocol = eth_type(skb);
    skb_pull(skb, host->mac_head_len);
    skb->network_head = skb->data;

    switch (skb->protocol)
    {
        case ETH_P_ARP:
        {
            return arp_process(skb);
        }
        case ETH_P_IP:
        {
            return ip_process(skb);
        }
        default:
        {
            debug("Unknown packet type %d", skb->protocol);
            return -1;
        }
    }
}

void
netdev_send(skb_t *skb, const mac_t dst, int ether_type)
{
    const netdev_t *dev = skb->out_dev;
    skb_push(skb, dev->mac_head_len);

    skb->mac_head = skb->data;

    ethhdr_t *frame = mac_hdr(skb);
    memcpy(frame->dmac, dst, dev->mac_len);
    memcpy(frame->smac, dev->mac, dev->mac_len);
    frame->ether_type = htons(ether_type);

    tap_write(&g_tap, skb->len, skb->data);
}
