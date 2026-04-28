#include "route.h"
#include "netdev.h"
#include "tll.h"
#include "tuntap.h"
#include "util.h"

route_table_t rt_table = RT_TABLE_INIT();
extern tap_t g_tap;
extern netdev_t host;

static int
rt_add(uint32_t prefix, uint32_t mask, uint32_t gateway, uint32_t flags,
       netdev_t *dev)
{
    rtentry_t tmp = { .prefix = prefix,
                      .mask = mask,
                      .gateway = gateway,
                      .flags = flags,
                      .dev = dev };
    tll_push_back(rt_table, tmp);
    return 0;
}

void
rt_init()
{
    rt_add(host.dev_addr & host.netmask, host.netmask, 0, RT_FLAGS_HOST,
           &host);
    rt_add(0, 0, g_tap.netdev.dev_addr, RT_FLAGS_GATEWAY, &host);
}

rtentry_t *
rt_lookup(uint32_t daddr)
{
    rtentry_t *res = NULL;

    tll_foreach(rt_table, rt_entry)
    {
        if (rt_entry->item.prefix == (daddr & rt_entry->item.mask))
        {
            if (res)
            {
                if (rt_entry->item.mask > res->mask)
                {
                    res = &rt_entry->item;
                }
            }
            else
            {
                res = &rt_entry->item;
            }
        }
    }

    assert(res != NULL);

    return res;
}
