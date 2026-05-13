#include "udp.h"
#include "skb.h"

static inline struct udp_sock *
udp_sk(struct sock *sk)
{
    return (struct udp_sock *)sk;
}

int
udp_recvmsg(struct sock *sk, struct _msghdr *m, size_t len)
{
    struct udp_sock *up = udp_sk(sk);
}

struct proto udp_prot = { .obj_size = sizeof(struct udp_sock) };
