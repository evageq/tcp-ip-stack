#include "route.h"
#include "netdev.h"
#include "tll.h"
#include "tuntap.h"
#include "util.h"

route_table_t rt_table = RT_TABLE_INIT();
extern tap_t g_tap;
extern netdev_t host;

static int
rt_add(uint32_t addr, uint32_t mask, int flags, netdev_t *dev)
{
    rtentry_t tmp = { .dst = addr, .flags = flags, .dev = dev };
    tll_push_back(rt_table, tmp);
    return 0;
}

void
rt_init()
{
    rt_add(g_tap.netdev.dev_addr, g_tap.netdev.netmask, RT_DEFAULT,
           &g_tap.netdev);
    // rt_add(RT_LOOPBACK);
    rt_add(host.dev_addr, host.netmask, RT_DEV, &host);
}

rtentry_t *
rt_lookup(uint32_t daddr)
{
    rtentry_t *res = NULL;
    rtentry_t *gw = NULL;
    tll_foreach(rt_table, rt_entry)
    {
        if ((rt_entry->item.dst & rt_entry->item.dev->netmask)
            == (daddr & rt_entry->item.dev->netmask))
        {
            if (res)
            {
                if (rt_entry->item.dev->netmask > res->dev->netmask)
                {
                    res = &rt_entry->item;
                }
            }
            else
            {
                res = &rt_entry->item;
            }
        }

        if (CHECK_FLAG(rt_entry->item.flags, RT_DEFAULT))
        {
            assert(gw == NULL);
            gw = &rt_entry->item;
        }
    }

    if (res == NULL)
    {
        assert(gw != NULL);
        return gw;
    }

    return res;
}
