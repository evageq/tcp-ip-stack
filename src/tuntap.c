#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include "tuntap.h"
#include "util.h"

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

tap_t
tap_init(const char *dev, const char *addr, const char *hwaddr)
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

    tap.netdev = netdev_init(addr, hwaddr);

    return (tap.valid = true, tap);
}

void
tap_close(tap_t *tap)
{
    close(tap->fd);
}

