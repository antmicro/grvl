#ifndef FBTERM_LEASE_H_
#define FBTERM_LEASE_H_

#include <grvl/grvl.h>

namespace grvl {

    int AcquireFbtermLease();
    bool ReleaseFbtermLease();

}

#endif
