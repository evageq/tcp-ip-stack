#ifndef __DEFS__
#define __DEFS__

#include <stdint.h>
#include <sys/socket.h>

#define DEV_NAME_LEN 128
#define HOST_IP "192.168.1.10"

#define BUF_READ_LEN 4096
#define HOST_ADDR_LEN 128
#define ADDR(x) ((uint32_t)(*(uint32_t*)x))
#define SOCK_ADDR(saddr) (ADDR(&(((struct sockaddr_in*)&saddr)->sin_addr)))
#define TAP_ADDR(tap) (SOCK_ADDR(tap->saddr))

#define error(...) _error(__FILE__, __LINE__, __VA_ARGS__)
#define debug(...) _debug(__FILE__, __LINE__, __VA_ARGS__)

typedef enum arp_codes_e
{
    ARP_REQUEST = 1,
    ARP_REPLY = 2,

} arp_codes_t;

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
    struct sockaddr hwaddr;
    bool valid;
} tap_t;

typedef struct arp_record_s
{
    struct sockaddr saddr;
    uint16_t ptype;
    uint16_t hwt;
    uint8_t hwa[6];
    uint32_t flags;
    uint32_t mask;
    char tif[DEV_NAME_LEN];
} arp_record_t;

#endif // __DEFS__
