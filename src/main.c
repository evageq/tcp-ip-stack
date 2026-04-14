#include "skb.h"
#include "eth.h"
#include "tuntap.h"
#include "util.h"
#include <arpa/inet.h>
#include <assert.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

tap_t g_tap;
bool SHELL_DEBUG = true;
netdev_t host;

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

    host = netdev_init("10.0.0.4", "2a:e9:ea:46:21:71");

    tap_up(&g_tap);
    return 0;
}

static int
stack_init()
{
    net_init();
    return 0;
}

static int
netdev_rx_loop()
{
    while (1)
    {
        uint8_t buf[BUF_READ_LEN];
        int bytes_read = tap_read(&g_tap, LENGTH(buf), buf);
        if (bytes_read < 0)
        {
            error("Failed tap_read");
            continue;
        }

        skb_t *skb = skb_alloc(bytes_read);
        skb_put_data(skb, buf, bytes_read);
        netdev_receive(skb, &host);
        skb_free(skb);
    }

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
    netdev_rx_loop();
    stack_free();

    return 0;
}
