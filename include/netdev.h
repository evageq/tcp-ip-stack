#ifndef __NETDEV_H__
#define __NETDEV_H__

#include "eth.h"
#include <net/ethernet.h>
#include <netinet/in.h>

typedef struct netdev_s
{
    struct in_addr in_addr; // network byte order
    mac_t mac;
    mac_t bcast_addr;
    int mac_head_len;
    int mac_len;
    bool valid;
} netdev_t;

#define dev_addr in_addr.s_addr

netdev_t netdev_init(const char *addr, const char *hwaddr);
void netdev_receive(skb_t *skb, netdev_t *host);
int mac_hdr_len(const netdev_t *dev);
void netdev_send(skb_t *skb, const mac_t dst, int proto);

#endif // __NETDEV_H__
