#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "defs.h"
#include "eth.h"
#include "arp.h"
#include "tuntap.h"

int
main()
{
    // https://stackoverflow.com/questions/79758511/get-tap-device-mac-address
    tap_t tap = init_tap("tap%d");
    if (tap.valid == false)
    {
        error("Failed to init tap");
        exit(-1);
    }

    if (tap_set_addr(&tap, HOST_IP) != 0)
    {
        error("Failed to set tap address");
    }

    if (tap_up(&tap) != 0)
    {
        error("Failed to up tap dervice");
    }
    set_tap_hwa(&tap);
    get_tap_hwa(&tap);

    while (1)
    {
        uint8_t tmp_buf[BUF_READ_LEN];
        int bytes_read = tap_read(&tap, BUF_READ_LEN, tmp_buf);

        if (bytes_read < 0)
        {
            error("Failed to tap_read");
        }

        eth_frame_t *eth_hdr = (eth_frame_t *)tmp_buf;
        if (is_eth_arp(eth_hdr))
        {
            bool arp_request = ntohs(((arp_hdr_t*)eth_hdr->payload)->oper) == ARP_REQUEST;
            arp_process(&tap, (arp_hdr_t *)eth_hdr->payload);
            if (arp_request == true)
            {
            }
        }
    }

    close_tap(&tap);

    fflush(stdout);
    fflush(stderr);
    getc(stdin);

    return 0;
}
