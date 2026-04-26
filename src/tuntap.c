#include "tuntap.h"
#include "util.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int
tap_write(const tap_t *tap, size_t n, uint8_t buf[n])
{
    int bytes_write = write(tap->fd, buf, n);
    if (bytes_write != n)
    {
        error("Failed to send, sent %d, needed %d", bytes_write, n);
    }
    return bytes_write;
}

int
tap_read(const tap_t *tap, size_t n, uint8_t buf[n])
{
    int bytes_read = read(tap->fd, buf, n);
    return bytes_read;
}

int
tap_setaddr(const tap_t *tap)
{
    return SHELL("ip addr add %s dev %s", inet_ntoa(tap->netdev.in_addr),
                 tap->name);
}

int
tap_sethwaddr(const tap_t *tap)
{
    char mac_str[128];
    return SHELL("ip link set %s address %s", tap->name,
                 mac2str(tap->netdev.mac, LENGTH(mac_str), mac_str));
}

tap_t
tap_create(const char *dev, const char *addr, const char *hwaddr)
{
    tap_t tap = { .valid = false };

    char fmt;
    struct ifreq ifr;
    int fd, err;

    if ((tap.fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        error("Failed to open /dev/net/tun");
        return tap;
    }

    ifr = (struct ifreq){ 0 };
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", dev);
    if (ioctl(tap.fd, TUNSETIFF, (void *)&ifr) < 0)
    {
        error("Failed to set tuniff");
        return tap;
    }

    strcpy(tap.name, ifr.ifr_name);

    debug("Created tap: %s", tap.name);

    tap.netdev = netdev_init(addr, 0xffffffff, hwaddr);

    // wait for udev events
    // refacotr to use with libudev
    sleep(1);

    tap_setaddr(&tap);
    tap_sethwaddr(&tap);

    return (tap.valid = true, tap);
}

int
tap_up(const tap_t *tap)
{
    return SHELL("ip link set %s up", tap->name);
}

void
tap_close(tap_t *tap)
{
    close(tap->fd);
}
