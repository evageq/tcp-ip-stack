#include "skbqueue.h"
#include "util.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>

int
queue_init(skb_queue_t *q)
{
    int ret = 0;

    memset(q, 0, sizeof(*q));

    ret = sem_init(&q->slots_sem, 0, RING_BUF_MAX);
    if (ret < 0)
    {
        error("Failed to init rx queue slots sem");
        return -1;
    }

    ret = sem_init(&q->items_sem, 0, 0);
    if (ret < 0)
    {
        error("Failed to init rx queue items sem");
        sem_destroy(&q->slots_sem);
        return -1;
    }

    ret = pthread_mutex_init(&q->lock, NULL);
    if (ret != 0)
    {
        errno = ret;
        error("Failed to init queue mutex");
        sem_destroy(&q->items_sem);
        sem_destroy(&q->slots_sem);
        return -1;
    }

    return 0;
}

static inline void
_skb_enqueue(skb_t *skb, skb_queue_t *q)
{
    ring_push(skb, &q->ring);
}

void
skb_enqueue(skb_t *skb, skb_queue_t *q)
{
    sem_wait(&q->slots_sem);
    pthread_mutex_lock(&q->lock);

    _skb_enqueue(skb, q);

    pthread_mutex_unlock(&q->lock);
    sem_post(&q->items_sem);
}

static inline skb_t *
_skb_dequeue(skb_queue_t *q)
{
    return ring_pop(&q->ring);
}

skb_t *
skb_dequeue(skb_queue_t *q)
{
    sem_wait(&q->items_sem);
    pthread_mutex_lock(&q->lock);

    skb_t *skb = _skb_dequeue(q);

    pthread_mutex_unlock(&q->lock);
    sem_post(&q->slots_sem);

    return skb;
}
