// Copyright 2014-2024 Antmicro <antmicro.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef GRVL_MANAGER_H_
#define GRVL_MANAGER_H_

#include "stl.h"
#include "tinyxml2.h"

#include <math.h>
#include <time.h>
#include <unistd.h>

#include "grvl.h"
#include "Queue.h"
#include "Popup.h"
#include "XMLSupport.h"

#include "Image.h"
#include "Painter.h"
#include "Panel.h"

#include "AbstractView.h"
#include "CustomView.h"
#include "VerticalScrollView.h"

// #include "Component.h"
// #include "Font.h"
#include "Button.h"
#include "CircleProgressBar.h"
#include "Clock.h"
#include "GridRow.h"
#include "GridView.h"
#include "Label.h"
#include "ListView.h"
#include "ProgressBar.h"
#include "Slider.h"
#include "SwitchButton.h"
#include "ScrollPanel.h"
// #include "Ico.h"
// #include "Touch.h"
// #include "Event.h"
// #include "ContentManager.h"
#include "Division.h"

using namespace tinyxml2;
namespace grvl {

    /// grvl manager class.
    ///
    /// This class is used as an entry point for all
    /// operations invoked on the library.
    class Manager {
    public:
        static void Initialize(uint32_t xSize, uint32_t ySize, int bpp, bool rotate90);
        static void Initialize(uint32_t xSize, uint32_t ySize, int bpp, bool rotate90, uint8_t* framebuffer);
        static Manager& GetInstance();

        typedef Queue<Event> EventQueue;
        typedef Queue<Popup> PopupQueue;

        enum State {
            Loading = 0,
            Refreshing,
            ToTheRight,
            ToTheLeft,
            CrossFade
        };

        enum Mode {
            Rotation0 = 0,
            Rotation90
        };

        // Todo use the defined names in the lib
        enum BufferName {
            Hidden1 = 0,
            Hidden2,
            Visible1,
            Visible2
        };

        class KeyData {
        public:
            KeyData()
                : name("")
                , repeat(100)
            {
            }
            string name;
            uint32_t repeat;
        };

        map<string, char*> widgetAttributes;
        map<string, char*> classAttributes;
        map<string, char*> idAttributes;

        void* operator new(size_t size);
        void operator delete(void* ptr);

        virtual ~Manager();
        Panel* GetTopPanel();
        Panel* GetBottomPanel();

        unsigned int flips;

        /// @return Width of the display in pixels.
        uint32_t GetWidth() const;
        /// @return Height of the display in pixels.
        uint32_t GetHeight() const;

        int32_t GetTopPanelHeight();
        bool GetGlobalTopPanelVisibility();
        int32_t GetTotalHeadersHeight();
        int32_t GetBottomPanelHeight();

        Manager& SetScrollingDuration(uint32_t milliseconds);
        uint32_t GetScrollingDuration(void) const;

        uintptr_t GetFramebufferPointer(BufferName name) const;

        /// Sets transparency. TODO - Adjust the description
        ///
        /// @remark
        /// Calling this method will invoke registered callback
        /// called 'set_layer_transparency' with provided argument.
        /// It does not change internal state of a library.
        ///
        /// @param value Transparency value.
        Manager& SetTransparency(float value, uint32_t milliseconds);

        /// Changes currently displayed screen with optional animation.
        ///
        /// @param activeScreenId Identifier of a screen to display.
        /// @param direction Direction of animation (-1 = to the left, 1 = to the right, 0 = no animation)
        Manager& SetActiveScreen(const char* activeScreenId, int8_t direction);

        /// Sets an image that is displayed while the application is loading its resources.
        ///
        /// @remark
        /// In order to switch from loading to normal state, application should
        /// call InitializationFinished method.
        ///
        /// @param image Reference to image object that should be displayed when loading.
        Manager& SetLoadingImage(const Image& image);
        Manager& SetLoadingImageVisibility(uint8_t visible);

        /// Sets image that is displayed below screens.
        ///
        /// @param image Reference to the image to display.
        Manager& SetBackgroundImage(const Image& image);

        /// Sets image that is displayed as screen collection indicator.
        ///
        /// @param image Reference to the image to display.
        Manager& SetCollectionImage(const Image& image);

