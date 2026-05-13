#include "in.h"
#include "sock.h"
#include "socket.h"
#include "util.h"
#include <asm-generic/errno-base.h>

extern struct proto udp_prot;
extern struct proto_ops inet_dgram_ops;
extern struct socket socket_tbl[];

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

    struct sock *sk;
    struct proto *prot = NULL;

    tll_foreach(inetsw[sock->type], it)
    {
        if (it->item.protocol == protocol || protocol == 0)
        {
            sock->ops = it->item.ops;
            prot = it->item.prot;
        }
    }

    if (sock->ops == NULL)
    {
        return -1;
    }

    sock->sk = sk_alloc(prot, sock->family, protocol);
    sk = sock->sk;

    return 0;
}

static inline bool
inet_chk_sockaddr(const struct _sockaddr_in *sin1,
                  const struct _sockaddr_in *sin2, size_t saddrlen)
{
    return (sin1->saddr == sin2->saddr && sin1->sport == sin2->sport);
}

static int
__inet_set_sockaddr(struct sock *sk, const struct _sockaddr_in *tsin,
                    size_t saddrlen)
{
    struct _sockaddr_in *ssin = (struct _sockaddr_in *)&sk->sk_addr;

    ssin->saddr = tsin->saddr;
    ssin->sport = tsin->sport;

    return 0;
}

static int
__inet_bind(struct sock *sk, const struct _sockaddr *saddr, size_t saddrlen)
{
    // todo: maintain bind table for checks
    for (int i = 0; i < SOCKET_TBL_MAX; ++i)
    {
        if (socket_tbl[i].family == sk->family)
        {
            if (inet_chk_sockaddr(
                    (struct _sockaddr_in *)&socket_tbl[i].sk->sk_addr,
                    (struct _sockaddr_in *)&sk->sk_addr, saddrlen)
                == true)
            {
                return -1;
            }
        }
    }

    return __inet_set_sockaddr(sk, (struct _sockaddr_in *)saddr, saddrlen);
}

static int
inet_bind_sk(struct sock *sk, const struct _sockaddr *saddr, size_t saddrlen)
{
    if (saddrlen < sizeof(struct _sockaddr_in))
    {
        return -EINVAL;
    }

    return __inet_bind(sk, saddr, saddrlen);
}

static int
inet_bind(struct socket *sock, const struct _sockaddr *saddr, size_t saddrlen)
{
    return inet_bind_sk(sock->sk, saddr, saddrlen);
}

static int
inet_recvmsg(struct socket *sock, struct _msghdr *m, size_t len)
{
    struct sock *sk = sock->sk;
    sk->sk_prot->recvmsg(sk, m, len);

    return 0;
}

struct proto_ops inet_dgram_ops
    = { .bind = inet_bind, .recvmsg = inet_recvmsg };
