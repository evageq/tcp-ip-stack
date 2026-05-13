#include "sock.h"

struct sock *
sk_alloc(struct proto *prot, int family, int protocol)
{
    struct sock *sk = malloc(prot->obj_size);

    sk->sk_prot = prot;
    sk->protocol = protocol;

    assert(sk != NULL);
    return sk;
}
