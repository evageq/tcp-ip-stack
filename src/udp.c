#include "udp.h"
#include "skb.h"

int
udp4_socket_send(sock_t *sk, const void *buf, size_t len)
{
    skb_t *skb = skb_alloc(L4_HEADROOM + len);
}

int udp4_socket_recv(socket_t *sk, void *buf, size_t len);
int udp4_socket_close(socket_t *sk);
