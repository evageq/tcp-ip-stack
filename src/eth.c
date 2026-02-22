#include <netinet/in.h>
#include "eth.h"

inline int
eth_type(const eth_frame_t *frame)
{
    return ntohs(frame->ether_type);
}