        /// Sets background color for underlying layer.
        ///
        /// @remark
        /// Calling this method will invoke registered callback
        /// called 'set_background_color' with provided argument.
        /// It does not change internal state of a library.
        ///
        /// @param color Background color in format acceptable by the provided callback.
        Manager& SetBackgroundColor(uint32_t color);
        Manager& SetLogoImage(const Image& logo);

        Manager& SetTouchAreaModificator(int32_t modificator);
        int32_t GetTouchAreaModificator(void) const;

        /// Handles touch event.
        ///
        /// @param touched Indicates if the display is touched.
        /// @param touchX Position in axis X where the touch event was detected.
        /// @param touchY Position in axis Y where the touch event was detected.
        void ProcessTouchPoint(bool touched, uint32_t touchX, uint32_t touchY);

        Popup* GetPopupInstance();
        Manager& AddPopup(Popup* popup);

        /// Displays pop-up window.
        ///
        /// @param Style Name of the style defined in XML document.
        void ShowPopup(const char* Style);
        void ShowPopup(const char* Style, const char* Message);
        void ShowPopup(const char* Style, uint32_t milliseconds);
        void ShowPopup(const char* Style, const char* Message, uint32_t milliseconds);

        /// Closes last pop-up window.
        void ClosePopup();

        Manager& SetPopupPointer(Popup* popup);
        Manager& KeepPopupVisible(uint32_t milliseconds);
        bool IsPopupVisible();

        /// Switches library to normal state when screens are displayed.
        ///
        /// @remark
        /// Before calling this method loading image (if defined) is displayed.
        void InitializationFinished();
        int8_t IsInitializationFinished();
        int8_t IsLoadingIcoVisible() const;

        Manager& AddScreen(AbstractView* screen);
        vector<AbstractView*>& GetScreensCollection();
        AbstractView* GetLastScreen();
        AbstractView* GetActiveScreen();

        /// Gets screen based on its name.
        ///
        /// @param id Name of the screen.
        /// @return Pointer to a screen or NULL if not found.
        AbstractView* GetScreen(const char* id);

        /// Gets screen based on its location.
        /// @param id Numerical identifier of a screen (zero-based).
        /// @return Pointer to the screen or NULL if not found.
        AbstractView* GetScreen(int id);
        Image& GetBackgroundImage();
        Image& GetLoadingImage();
        State GetState();

        void Animate(AbstractView* startScreen, AbstractView* targetScreen, int8_t direction);

        /// Registers image content in content manager.
        ///
        /// The image content will be accessible for other
        /// components by the provided name.
        ///
        /// @remark
        /// It is advised to use content manager when
        /// image content is shared by different
        /// components, as it will save memory.
        ///
        /// @param name Image content's identifier.
        /// @param image Image content object.
        Manager& AddImageContentToContainer(string name, ImageContent* image);

        /// Binds registered image content to an image object.
        ///
        /// @param contentName Identifier of the content.
        /// @param image Image object.
        Manager& BindImageContentToImage(const string& contentName, Image* image);

        /// Registers callback method for an event.
        ///
        /// @remark
        /// This method filters out empty callbacks (i.e., NULLs).
        ///
        /// @param name Identifier of an event that should invoke the callback.
        /// @param Callback Callback that should be called on event.
        Manager& AddCallbackToContainer(const string& name, Event::CallbackPointer Callback);
        
        /// Tries to search if there is callback defined with C/C++ code (added by AddCallbackToContainer), if not
        /// then it creates new one that will call JavaScript code with provided constant args.
        ///
        /// @param callbackFunctionName Identifier of a function invoked by the callback.
        /// @param callbackArgs Constant args that will be passed as callback function arguments
        Event GetOrCreateCallback(const string& callbackFunctionName, const Event::ArgVector& callbackArgs);
        Event GetOrCreateCallback(const CallbackDefinition& callbackDefinition);

        /// Register font in content manager.
        ///
        /// The font will be accessible for other
        /// components by the provided name.
        ///
        /// @remark
        /// It is advised to use content manager when
        /// font is used by multiple components, as it
        /// will save memory.
        ///
        /// @param name Font's identifier.
        /// @param font Font object.
        Manager& AddFontToFontContainer(const string& name, Font* font);

        void AddKeyMappingToContainer(uint16_t mask, const string& name, uint32_t repeat);

        Font const* GetFontFromContainer(const char* name) const;
        Font const* GetDefaultFontFromContainer() const;
        Popup* GetPopupFromContainer(const char* name);
        Font const* GetFontPointer(const char* fontName) const;

