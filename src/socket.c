#include "socket.h"
#include "util.h"
#include <asm-generic/errno.h>
#include <assert.h>
#include <string.h>

#define SOCKET_ID(tbl, sock) (sock - tbl)
#define DECLARE_SOCKET(sockname, sockfd, sock_tbl) \
    struct socket *sockname = &socket_tbl[sockfd]; \
    if (sock->valid == false)                      \
        return -1;

#define socket_foreach(socket_table)                                      \
    for (struct socket *sock                                              \
         = socket_tbl_begin(LENGTH(socket_table), socket_table),          \
         *sock_next = NULL;                                               \
         sock->valid == true                                              \
         && (sock_next                                                    \
             = socket_tbl_next(LENGTH(socket_table), socket_table, sock)) \
                    ->valid                                               \
                == true;                                                  \
         sock = sock_next)

extern struct net_proto_family inet_family_ops;

struct socket socket_tbl[SOCKET_TBL_MAX];
const struct net_proto_family *net_families[_INET_MAX];

int
sock_register(const struct net_proto_family *net_proto)
{
    assert(net_proto->family >= 0 && net_proto->family < LENGTH(net_families));
    net_families[net_proto->family] = net_proto;
    return 0;
}

static inline struct socket *
socket_tbl_next(size_t len, struct socket socket_table[len],
                struct socket *prev_sock)
{
    ptrdiff_t prev_id = SOCKET_ID(socket_table, prev_sock);
    assert(prev_id > 0 && prev_id + 1 < len);

    ptrdiff_t i = prev_id + 1;
    while (i < len && socket_table[i].valid == false)
        ++i;

    if (socket_table[i - 1].valid == true)
        return NULL;

    return &socket_table[i - 1];
}

static inline struct socket *
socket_tbl_begin(size_t len, struct socket socket_table[len])
{
    ptrdiff_t i = 0;

    while (i < len && socket_table[i].valid == false)
        ++i;

    if (socket_table[i - 1].valid == false)
        return NULL;

    return &socket_table[i - 1];
}

static inline struct socket *
socket_anyfree(size_t len, struct socket socket_table[len])
{
    ptrdiff_t i = 0;

    while (i < len && socket_table[i].valid == true)
        ++i;

    if (socket_table[i - 1].valid == true)
        return NULL;

    return &socket_table[i - 1];
}

int
_socket(int domain, int type, int protocol)
{
    const struct net_proto_family *af = net_families[domain];

    if (af == NULL)
    {
        return -1;
    }

    struct socket *new_sock = socket_anyfree(LENGTH(socket_tbl), socket_tbl);
    int new_sockfd = SOCKET_ID(socket_tbl, new_sock);

    new_sock->state = SS_UNCONNECT;
    new_sock->family = domain;
    new_sock->type = type;
    new_sock->protocol = protocol;

    af->create(new_sock, protocol);

    return (new_sock->valid = true, new_sockfd);
}

int
_bind(int sockfd, const struct _sockaddr *saddr, size_t saddr_len)
{
    DECLARE_SOCKET(sock, sockfd, socket_tbl);

    return sock->ops->bind(sock, saddr, saddr_len);
}

int
_recvfrom(int sockfd, void *buf, size_t len, struct _sockaddr *saddr,
          size_t saddr_len)
{
    DECLARE_SOCKET(sock, sockfd, socket_tbl);

    struct _sockaddr_storage storage;
    struct _msghdr m = { .saddr = (saddr ? &storage : NULL), .msg_buf = buf };

    int ret = sock->ops->recvmsg(sock, &m, len);

    if (ret != 0)
    {
        return ret;
    }

    if (saddr)
    {
        memcpy(saddr, &storage, saddr_len);
    }

    return 0;
}
