#ifndef __SKB_H__
#define __SKB_H__

#include "sock.h"
#include <stddef.h>

typedef struct netdev_s netdev_t;

typedef struct skb_s
{
    netdev_t *dev;
    unsigned char *head;
    unsigned char *data;
    unsigned char *tail;
    unsigned char *end;

    unsigned char *mac_head;
    unsigned char *network_head;
    unsigned char *transport_head;

    int refcnt;
    int protocol;
    int len;

    struct sock sock;
} skb_t;

skb_t *skb_alloc(size_t len);
void skb_reserve(skb_t *skb, size_t len);
void *skb_push(skb_t *skb, size_t len);
void *skb_put(skb_t *skb, size_t len);
void *skb_pull(skb_t *skb, size_t len);
void skb_free(skb_t *skb);
void *skb_put_data(skb_t *skb, const void *src, size_t len);

#endif // __SKB_H__
