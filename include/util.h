#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

#define HOST_IP "192.168.1.10"

#define BUF_READ_LEN 4096
#define HOST_ADDR_LEN 128

#define error(...) _error(__FILE__, __LINE__, __VA_ARGS__)
#define debug(...) _debug(__FILE__, __LINE__, __VA_ARGS__)

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define MAX(a, b)                 \
    ({                            \
        __typeof__((a)) _a = (a); \
        __typeof__((b)) _b = (b); \
        _a > _b ? _a : _b;        \
    })
#define MIN(a, b)                 \
    ({                            \
        __typeof__((a)) _a = (a); \
        __typeof__((b)) _b = (b); \
        _a < _b ? _a : _b;        \
    })

#define SWAP(a, b)                               \
    do                                           \
    {                                            \
        __typeof__(a) _a = (a);                  \
        __typeof__(b) _b = (b);                  \
        size_t sz = MIN(sizeof(_a), sizeof(_b)); \
        uint8_t tmp[sz];                         \
        memcpy(tmp, _a, sz);                     \
        memcpy(_a, _b, sz);                      \
        memcpy(_b, tmp, sz);                     \
    } while (0)

void _error(const char *f, int line, const char *fmt, ...);
void _debug(const char *f, int line, const char *fmt, ...);
void print_hex_packet(size_t n, const uint8_t buf[n], int bytes_read);
