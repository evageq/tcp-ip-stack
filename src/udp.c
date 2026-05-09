#include "udp.h"
#include "sock.h"
#include "skb.h"

struct proto udp_prot;

int
udp4_socket_send(struct sock *sk, const void *buf, size_t len)
{
    // skb_t *skb = skb_alloc(L4_HEADROOM + len);
    return 0;
}
