#include "skbqueue.h"
#include "tuntap.h"
#include "util.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>

extern tap_t g_tap;
extern netdev_t host;

skb_queue_t rxq;
skb_queue_t txq;

static int
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

int
skb_queues_init(void)
{
    if (queue_init(&rxq) < 0)
    {
        return -1;
    }

    if (queue_init(&txq) < 0)
    {
        pthread_mutex_destroy(&rxq.lock);
        sem_destroy(&rxq.items_sem);
        sem_destroy(&rxq.slots_sem);
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

void *
thread_rx_queue(void *arg)
{
    while (true)
    {
        int ret = 0;
        uint8_t buf[PKT_BUF_SIZE];
        int bytes_read = tap_read(&g_tap, LENGTH(buf), buf);
        if (bytes_read < 0)
        {
            error("Failed tap_read");
            continue;
        }

        skb_t *skb = skb_alloc(bytes_read);
        skb_put_data(skb, buf, bytes_read);
        skb_enqueue(skb, &rxq);
    }

    return 0;
}

void *
thread_tx_queue(void *arg)
{
    while (true)
    {
        skb_t *skb = skb_dequeue(&txq);
        tap_write(&g_tap, skb->len, skb->data);
        skb_free(skb);
    }

    return 0;
}
