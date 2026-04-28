#include "util.h"
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char *packet_dir_str[PACKET_DIR_MAX] = { "in", "out" };
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

void
_error(const char *f, int line, const char *fmt, ...)
{
    pthread_mutex_lock(&log_lock);
    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4096, fmt, args);
    va_end(args);
    fprintf(stderr, "[%s:%d][ERROR]: %s: %s\n", f, line, buf, strerror(errno));
    fflush(stdout);

    pthread_mutex_unlock(&log_lock);
}

void
_debug(const char *f, int line, const char *fmt, ...)
{
    pthread_mutex_lock(&log_lock);

    char buf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4096, fmt, args);
    va_end(args);
    fprintf(stderr, "[%s:%d][DEBUG]: %s\n", f, line, buf);
    fflush(stdout);

    pthread_mutex_unlock(&log_lock);
}

void
print_hex_packet(size_t n, const uint8_t buf[n], int bytes_read,
                 unsigned int dir)
{
    pthread_mutex_lock(&log_lock);

    printf("Packet");
    if (dir < PACKET_DIR_MAX)
    {
        printf(" dir %s", packet_dir_str[dir]);
    }
    else
    {
        printf(";");
    }
    printf("\n");

    for (int i = 0; i < bytes_read; ++i)
    {
        if (i % 8 == 0)
        {
            printf("\n\t");
        }
        printf("%02x ", buf[i]);
    }
    printf("\n");
    fflush(stdout);

    pthread_mutex_unlock(&log_lock);

}

inline bool
cmp_hw_addr(uint8_t hw1[6], uint8_t hw2[6])
{
    bool flag = true;
    for (int i = 0; i < 6; ++i)
    {
        flag &= hw1[i] == hw2[i];
    }

    return flag;
}
