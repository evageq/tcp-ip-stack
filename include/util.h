#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void _error(const char *f, int line, const char *fmt, ...);
void _debug(const char *f, int line, const char *fmt, ...);
void print_hex_packet(size_t n, const uint8_t buf[n], int bytes_read);
inline bool cmp_hw_addr(uint8_t hw1[6], uint8_t hw2[6]);
