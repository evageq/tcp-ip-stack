#include "skb.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

skb_t *
skb_alloc(int len)
{
    skb_t *skb = malloc(sizeof(*skb));
    memset(skb, 0, sizeof(*skb));
    if (skb == NULL)
    {
        return skb;
    }

    skb->head = skb->data = skb->tail = malloc(len);
    skb->end = skb->head + len;
    if (skb->data == NULL)
    {
        free(skb);
        return NULL;
    }

    skb->len = 0;
    skb->refcnt = 1;

    return skb;
}

void
skb_reserve(skb_t *skb, int len)
{
    assert(skb->data + len <= skb->end);
    skb->data += len;

    assert(skb->tail + len <= skb->end);
    skb->tail += len;
}

void *
skb_push(skb_t *skb, int len)
{

    assert(skb->data - len >= skb->head);
    skb->data -= len;

    skb->len += len;

    return skb->data;
}

void *
skb_put(skb_t *skb, int len)
{
    assert(skb->tail + len <= skb->end);
    skb->tail += len;

    return skb->tail - len;
}

void *
skb_pull(skb_t *skb, int len)
{
    assert(skb->data + len <= skb->end);
    skb->data += len;
    skb->len -= len;

    return skb->data;
}

void
skb_free(skb_t *skb)
{
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
