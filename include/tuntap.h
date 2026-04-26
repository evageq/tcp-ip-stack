#ifndef __TUNTAP__
#define __TUNTAP__

#include "netdev.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <stdbool.h>

#define PKT_BUF_SIZE 2048

typedef struct tap_s
{
    netdev_t netdev;
    int fd;
    char name[IF_NAMESIZE];
    bool valid;
} tap_t;

tap_t tap_create(const char *dev, const char *addr, const char *hwaddr);
int tap_up(const tap_t *tap);
int tap_setaddr(const tap_t *tap);
int tap_sethwaddr(const tap_t *tap);
int tap_write(const tap_t *tap, size_t n, uint8_t buf[n]);
int tap_read(const tap_t *tap, size_t n, uint8_t buf[n]);
void tap_close(tap_t *tap);

#endif // __TUNTAP__
