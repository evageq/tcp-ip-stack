#ifndef __ICMPV4__
#define __ICMPV4__

#include "skb.h"
#include <stdint.h>

enum
{
    ICMP4_ECHO = 8,
};

typedef struct icmp4_s
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint8_t data[];

} __attribute__((packed)) icmp4_t;

icmp4_t *icmp4_hdr(const skb_t *skb);
int icmp_echo_process(skb_t *skb);
int icmp_echo_response(skb_t *skb);
int icmp4_len(skb_t *skb);

#endif // __ICMPV4__
