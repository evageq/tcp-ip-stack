#include "sock.h"

struct sock *
sk_alloc(int family, int protocol)
{
    struct sock *sk = malloc(sizeof(*sk));
    sk->family = family;
    sk->protocol = protocol;
    assert(sk != NULL);
    return sk;
}
