#include "ringbuf.h"
#include "util.h"
#include <assert.h>

void
ring_push(skb_t *skb, ring_buffer_t *ring)
{
    size_t cap = LENGTH(ring->skbs);
    assert(ring->n <= cap);

    int i_w = ring->i_w;
    ring->skbs[i_w] = skb;
    ring->i_w = (i_w + 1) % cap;
    ++ring->n;
}

skb_t *
ring_pop(ring_buffer_t *ring)
{
    size_t cap = LENGTH(ring->skbs);
    assert(ring->n <= cap);
    assert(ring->n > 0);

    int i_r = ring->i_r;
    skb_t *skb = ring->skbs[i_r];
    ring->i_r = (i_r + 1) % cap;
    --ring->n;

    return skb;
}
