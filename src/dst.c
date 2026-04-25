#include "dst.h"
#include "arp.h"
#include "route.h"
#include "util.h"

int
dst_neigh_send(skb_t *skb)
{
    const mac_t *dmac;
    uint32_t daddr;

    dmac = arp_get_hw_addr(skb->rt->dst);

    if (dmac == NULL)
    {
        arp_request(skb->out_dev, skb->rt->dst);
        return -1;
    }

    netdev_send(skb, *dmac, ETH_P_IP);

    return 0;
}
