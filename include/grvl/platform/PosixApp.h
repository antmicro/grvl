#ifndef POSIXAPP_H_
#define POSIXAPP_H_

#include <grvl/Application.h>

namespace grvl {

    class PosixApp : public Application {
    protected:

        PosixApp(int width, int height, bool rotate_sideways);

        void SetCallbacks(gui_callbacks_t& callbacks) override;

    };
}

#endif // POSIXAPP_H_
