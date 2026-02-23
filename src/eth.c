#include "eth.h"
#include <netinet/in.h>
#include <stdio.h>

inline int
eth_type(const eth_frame_t *frame)
{
    return ntohs(frame->ether_type);
}

inline char *
mac2str(const mac_t mac, size_t n, char buf[n])
{
    snprintf(buf, n, "%2x:%2x:%2x:%2x:%2x:%2x", mac[0], mac[1], mac[2], mac[3],
             mac[4], mac[5]);
    return buf;
}
