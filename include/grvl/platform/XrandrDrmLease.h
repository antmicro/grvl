#ifndef XRANDR_LEASE_H_
#define XRANDR_LEASE_H_

#include <grvl/grvl.h>

namespace grvl {

    int AcquireXrandrLease(int driver_fd);

}

#endif
