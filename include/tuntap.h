#ifndef __TUNTAP__
#define __TUNTAP__

#include <arpa/inet.h>
#include <net/if.h>
#include <stdbool.h>
#include "defs.h"

typedef struct tap_s
{
    int fd;
    char name[IF_NAMESIZE];
    char addr[128];
    struct sockaddr saddr;
    struct sockaddr hwaddr;
    bool valid;
} tap_t;

tap_t init_tap(char *dev);
int tap_write(const tap_t *tap, size_t n, uint8_t buf[n]);
int tap_read(const tap_t *tap, size_t n, uint8_t buf[n]);
int tap_up(tap_t *tap);
bool is_tap_up(const char *dev);
int tap_set_addr(tap_t *tap, const char *host_ip);
int set_tap_address(const char *dev);
int set_tap_hwa(tap_t *tap);
void close_tap(tap_t *tap);
int get_tap_hwa(tap_t *tap);

#endif // __TUNTAP__
