#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEV_NAME_LEN 128
#define HOST_IP "192.168.1.10"

#define error(...) _error(__FILE__, __LINE__, __VA_ARGS__)
#define debug(...) _debug(__FILE__, __LINE__, __VA_ARGS__)

typedef struct eth_frame_s
{
    uint8_t dmac[6];
    uint8_t smac[6];
    uint16_t ether_type;
    uint8_t payload[];

} __attribute__((packed)) eth_frame_t;

typedef struct arp_frame_s
{
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];
    uint8_t spa[4];
    uint8_t tha[6];
    uint8_t tpa[4];

} __attribute__((packed)) arp_frame_t;

typedef struct tap_s
{
    int fd;
    char name[DEV_NAME_LEN];
    char addr[128];
    struct sockaddr saddr;
    bool valid;
} tap_t;

void
_error(const char *f, int line, const char *fmt, ...)
{
    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4096, fmt, args);
    va_end(args);
    fprintf(stderr, "[%s:%d][ERROR]: %s: %s\n", f, line, buf, strerror(errno));
}

void
_debug(const char *f, int line, const char *fmt, ...)
{
    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4096, fmt, args);
    va_end(args);
    fprintf(stderr, "[%s:%d][DEBUG]: %s\n", f, line, buf);
}

int
tap_read(tap_t *tap, size_t n, uint8_t buf[n])
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

void
print_hex_packet(size_t n, const uint8_t buf[n], int bytes_read)
{
    printf("Packet:\n");
    for (int i = 0; i < bytes_read; ++i)
    {
        if (i % 8 == 0)
        {
            printf("\n\t");
        }
        printf("%02x ", buf[i]);
    }
    printf("\n");
    fflush(stdout);
}

static inline bool
is_eth_arp(eth_frame_t *eth)
{
    if (ntohs(eth->ether_type) == ETH_P_ARP)
    {
        return true;
    }

    return false;
}

int
process_arp(arp_frame_t *arp_hdr)
{
    if (ntohs(arp_hdr->htype) == 1)
    {
        if (ntohs(arp_hdr->ptype) == 0x0800 && arp_hdr->plen == 4)
        {
            char net_ip[128] = { 0 };
            struct in_addr saddr;
            saddr.s_addr = (uint32_t)(*(uint32_t *)(arp_hdr->tpa));
            strcpy(net_ip, inet_ntoa(saddr));
            if (strncmp(HOST_IP, net_ip, sizeof(HOST_IP)) == 0)
            {
                debug("addres of mine");
            }
            // debug("%s", net_ip);
        }
    }
    return 0;
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
    }

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

void
close_tap(tap_t *tap)
{
    close(tap->fd);
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

int
main()
{
    tap_t tap = init_tap("tap%d");
    if (tap.valid == false)
    {
        error("Failed to init tap");
        exit(-1);
    }

    if (tap_set_addr(&tap, HOST_IP) != 0)
    {
        error("Failed to set tap address");
    }

    if (tap_up(&tap) != 0)
    {
        error("Failed to up tap dervice");
    }

    while (1)
    {
        uint8_t tmp_buf[4096];
        int bytes_read = tap_read(&tap, 4096, tmp_buf);

        if (bytes_read < 0)
        {
            error("Failed to tap_read");
        }

        eth_frame_t *eth_hdr = (eth_frame_t *)tmp_buf;
        if (is_eth_arp(eth_hdr))
        {
            process_arp((arp_frame_t *)eth_hdr->payload);
        }
    }

    close_tap(&tap);

    fflush(stdout);
    fflush(stderr);
    getc(stdin);

    return 0;
}
