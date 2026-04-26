#include "eth.h"
#include "netdev.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

inline int
eth_type(const skb_t *skb)
{
    ethhdr_t *e_hdr = (ethhdr_t *)skb->mac_head;
    return ntohs(e_hdr->ether_type);
}

inline char *
mac2str(const mac_t mac, size_t n, char buf[n])
{
    snprintf(buf, n, "%2x:%2x:%2x:%2x:%2x:%2x", mac[0], mac[1], mac[2], mac[3],
             mac[4], mac[5]);
    return buf;
}

inline int
mac_hdr_len(const netdev_t *dev)
{
    return sizeof(ethhdr_t);
}

inline ethhdr_t *
mac_hdr(skb_t *skb)
{
    return (ethhdr_t *)skb->mac_head;
}

inline int
mac_equal(const mac_t *a, const mac_t *b)
{
    return memcmp(a, b, sizeof(mac_t)) == 0;
}

bool
mac_recv_check_host(skb_t *skb)
{
    mac_t *mac = &mac_hdr(skb)->dmac;
    bool is_bcast = mac_equal(mac, &skb->in_dev->bcast_addr);
    bool is_eq = mac_equal(mac, &skb->in_dev->mac);
    return is_bcast || is_eq;
}
