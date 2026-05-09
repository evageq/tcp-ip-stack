#ifndef __SOCK_H__
#define __SOCK_H__

#include "skbqueue.h"
#include "socket.h"
#include <stdint.h>

struct proto;

struct sock
{
    int family;
    int type;
    int protocol;

    uint32_t daddr;
    uint32_t saddr;
    uint16_t sport;
    uint16_t dport;

    struct socket *sk_socket;
    skb_queue_t rxq;

    struct proto *sk_prot; // protocol handler
};

struct proto
{
    // init
    // connect
    // sendmsg
    // recvmsg
    // accept
    // bind
    // release
    // get/set sockopt
    // close
};

struct sock *sk_alloc(int family, int protocol);

#endif
