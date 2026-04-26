#ifndef __RING_BUF_H__
#define __RING_BUF_H__

#include "skb.h"

#define RING_BUF_MAX 512

typedef struct ring_buffer_s
{
    skb_t *skbs[RING_BUF_MAX];
    size_t n;
    int i_w;
    int i_r;

} ring_buffer_t;

void ring_push(skb_t *skb, ring_buffer_t *ring);
skb_t *ring_pop(ring_buffer_t *ring);

#endif // __RING_BUF_H__
