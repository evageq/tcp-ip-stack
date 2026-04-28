#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "netdev.h"
#include "tll.h"
#include <stdint.h>

#define RT_TABLE_INIT() tll_init()

#define RT_FLAGS_GATEWAY (1 << 0)
#define RT_FLAGS_LOOP (1 << 1)
#define RT_FLAGS_HOST (1 << 2)

typedef struct rtentry_s
{
    uint32_t prefix;
    uint32_t mask;
    uint32_t gateway;
    uint32_t flags;
    netdev_t *dev;
} rtentry_t;

typedef tll_type(rtentry_t, route_table_s) route_table_t;

rtentry_t *rt_lookup(uint32_t daddr);

void rt_init();

#endif // __ROUTE_H__
