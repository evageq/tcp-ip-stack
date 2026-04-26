#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

#define HOST_IP "192.168.1.10"

#define BUF_READ_LEN 4096
#define HOST_ADDR_LEN 128

#define BUFCMDSZ 128

extern bool SHELL_DEBUG;

#define SET_FLAG(v, f) (f |= f)
#define UNSET_FLAG(v, f) (f &= !f)
#define CHECK_FLAG(v, f) (f & f)

#define SHELL(s, ...)                               \
    ({                                              \
        char cmd[BUFCMDSZ];                         \
        snprintf(cmd, LENGTH(cmd), s, __VA_ARGS__); \
        if (SHELL_DEBUG == true)                    \
        {                                           \
            debug("exec: %s", cmd);                 \
        }                                           \
        system(cmd);                                \
    })

#define error(...) _error(__FILE__, __LINE__, __VA_ARGS__)
#define debug(...) _debug(__FILE__, __LINE__, __VA_ARGS__)

#define LENGTH(a) (sizeof((a)) / sizeof((a)[0]))

#define MAX(a, b)                  \
    ({                             \
        __typeof__((a)) __a = (a); \
        __typeof__((b)) __b = (b); \
        __a > __b ? __a : __b;     \
    })
#define MIN(a, b)                 \
    ({                            \
        __typeof__((a)) _a = (a); \
        __typeof__((b)) _b = (b); \
        _a < _b ? _a : _b;        \
    })

#define SWAP(a, b)                                 \
    do                                             \
    {                                              \
        __typeof__(a) _a = (a);                    \
        __typeof__(b) _b = (b);                    \
        assert(sizeof(*_a) == sizeof(*_b));        \
        size_t sz = sizeof(*_a);                   \
        uint8_t tmp[sz];                           \
        memcpy((uint8_t *)tmp, (uint8_t *)_a, sz); \
        memcpy((uint8_t *)_a, (uint8_t *)_b, sz);  \
        memcpy((uint8_t *)_b, (uint8_t *)tmp, sz); \
    } while (0)

void _error(const char *f, int line, const char *fmt, ...);
void _debug(const char *f, int line, const char *fmt, ...);
void print_hex_packet(size_t n, const uint8_t buf[n], int bytes_read);
