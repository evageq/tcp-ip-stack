#ifndef __NET_H__
#define __NET_H__

typedef int family_t;
struct socket;

enum family_type_e
{
    _AF_INET,
    _INET_MAX,
};

enum sock_type_e
{
    SOCK_MAX,
};

struct net_family
{
    family_t domain;
    int (*create)(struct socket *sock, int protocol);
};

int inet_init(void);

#endif // __NET_H__
