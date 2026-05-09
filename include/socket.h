#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "tll.h"
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define SOCKET_TBL_MAX 4096

struct sock;
struct socket;
struct sock_ops;
struct proto;
struct proto_ops;
struct _msghdr;

enum family_type_e
{
    _AF_INET,
    _INET_MAX,
};

enum sock_type_e
{
    _SOCK_DGRAM,
    _SOCK_MAX,
};

struct net_proto_family
{
    int family;
    int (*create)(struct socket *sock, int protocol);
};

enum sock_state_e
{
    SS_UNCONNECT,
    SS_MAX,
};

struct socket
{
    int state;
    int family;
    int type;
    int protocol;

    struct sock *sk;
    struct proto_ops *ops; // bsd sock api
    struct sock_ops *sock_ops; // try to mimic to linux file ops

    bool valid;
};

struct sock_ops
{
    ssize_t (*read)(struct socket *, unsigned char *buf, size_t len);
    ssize_t (*write)(struct socket *, const unsigned char *buf, size_t len);
    int (*open)(struct socket *);
    int (*release)(struct socket *);
    // lock?
    // iopoll/poll
};


struct proto_ops
{
    // release
    // bind
    // connect
    // listen
    // sendmsg
    // recvmsg
    // getsockopt
    // setsockopt
    // poll/ioctl?
};

struct _msghdr
{
    // ptr to sockaddr struct
    // data buf
    // data buf len
};

struct _sockaddr
{
};

typedef struct protosw
{
    int type;
    int protocol;
    int flags;
    struct proto *prot;
    struct proto_ops *ops;
} protosw_t;

typedef tll_type(protosw_t, tll_protosw_s) tll_protosw_t;

int sock_register(const struct net_proto_family *proto);

int _socket(int domain, int type, int protocol);
int _bind(int sockfd, const struct _sockaddr *saddr, size_t len);
int _listen(int sockfd);
int _accept(int sockfd);
int _recv(int sockfd, void *buf, size_t len);
int _send(int sockfd, const void *buf, size_t len);
int _recvfrom(int sockfd, void *buf, size_t len, const struct _sockaddr *saddr,
              size_t saddr_len);
int _sendto(int sockfd, const void *buf, size_t len,
            const struct _sockaddr *saddr, size_t saddr_len);
int _close(int sockfd);

#endif // __SOCKET_H__
