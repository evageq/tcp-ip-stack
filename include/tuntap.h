#ifndef __TUNTAP__
#define __TUNTAP__

#include <arpa/inet.h>
#include <net/if.h>
#include <stdbool.h>
#include "netdev.h"

typedef struct tap_s
{
    netdev_t netdev;
    int fd;
    char name[IF_NAMESIZE];
    bool valid;
} tap_t;

tap_t tap_init(const char *dev, const char *addr, const char *hwaddr);
int tap_up(const tap_t *tap);
int tap_write(const tap_t *tap, size_t n, uint8_t buf[n]);
int tap_read(const tap_t *tap, size_t n, uint8_t buf[n]);
void tap_close(tap_t *tap);

#endif // __TUNTAP__
