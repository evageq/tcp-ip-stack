#include "ipv4.h"

int
ip_process(const netdev_t *dev, skb_t *skb)
{
    skb_pull(skb, skb->dev->mac_len);
    skb->network_head = skb->data;
    iphdr_t *iphdr = ip_hdr(skb);
    int ip_proto_type = ntohs(iphdr->proto);
    switch (ip_proto_type)
    {
        case ICMP_PROTO:
        {
            // icmp_process(datagram->)
            // break;
        }
    }
    return 0;
}

inline iphdr_t *
ip_hdr(const skb_t *skb)
{
    return (iphdr_t *)skb->network_head;
}
