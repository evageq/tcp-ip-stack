#ifndef __DEFS__
#define __DEFS__

#include <stdint.h>
#include <sys/socket.h>
#include "util.h"

#define HOST_IP "192.168.1.10"

#define BUF_READ_LEN 4096
#define HOST_ADDR_LEN 128
#define ADDR(x) ((uint32_t)(*(uint32_t*)x))
#define SOCK_ADDR(saddr) (ADDR(&(((struct sockaddr_in*)&saddr)->sin_addr)))
#define TAP_ADDR(tap) (SOCK_ADDR(tap->saddr))

#define error(...) _error(__FILE__, __LINE__, __VA_ARGS__)
#define debug(...) _debug(__FILE__, __LINE__, __VA_ARGS__)

#endif // __DEFS__
