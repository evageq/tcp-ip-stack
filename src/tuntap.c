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
#include "defs.h"

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
tap_up(tap_t *tap)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct ifreq ifr;
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    strcpy(ifr.ifr_name, tap->name);
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr))
    {
        error("Failed to up %s dev", tap->name);
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

bool
is_tap_up(const char *dev)
{
    struct ifreq ifr;
    int socketfd;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    strcpy(ifr.ifr_name, dev);

    if (!(ioctl(socketfd, SIOCGIFFLAGS, &ifr) < 0))
    {
        debug("%d", ifr.ifr_flags & IFF_UP);
    }
    else
    {
        error("Fafiled to ioctl");
        return false;
    }
    return true;
}

int
tap_set_addr(tap_t *tap, const char *host_ip)
{
    int socketfd;
    struct ifreq ifr;
    struct sockaddr_in sin;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);

    sin = (struct sockaddr_in){ 0 };
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.s_addr = inet_addr(host_ip);

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    strcpy(ifr.ifr_name, tap->name);
    if (ioctl(socketfd, SIOCSIFADDR, &ifr) < 0)
    {
        error("Failed to ioctl SIOCSIFADDR");
        close(socketfd);
        return -1;
    }

    strncpy(tap->addr, host_ip, sizeof(tap->addr));
    tap->saddr = ifr.ifr_addr;

    close(socketfd);
    return 0;
}

tap_t
init_tap(char *dev)
{
    tap_t tap = { .valid = false };

    char fmt;
    struct ifreq ifr;
    int fd, err;

    if ((tap.fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        error("Failed to open /dev/net/tun");
    }

    ifr = (struct ifreq){ 0 };
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", dev);
    if (ioctl(tap.fd, TUNSETIFF, (void *)&ifr) < 0)
    {
        error("Failed to set tuniff");
    }

    strcpy(tap.name, ifr.ifr_name);

    return (tap.valid = true, tap);
}

int
set_tap_address(const char *dev)
{
    int socketfd;
    struct ifreq ifr;
    struct sockaddr_in sin;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0)
    {
        error("Failed to create socket");
        return -1;
    }

    sin = (struct sockaddr_in){ 0 };
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.s_addr = inet_addr(HOST_IP);

    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    strcpy(ifr.ifr_name, dev);
    if (ioctl(socketfd, SIOCSIFADDR, &ifr) < 0)
    {
        error("Failed to set tap address");
        close(socketfd);
        return -1;
    }

    close(socketfd);

    return 0;
}

int
set_tap_hwa(tap_t *tap)
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct ifreq ifr = {0};
    // 2a:e9:ea:46:21:73
    uint8_t mac[6];
    mac[0] = 0x2a;
    mac[1] = 0xe9;
    mac[2] = 0xea;
    mac[3] = 0x46;
    mac[4] = 0x21;
    mac[5] = 0x77;

    strcpy(ifr.ifr_name, tap->name);
    memcpy(ifr.ifr_hwaddr.sa_data, mac, sizeof(mac));
    ifr.ifr_hwaddr.sa_family = 1;
    if (ioctl(socketfd, SIOCSIFHWADDR, &ifr) < 0)
    {
        error("failed");
        close(socketfd);
        return -1;
    }
    close(socketfd);
    return 0;
}

int
get_tap_hwa(tap_t *tap)
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct ifreq ifr = {0};

    strcpy(ifr.ifr_name, tap->name);

    if (ioctl(socketfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(socketfd);
        return -1;
    }

    memcpy(&tap->hwaddr, &ifr.ifr_hwaddr, sizeof(tap->hwaddr));
    close(socketfd);
    return 0;
}


void
close_tap(tap_t *tap)
{
    close(tap->fd);
}

