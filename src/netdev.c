#include "netdev.h"
#include "util.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>

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
