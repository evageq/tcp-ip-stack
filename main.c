#include "defs.h"
#include "tll.h"
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

tll(arp_record_t) arp_cache_ll;

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
tap_write(tap_t *tap, size_t n, uint8_t buf[n])
{
    int bytes_write = write(tap->fd, buf, n);
    if (bytes_write != n)
    {
        error("Failed to send, sent %d, needed %d", bytes_write, n);
    }
    return bytes_write;
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

arp_record_t *
arp_cache_hit(arp_frame_t *arp_hdr)
{
    tll_foreach(arp_cache_ll, arp_rec)
    {
        if (arp_rec->item.ptype == ntohs(arp_hdr->ptype)
            && ADDR(arp_hdr->spa) == SOCK_ADDR(arp_rec->item.saddr))
        {
            return &arp_rec->item;
        }
    }

    return NULL;
}

void
merge_arp_cache(tap_t *tap, arp_frame_t *arp_hdr)
{
    arp_record_t arp_rec = { 0 };
    arp_record_t *cache = arp_cache_hit(arp_hdr);
    if (cache != NULL)
    {
        memcpy(&arp_rec, cache, sizeof(arp_rec));
    }
    else
    {
        arp_rec.hwt = ntohs(arp_hdr->htype);
        arp_rec.ptype = ntohs(arp_hdr->ptype);
        struct sockaddr saddr;
        if (arp_rec.hwt == 1)
        {
            struct sockaddr_in sin = {0};
            memcpy(&sin.sin_addr, arp_hdr->spa, sizeof(sin.sin_addr));
            memcpy(&saddr, &sin, sizeof(saddr));
        }
        memcpy(&arp_rec.saddr, &saddr, sizeof(arp_rec.saddr));
    }

    strcpy(arp_rec.tif, tap->name);

    memcpy(&arp_rec.hwa, arp_hdr->sha, sizeof(arp_rec.hwa));
}

inline bool
cmp_hw_addr(uint8_t hw1[6], uint8_t hw2[6])
{
    bool flag = true;
    for (int i = 0; i < 6; ++i)
    {
        flag &= hw1[i] == hw2[i];
    }

    return flag;
}

int
process_arp(tap_t *tap, arp_frame_t *arp_hdr)
{
    bool merge_flag;
    if (ntohs(arp_hdr->htype) == 1)
    {
        if (ntohs(arp_hdr->ptype) == 0x0800 && arp_hdr->plen == 4)
        {
            merge_flag = false;
            if (arp_cache_hit(arp_hdr))
            {
                merge_flag = true;
                merge_arp_cache(tap, arp_hdr);
            }

            // if (ADDR(arp_hdr->tpa) == TAP_ADDR(tap))
            {
                if (merge_flag == false)
                {
                    merge_arp_cache(tap, arp_hdr);
                }

                if (ntohs(arp_hdr->oper) == ARP_REQUEST)
                {
                    uint8_t tmp_spa[arp_hdr->plen];
                    uint8_t tmp_sha[arp_hdr->hlen];
                    memcpy(&tmp_spa, arp_hdr->spa, sizeof(tmp_spa));
                    memcpy(&tmp_sha, arp_hdr->sha, sizeof(tmp_sha));
                        

                    memcpy(arp_hdr->spa, arp_hdr->tpa, sizeof(arp_hdr->spa));
                    memcpy(arp_hdr->sha, tap->hwaddr.sa_data, sizeof(arp_hdr->sha));
                    memcpy(arp_hdr->tpa, tmp_spa, sizeof(arp_hdr->tpa));
                    memcpy(arp_hdr->tha, tmp_sha, sizeof(arp_hdr->tha));

                    arp_hdr->oper = ntohs(ARP_REPLY);
                }
            }
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
    // https://stackoverflow.com/questions/79758511/get-tap-device-mac-address
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
    set_tap_hwa(&tap);
    get_tap_hwa(&tap);

    while (1)
    {
        uint8_t tmp_buf[BUF_READ_LEN];
        int bytes_read = tap_read(&tap, BUF_READ_LEN, tmp_buf);

        if (bytes_read < 0)
        {
            error("Failed to tap_read");
        }

        eth_frame_t *eth_hdr = (eth_frame_t *)tmp_buf;
        if (is_eth_arp(eth_hdr))
        {
            bool arp_request = ntohs(((arp_frame_t*)eth_hdr->payload)->oper) == ARP_REQUEST;
            process_arp(&tap, (arp_frame_t *)eth_hdr->payload);
            if (arp_request == true)
            {
                arp_frame_t ans_arp;
                memcpy(&ans_arp, eth_hdr->payload, sizeof(ans_arp));

                memcpy(eth_hdr->dmac, ((arp_frame_t*)eth_hdr->payload)->tha, sizeof(eth_hdr->dmac));
                memcpy(eth_hdr->smac, ((arp_frame_t*)eth_hdr->payload)->sha, sizeof(eth_hdr->smac));
                tap_write(&tap, sizeof(eth_frame_t) + sizeof(arp_frame_t), (uint8_t*)eth_hdr);
            }
        }
    }

    close_tap(&tap);

    fflush(stdout);
    fflush(stderr);
    getc(stdin);

    return 0;
}
