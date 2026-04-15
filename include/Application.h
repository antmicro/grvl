
#ifndef GRVL_APPLICATION_H_
#define GRVL_APPLICATION_H_

#include "grvl.h"



namespace grvl {

    // higher level abstraction over grvl setup
    //
    // This class (and its sub-classes) can be used to more easly configure grvl
    // for specific usecases. Without the need to handle events and rendering yourself.
    //
    // @remark
    // This class will terminate grvl and all application specific elements as soon as it is destroyed,
    // it is recomended to create the instance before the main render loop and only destroy it after the loop exits.
    class Application {
    protected:

        int width = 100;
        int height = 100;
        bool sideways = false;
        bool should_run = true;

        static inline Application* instance;
        uint8_t* framebuffer;

        static void SetFramebuffer(uintptr_t address);

        virtual void SetCallbacks(gui_callbacks_t& callbacks);
        virtual bool Setup() = 0;

    public:

        static void Init(Application* app);
        static Application* GetInstance();

        Application(int width, int height, bool rotate_sideways);
        virtual ~Application() = default;

        // Draw the next frame.
        // This method should be called in the main render loop.
        virtual void Render() = 0;

        // Pool user inputs.
        // This method should be called in the main render loop.
        virtual void Poll() = 0;

        // Check if the application should exit the main loop and terminate
        // for example due to the user closing the application window.
        virtual bool ShouldRun() const;

    };
}

#endif /* GRVL_APPLICATION_H_ */