        // XML
        /// Loads screens from XML file.
        ///
        /// @param filename Path to the file with screens definition.
        /// @return Result of parsing the file (0 = OK, -1 = there was an error)
        int32_t BuildFromXML(const char* filename);
        int32_t BuildFromXML(const char* XMLDoc, size_t len);
        Event GetEventWithArguments(const char* eventName) const;

        // //Physical keys
        // /// Handles physical key press/release event.
        // ///
        // /// @param pressed Indicates if the key is pressed (true) or released (false).
        // /// @param code Code of the key.
        void ProcessKeyInput(bool pressed, uint16_t code);
        Key::KeyState PressKey(const char* id);
        Key::KeyState ReleaseKey(const char* id);

        void DrawNextLoadingFrame();

        /// Redraws content of a display based on current state of components.
        void Draw();
        void DrawDots(uint32_t numberOfDots, uint32_t activeDot);

        EventQueue& GetEventsQueueInstance();

        /// Executes an iteration of processing loop.
        ///
        /// This method handles pop-up windows, processes events and redraw screen if needed.
        void MainLoopIteration();

        void ResetScreens();
        void ClearBuffers();

        /// Registers method that is called when request for external content is issued by the library.
        ///
        /// @param requestCallback Pointer to the method to call.
        Manager& SetExternalContentRequestCallback(ContentManager::ContentCallback requestCallback);
        Manager& SetCancelExternalContentRequestCallback(ContentManager::ContentCallback cancelRequestCallback);
        Painter painter; // TODO

        Component* FindElementInTheActiveScreenById(const char* id);

    private:
        Manager(uint32_t xSize, uint32_t ySize, int bpp, bool rotate90 = false, uint8_t* framebuffer = NULL);

        static Manager* instance;

        uint32_t width;
        uint32_t height;
        typedef map<string, Event::CallbackPointer> EventsContainerMap;
        typedef map<string, Font const*> FontContainerMap;
        typedef map<uint32_t, KeyData> KeyMappingMap;
        EventsContainerMap EventsContainer;
        FontContainerMap FontContainer;
        KeyMappingMap KeyMappingContainer;
        vector<AbstractView*> Screens;

        vector<Popup*> PopupsContainer;
        Panel* TopPanel;
        Panel* BottomPanel;
        Panel* StartHeader;
        Panel* TargetHeader;
        AbstractView *ActiveScreen, *TargetScreen;
        int32_t AnimationWindowOffset; // 0 to XSize
        uint64_t CurrentTimestamp, BeginTimestamp, PointerTimestamp, KeyPressTimestamp;
        State ManagerState;
        Image LoadingImage;
        Image BackgroundImage;
        Image CollectionImage;
        uint8_t LoadingImageVisible;
        Popup* CurrentPopup;
        uint64_t ScrollingDuration;
        Touch TouchEvent;
        Image LogoImage;
        // Painter painter;
        bool previousTouchState;
        bool touchToPopup;
        bool timeoutedPopupMode;
        bool longPressActive, keyActive;
        int32_t touchAreaModificator; /* TODO: should be touchAreaModifier */
        uint32_t dotColor, dotActiveColor;
        int32_t dotDistance, dotRadius, dotYPos, debugDot;
        KeyData activeKey;
        EventQueue eventsQueue;
        PopupQueue popupsQueue;
        ContentManager contentManager;
        float initialTransparency, desiredTransparency, currentTransparency;
        uint64_t fadeBeginTimestamp, fadeEndTimestamp;

        // mutex
        void* DrawMutex;
        int LockDrawMutex();
        void UnlockDrawMutex();

        // XML private
        void ParseGuiConfiguration(XMLElement* ConfigNode);
        void ParseKeypadMapping(XMLElement* KeypadNode);
        void ParseStylesheet(XMLElement* stylesheet);
        void ParseScripts(XMLElement* scripts);
        void ParseStyle(char* active_object, char* active_parameter, char* active_parameter_value);

        void ProcessEvents();
        bool UpdateAnimationWindowOffset();
        void ShowPopup();

        void ApplyTransparency();

        Event::CallbackPointer GetCallbackFromContainer(const string& name) const;
    };

} /* namespace grvl */

#endif /* GRVL_MANAGER_H_ */
