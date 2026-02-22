#ifndef NETDEV_H
#define NETDEV_H

#include <netinet/in.h>
#include <net/ethernet.h>
#include "eth.h"

typedef struct netdev_s
{
    uint32_t addr; // network byte order
    mac_t mac;
    bool valid;
} netdev_t;

netdev_t netdev_init(const char *addr, const char *hwaddr);

#endif
