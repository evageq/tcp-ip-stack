#ifndef __IN_H__
#define __IN_H__

#include "sock.h"
#include "socket.h"
#include <stdint.h>

enum ip_protocols_e
{
    _IPPROTO_UDP,
};

struct _sockaddr_in
{
    family_t sfamily;
    uint32_t saddr;
    int sport;
};

struct inet_sock
{
    struct sock sk;
    uint32_t saddr;
    uint32_t daddr;
    uint32_t sport;
    uint32_t dport;
};

int inet_init(void);

#endif // __IN_H__
