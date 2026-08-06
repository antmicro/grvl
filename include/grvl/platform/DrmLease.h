#ifndef DRMLEASE_H_
#define DRMLEASE_H_

#include <grvl/grvl.h>

namespace grvl {

    int LeaseDriver(int driver_fd, uint32_t preferred_connector_id);

}

#endif
