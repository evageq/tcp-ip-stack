#include "icmp4.h"
#include "ipv4.h"
#include "util.h"
#include <string.h>

inline int
icmp4_len(skb_t *skb)
{
    iphdr_t *iphdr = ip_hdr(skb);
    return ntohs(iphdr->len) - iphdr->ihl * 4;
}

inline icmp4_t *
icmp4_hdr(const skb_t *skb)
{
    return (icmp4_t *)skb->transport_head;
}

int
icmp_process(skb_t *skb)
{
    icmp4_t *icmp = icmp4_hdr(skb);
    int type = icmp->type;

    switch (type)
    {
        case ICMP4_ECHO:
        {
            return icmp_echo_process(skb);
        }
        default:
        {
            debug("Unknown icmp request type");
            return -1;
        }
    }
}

int
icmp_echo_process(skb_t *skb)
{
    iphdr_t *iphdr = ip_hdr(skb);
    icmp4_t *icmphdr = icmp4_hdr(skb);
    int icmp_len_ = icmp4_len(skb);

    if (checksum(icmphdr, icmp4_len(skb)) != 0)
    {
        debug("Invalid checksum for icmp echo request");
        return -1;
    }

    icmp_echo_response(skb);
    return 0;
}

int
icmp_echo_response(skb_t *skb_request)
{
    skb_t *skb_response
        = skb_alloc(ip_headroom(skb_request->dev) + icmp4_len(skb_request));
    skb_reserve(skb_response, ip_headroom(skb_request->dev));

    struct sock sk;
    sk.daddr = ip_hdr(skb_request)->daddr;

    icmp4_t *icmp_hdr = skb_put(skb_response, icmp4_len(skb_request));
    int icmp_len = icmp4_len(skb_request);
    memcpy(icmp_hdr, icmp4_hdr(skb_request), icmp4_len(skb_request));
    icmp_hdr->code = 0;
    icmp_hdr->csum = 0;
    icmp_hdr->csum = checksum(icmp_hdr, icmp_len);

    ip_send(&sk, skb_response);

    skb_free(skb_response);

    return 0;
}
