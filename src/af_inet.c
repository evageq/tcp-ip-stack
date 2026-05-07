#include "net.h"

static int inet_create(struct socket *sock, int protocol);

struct net_family af_inet = { .domain = _AF_INET, .create = &inet_create };

static int
inet_create(struct socket *sock, int protocol)
{
}
