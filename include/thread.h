#ifndef __THREAD_H__
#define __THREAD_H__

typedef void *(thread_start_t)(void *);
typedef void (thread_init_t)(void);
enum thread_types_e
{
    THREAD_CORE,
    THREAD_RX_QUEUE,
    THREAD_TX_QUEUE,
    THREAD_MAX,
};

#endif // __THREAD_H__
