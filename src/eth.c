#include "eth.h"
#include <netinet/in.h>
#include <stdio.h>

inline int
eth_type(const skb_t *skb)
{
    eth_frame_t *e_hdr = (eth_frame_t*)skb->data;
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
    return sizeof(eth_frame_t);
}

inline void *
mac_hdr(skb_t *skb)
{
    return skb->mac_head;
}
