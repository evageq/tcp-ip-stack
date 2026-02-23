#include "arp.h"
#include "eth.h"
#include "tll.h"
#include "tuntap.h"
#include "util.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

tap_t g_tap;
bool SHELL_DEBUG = true;

int
main()
{
    // https://stackoverflow.com/questions/79758511/get-tap-device-mac-address
    g_tap = tap_create("192.186.12.4", "2a:e9:ea:46:21:70", "tap%d");
    if (g_tap.valid == false)
    {
        error("Failed to init tap %s", g_tap.name);
        exit(-1);
    }
    else
    {
        debug("%s valid", g_tap.name);
    }

    netdev_t host = netdev_init("192.168.12.100", "2a:e9:ea:46:21:71");

    tap_up(&g_tap);

    while (1)
    {
        uint8_t frame[BUF_READ_LEN];

        int bytes_read = tap_read(&g_tap, LENGTH(frame), frame);
        if (bytes_read < 0)
        {
            error("Failed tap_read");
        }

        eth_frame_t *eth_hdr = (eth_frame_t *)frame;
        int e_type = eth_type(eth_hdr);
        switch (e_type)
        {
            case ETH_P_ARP:
            {
                arp_process(&host, (arp_hdr_t*)eth_hdr->payload);
                break;
            }
            default:
            {
                debug("Unknown eth_type %d", e_type);
                break;
            }
        }
    }

    tap_close(&g_tap);

    fflush(stdout);
    fflush(stderr);
    getc(stdin);

    return 0;
}
