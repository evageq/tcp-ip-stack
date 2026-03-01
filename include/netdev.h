#ifndef NETDEV_H
#define NETDEV_H

#include "eth.h"
#include <net/ethernet.h>
#include <netinet/in.h>

typedef struct netdev_s
{
    struct in_addr in_addr; // network byte order
    mac_t mac;
    bool valid;
} netdev_t;

#define dev_addr in_addr.s_addr

netdev_t netdev_init(const char *addr, const char *hwaddr);
void netdev_receive(const eth_frame_t *frame);

#endif
