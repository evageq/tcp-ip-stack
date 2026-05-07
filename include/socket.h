#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "net.h"
#include <stddef.h>
#include <sys/types.h>

#define SOCKET_TBL_MAX 4096

struct sock;
struct socket_ops;
struct proto_ops;
struct msghdr;

struct socket
{
    int state;
    family_t domain;
    int type;
    int protocol;

    struct sock *sk;
    struct socket_ops *socket_ops;
    struct proto_ops *proto_ops;
};

struct socket_ops
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
    ssize_t (*send)(struct socket *sock, const struct msghdr *m);
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

struct msghdr
{
    // ptr to sockaddr struct
    // data buf
    // data buf len
};

struct _sockaddr
{
};

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
