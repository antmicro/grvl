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

#ifndef GRVL_COMPONENT_H_
#define GRVL_COMPONENT_H_

#include "Definitions.h"
#include "Event.h"
#include "Painter.h"
#include "Touch.h"
#include "stl.h"
#include "tinyxml2.h"
#include <stdint.h>

namespace grvl {

#define WIDGET2(id, name)                                                                                                    \
    static Component* WidgetCreate##name##id(XMLElement* xmlElement) { return (Component*)name::BuildFromXML(xmlElement); }; \
    void __attribute__((used))* TEMP_##name##id = register_widget_constructor((void*)&WidgetCreate##name##id, (char*)#name);
#define WIDGET1(id, name) WIDGET2(id, name)
#define WIDGET(name) WIDGET1(__COUNTER__, name)

    typedef struct {
        void* fun;
        char name[80];
    } component_list_item;

    extern int ComponentCount;
    extern component_list_item list[150];

    void* register_widget_constructor(void* con, char* nm);
    class Component;
    Component* create_component(const char* nm, void* el);

    /// Represents base class for all widgets.
    class Component {
    public:
        enum ComponentState {
            Off = 0,
            On,
            Pressed,
            Released,
            OnAndSelected,
            OffAndSelected
        };

        void* operator new(size_t size);
        void operator delete(void* ptr);

        Component()
            : ID("")
            , X(0)
            , Y(0)
            , Height(0)
            , Width(0)
            , ForegroundColor(COLOR_ARGB8888_GRAY)
            , ActiveForegroundColor(COLOR_ARGB8888_GRAY)
            , BackgroundColor(COLOR_ARGB8888_TRANSPARENT)
            , ActiveBackgroundColor(COLOR_ARGB8888_TRANSPARENT)
            , State(Off)
            , onPress()
            , onRelease()
            , onClick()
            , touchActive(false)
            , childDroppedTouch(false)
            , previousResponse(Touch::TouchNA)
            , Visible(true)
        {
        }

        Component(int32_t x, int32_t y, int32_t width, int32_t height)
            : ID("")
            , X(x)
            , Y(y)
            , Height(height)
            , Width(width)
            , ForegroundColor(COLOR_ARGB8888_GRAY)
            , ActiveForegroundColor(COLOR_ARGB8888_GRAY)
            , BackgroundColor(COLOR_ARGB8888_TRANSPARENT)
            , ActiveBackgroundColor(COLOR_ARGB8888_TRANSPARENT)
            , State(Off)
            , onPress()
            , onRelease()
            , onClick()
            , touchActive(false)
            , childDroppedTouch(false)
            , previousResponse(Touch::TouchNA)
            , Visible(true)
        {
        }

        virtual ~Component();

        void SetID(const char* id);
        virtual void SetPosition(int32_t x, int32_t y);
        virtual void SetSize(int32_t width, int32_t height);
        /// Sets component's background color.
        ///
        /// @param color Desired color in ARGB8888 format.
        virtual void SetBackgroundColor(uint32_t color);
        virtual void SetForegroundColor(uint32_t color);
        /// Sets component's background color when pressed.
        ///
        /// @param color Desired color in ARGB8888 format.
        virtual void SetActiveBackgroundColor(uint32_t color);
        virtual void SetActiveForegroundColor(uint32_t color);
        void SetState(ComponentState state);
        virtual void SetVisible(bool state);

        const char* GetID();
        int32_t GetX() const;
        int32_t GetY() const;
        int32_t GetWidth() const;
        int32_t GetHeight() const;
        uint32_t GetBackgroundColor() const;
        uint32_t GetForegroundColor() const;
        uint32_t GetActiveBackgroundColor() const;
        uint32_t GetActiveForegroundColor() const;
        virtual uint32_t GetCurrentBackgroundColor();
        virtual uint32_t GetCurrentForegroundColor();
        ComponentState GetState();
        bool IsVisible() const;

        virtual void Hide();
        virtual void Show();

        virtual void OnPress();
        virtual void OnRelease();
        virtual void OnClick();

        void SetOnPressEvent(const Event& event);
        void SetOnReleaseEvent(const Event& event);
        void SetOnClickEvent(const Event& event);

        virtual Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY,
                                                  int32_t modificator = 0);
        virtual bool IsTouchPointInObject(int32_t x, int32_t y, int32_t modificator = 0);

        virtual void CheckPlacement();
        virtual void ClearTouch();

        virtual void Draw(Painter& painter, int32_t ParentX, int32_t ParentY, int32_t ParentWidth,
                          int32_t ParentHeight)
            = 0;

        bool CheckSizeValidity(Component* child) const;

    protected:
        string ID;
        int32_t X, Y, Height, Width;
        uint32_t ForegroundColor, ActiveForegroundColor, BackgroundColor, ActiveBackgroundColor;
        ComponentState State;
        Event onPress, onRelease, onClick;
        bool touchActive, childDroppedTouch;
        Touch::TouchResponse previousResponse;
        bool Visible;

        virtual Touch::TouchResponse ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY);
        Touch::TouchResponse OnMoveCase(const Touch& tp, int32_t ParentX, int32_t ParentY);

        virtual void InitFromXML(tinyxml2::XMLElement* xmlElement);
    };
} /* namespace grvl */

#endif /* GRVL_COMPONENT_H_ */
