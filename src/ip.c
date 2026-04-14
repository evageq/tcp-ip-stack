#include "ipv4.h"

int
ip_process(const netdev_t *dev, skb_t *skb)
{
    if (skb->len < (int)sizeof(iphdr_t))
    {
        return -1;
    }

    skb->network_head = skb->data;
    iphdr_t *iphdr = ip_hdr(skb);
    int ip_hdr_len = iphdr->ihl * 4;

    if (iphdr->version != 4)
    {
        return -1;
    }

    if (ip_hdr_len < (int)sizeof(iphdr_t) || skb->len < ip_hdr_len)
    {
        return -1;
    }

    skb->sock = (struct sock){ .addr = iphdr->saddr };
    skb_pull(skb, ip_hdr_len);
    skb->transport_head = skb->data;

    switch (iphdr->proto)
    {
        case ICMP_PROTO:
        {
            // icmp_process(skb);
            break;
        }
    }
    return 0;
}

inline iphdr_t *
ip_hdr(const skb_t *skb)
{
    return (iphdr_t *)skb->network_head;
}
