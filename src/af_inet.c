#include "in.h"
#include "sock.h"
#include "socket.h"
#include "util.h"

extern struct proto udp_prot;
struct proto_ops inet_dgram_ops;

static int inet_create(struct socket *sock, int protocol);

static struct protosw inetsw_array[] = {
    { .type = _SOCK_DGRAM,
     .protocol = _IPPROTO_UDP,
     .prot = &udp_prot,
     .ops = &inet_dgram_ops }
};

tll_protosw_t inetsw[_SOCK_MAX];

struct net_proto_family inet_family_ops
    = { .family = _AF_INET, .create = &inet_create };

int
inet_init()
{
    sock_register(&inet_family_ops);

    for (int i = 0; i < LENGTH(inetsw_array); ++i)
    {
        tll_push_back(inetsw[inetsw_array[i].type], inetsw_array[i]);
    }

    return 0;
}

static int
inet_create(struct socket *sock, int protocol)
{
    assert(protocol != 0); // check for non default value

    sock->sk = sk_alloc(sock->family, protocol);

    tll_foreach(inetsw[sock->type], it)
    {
        if (it->item.protocol == protocol || protocol == 0)
        {
            sock->ops = it->item.ops;
            sock->sk->sk_prot = it->item.prot;
        }
    }

    return (sock->ops ? 0 : -1);
}
