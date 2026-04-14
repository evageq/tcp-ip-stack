#include "skb.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static inline int
_skb_len(const skb_t *skb)
{
    return skb->tail - skb->data;
}

static inline void
_skb_invariant(const skb_t *skb)
{
    assert(skb->head <= skb->data);
    assert(skb->data <= skb->tail);
    assert(skb->tail <= skb->end);
    assert(skb->len == _skb_len(skb));
}

skb_t *
skb_alloc(size_t len)
{
    skb_t *skb = malloc(sizeof(*skb));

    if (skb == NULL)
    {
        error("No memory to allocate skb");
        exit(-1);
    }

    memset(skb, 0, sizeof(*skb));

    void *p = malloc(len);
    if (p == NULL)
    {
        error("Failed to alloc skb data");
        exit(-1);
    }
    skb->head = skb->data = skb->tail = p;
    skb->end = skb->head + len;
    if (skb->data == NULL)
    {
        free(skb);
        return NULL;
    }

    skb->len = _skb_len(skb);
    skb->refcnt = 1;

    _skb_invariant(skb);
    return skb;
}

void
skb_reserve(skb_t *skb, size_t len)
{
    _skb_invariant(skb);
    assert(skb->data + len <= skb->end);
    skb->data += len;

    assert(skb->tail + len <= skb->end);
    skb->tail += len;
    _skb_invariant(skb);
}

void *
skb_push(skb_t *skb, size_t len)
{
    _skb_invariant(skb);

    assert(skb->data - len >= skb->head);
    skb->data -= len;

    skb->len = _skb_len(skb);

    _skb_invariant(skb);
    return skb->data;
}

void *
skb_put(skb_t *skb, size_t len)
{
    _skb_invariant(skb);
    assert(skb->tail + len <= skb->end);
    void *pos = skb->tail;
    skb->tail += len;
    skb->len = _skb_len(skb);

    _skb_invariant(skb);
    return pos;
}

void *
skb_pull(skb_t *skb, size_t len)
{
    _skb_invariant(skb);
    assert(skb->data + len <= skb->tail);
    skb->data += len;
    skb->len = _skb_len(skb);

    _skb_invariant(skb);
    return skb->data;
}

void
skb_free(skb_t *skb)
{
    _skb_invariant(skb);
    if (skb->refcnt <= 1)
    {
        free(skb->head);
        free(skb);
    }
    else
    {
        --skb->refcnt;
    }
}

void *
skb_put_data(skb_t *skb, const void *src, size_t len)
{
    _skb_invariant(skb);
    assert(len >= 0);
    void *dst = skb_put(skb, len);
    memcpy(dst, src, len);
    _skb_invariant(skb);
    return dst;
}
