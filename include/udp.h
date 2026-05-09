#ifndef __UDP_H__
#define __UDP_H__

/*
 * rfc 791 interface for L4
 */

#include "socket.h"
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

int udp_send(const void *buf, size_t len);
int udp4_socket_send(struct sock *sk, const void *buf, size_t len);
int udp4_socket_recv(struct sock *sk, void *buf, size_t len);
int udp4_socket_close(struct sock *sk);

#endif // __UDP_H__
