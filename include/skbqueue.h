#ifndef __SKB_QUEUE_H__
#define __SKB_QUEUE_H__

#include "thread.h"
#include "ringbuf.h"
#include "skb.h"
#include <semaphore.h>

typedef struct skb_queue_s
{
    ring_buffer_t ring;
    sem_t items_sem;
    sem_t slots_sem;
    pthread_mutex_t lock;

} skb_queue_t;

void skb_enqueue(skb_t *skb, skb_queue_t *q);
skb_t *skb_dequeue(skb_queue_t *q);
int skb_queues_init(void);

thread_start_t thread_rx_queue;
thread_start_t thread_tx_queue;

#endif // __SKB_QUEUE_H__
