#include "eth.h"
#include "skb.h"
#include "skbqueue.h"
#include "thread.h"
#include "tuntap.h"
#include "util.h"
#include <arpa/inet.h>
#include <assert.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

extern skb_queue_t rxq;
extern skb_queue_t txq;

tap_t g_tap;
bool SHELL_DEBUG = true;
netdev_t host;

pthread_t threads[THREAD_MAX];

static int
net_init()
{
    // https://stackoverflow.com/questions/79758511/get-tap-device-mac-address
    g_tap = tap_create("tap%d", "192.168.17.9", "2a:e9:ea:46:21:70");
    if (g_tap.valid == false)
    {
        error("Failed to init tap %s", g_tap.name);
        return -1;
    }
    else
    {
        debug("tap %s valid", g_tap.name);
    }

    host = netdev_init("10.0.0.4", 0xffffff00, "2a:e9:ea:46:21:71");

    tap_up(&g_tap);
    return 0;
}

static int
stack_init()
{
    net_init();
    return 0;
}

int
thread_create(int ttype, thread_start_t *f)
{
    int ret = 0;
    ret = pthread_create(&threads[ttype], NULL, f, NULL);
    assert(ret == 0);
    return ret;
}

void *
thread_core(void *arg)
{
    while (true)
    {
        sem_wait(&rxq.items_sem);
        pthread_mutex_lock(&rxq.lock);

        skb_t *skb = skb_dequeue(&rxq);

        pthread_mutex_unlock(&rxq.lock);
        sem_post(&rxq.slots_sem);

        netdev_receive(skb, &host);
        skb_free(skb);
    }

    return 0;
}

static int
thread_init()
{
    thread_create(THREAD_CORE, &thread_core);
    thread_create(THREAD_RX_QUEUE, &thread_rx_queue);
    thread_create(THREAD_TX_QUEUE, &thread_tx_queue);

    return 0;
}

static void
stack_free()
{
    tap_close(&g_tap);
    fflush(stdout);
    fflush(stderr);
    getc(stdin);
}

int
main(int argc, char *argv[])
{
    stack_init();
    thread_init();
    stack_free();

    return 0;
}
