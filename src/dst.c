#include "dst.h"
#include "arp.h"
#include "ipv4.h"
#include "route.h"
#include "util.h"

int
dst_neigh_send(skb_t *skb)
{
    const mac_t *dmac;
    uint32_t daddr;

    if (CHECK_FLAG(skb->rt->flags, RT_FLAGS_GATEWAY))
    {
        daddr = skb->rt->gateway;
    }
    else
    {
        daddr = ip_hdr(skb)->daddr;
    }

    dmac = arp_get_hw_addr(daddr);

    if (dmac == NULL)
    {
        arp_request(skb->out_dev, daddr);
        return -1;
    }

    netdev_send(skb, *dmac, ETH_P_IP);

    return 0;
}
