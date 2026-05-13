#ifndef __UDP_H__
#define __UDP_H__

/*
 * rfc 791 interface for L4
 */

#include "socket.h"
#include "sock.h"
#include "in.h"
#include <stddef.h>
#include <stdint.h>

typedef struct udphdr_s
{
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t csum;
    uint8_t payload[];

} __attribute__((packed)) udphdr_t;

struct udp_sock
{
    struct inet_sock inet;
};

int udp_recvmsg(struct sock *sk, struct _msghdr *m, size_t len);

#endif // __UDP_H__
