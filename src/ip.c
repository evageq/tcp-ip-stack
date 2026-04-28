#include "dst.h"
#include "ipv4.h"
#include "route.h"
#include "sock.h"
#include "util.h"
#include <string.h>

inline int
mask2prefix_len(uint32_t mask)
{
    return __builtin_popcount(mask);
}

inline int
ip_headroom(const netdev_t *dev)
{
    return dev->mac_head_len + sizeof(iphdr_t);
}

int
ip_process(skb_t *skb)
{
    const netdev_t *dev = skb->in_dev;
    if (skb->len < (int)sizeof(iphdr_t))
    {
        return -1;
    }

    skb->network_head = skb->data;
    iphdr_t *iphdr = ip_hdr(skb);
    int version = (iphdr->ver_ihl & 0xf0) >> 4;
    int ip_hdr_len = (iphdr->ver_ihl & 0x0f) * 4;

    if (version != 4)
    {
        debug("Unsupported ip version");
        return -1;
    }

    if (ip_hdr_len < (int)sizeof(iphdr_t) || skb->len < ip_hdr_len)
    {
        debug("Malformed ip packet");
        return -1;
    }

    skb->sock = (struct sock){ .daddr = iphdr->saddr };
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

int
ip_send(struct sock *sk, skb_t *skb)
{
    int ret = 0;

    skb->network_head = skb_push(skb, sizeof(iphdr_t));

    iphdr_t *iphdr = ip_hdr(skb);
    rtentry_t *rt = rt_lookup(sk->daddr);
    skb->rt = rt;
    skb->out_dev = rt->dev;

    memset(iphdr, 0, sizeof(iphdr_t));
    iphdr->ver_ihl = (4 << 4) | (5);
    iphdr->saddr = rt->dev->dev_addr;
    iphdr->daddr = sk->daddr;
    iphdr->ttl = 64;
    iphdr->proto = ICMP_PROTO;
    iphdr->len = htons(skb->len);
    iphdr->flags_foffset = htons((1 << 14));
    iphdr->csum = 0;
    iphdr->csum = checksum(iphdr, sizeof(*iphdr));

    ret = dst_neigh_send(skb);
    if (ret < 0)
    {
        skb_free(skb);
    }

    return ret;
}
