#include "ipv4.h"
#include "util.h"

inline int
ip_headroom(const netdev_t *dev)
{
    return dev->mac_head_len + sizeof(iphdr_t);
}

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
        debug("Unsupported ip version");
        return -1;
    }

    if (ip_hdr_len < (int)sizeof(iphdr_t) || skb->len < ip_hdr_len)
    {
        debug("Malformed ip packet");
        return -1;
    }

    skb->sock = (struct sock){ .addr = iphdr->saddr };
    skb_pull(skb, ip_hdr_len);
    skb->transport_head = skb->data;

    switch (iphdr->proto)
    {
        case ICMP_PROTO:
        {
            return icmp_process(skb);
        }
        default:
        {
            return 0;
        }
    }
}

inline iphdr_t *
ip_hdr(const skb_t *skb)
{
    return (iphdr_t *)skb->network_head;
}

uint16_t
checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     * Taken from https://tools.ietf.org/html/rfc1071
     */

    register uint32_t sum = 0;
    uint16_t *ptr = addr;

    while (count > 1)
    {
        /*  This is the inner loop */
        sum += *ptr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if (count > 0)
        sum += *(uint8_t *)ptr;

    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}
