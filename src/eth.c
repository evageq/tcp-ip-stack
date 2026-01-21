#include <netinet/in.h>
#include "eth.h"

inline bool
is_eth_arp(eth_frame_t *eth)
{
    if (ntohs(eth->ether_type) == ETH_P_ARP)
    {
        return true;
    }

    return false;
}

