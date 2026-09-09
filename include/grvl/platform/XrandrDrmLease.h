#ifndef XRANDR_LEASE_H_
#define XRANDR_LEASE_H_

#include <grvl/grvl.h>

namespace grvl {

    int AcquireXrandrLease(int driver_fd, uint32_t preferred_connector_id);

}

#endif
