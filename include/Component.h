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

#include <duktape.h>

#include "Definitions.h"
#include "Event.h"
#include "Painter.h"
#include "Touch.h"
#include "stl.h"
#include "tinyxml2.h"
#include <stdint.h>
#include "JSObject.h"
#include "JSObjectBuilder.h"

namespace grvl {

#define WIDGET2(id, name)                                                                                                    \
    static Component* WidgetCreate##name##id(XMLElement* xmlElement) { return (Component*)name::BuildFromXML(xmlElement); }; \
    void __attribute__((used))* TEMP_##name##id = register_widget_constructor((void*)&WidgetCreate##name##id, (char*)#name);
#define WIDGET1(id, name) WIDGET2(id, name)
#define WIDGET(name) WIDGET1(__COUNTER__, name)

#define GENERATE_DUK_GETTER(class, property_name, getter_func_name, duk_push_func_name)              \
    static duk_ret_t JSGet##property_name##Wrapper(duk_context* ctx)                                 \
    {                                                                                                \
        duk_push_this(ctx);                                                                          \
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);                                \
        class* component = static_cast<class*>(duk_to_pointer(ctx, -1));                             \
        if (component) {                                                                             \
            duk_push_func_name(ctx, component->getter_func_name());                                  \
            return 1;                                                                                \
        }                                                                                            \
                                                                                                     \
        return 0;                                                                                    \
    }

#define GENERATE_DUK_SETTER(class, property_name, setter_func_name, duk_get_func_name)               \
    static duk_ret_t JSSet##property_name##Wrapper(duk_context* ctx)                                 \
    {                                                                                                \
        duk_push_this(ctx);                                                                          \
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);                                \
        class* component = static_cast<class*>(duk_to_pointer(ctx, -1));                             \
        if (component) {                                                                             \
            component->setter_func_name(duk_get_func_name(ctx, 0));                                  \
        }                                                                                            \
                                                                                                     \
        return 0;                                                                                    \
    }

#define GENERATE_DUK_STRING_GETTER(class, property_name, getter_func_name) GENERATE_DUK_GETTER(class, property_name, getter_func_name, duk_push_string)
#define GENERATE_DUK_STRING_SETTER(class, property_name, setter_func_name) GENERATE_DUK_SETTER(class, property_name, setter_func_name, duk_to_string)
#define GENERATE_DUK_INT_GETTER(class, property_name, getter_func_name) GENERATE_DUK_GETTER(class, property_name, getter_func_name, duk_push_int)
#define GENERATE_DUK_INT_SETTER(class, property_name, setter_func_name) GENERATE_DUK_SETTER(class, property_name, setter_func_name, duk_to_int)
#define GENERATE_DUK_UNSIGNED_INT_GETTER(class, property_name, getter_func_name) GENERATE_DUK_GETTER(class, property_name, getter_func_name, duk_push_uint)
#define GENERATE_DUK_UNSIGNED_INT_SETTER(class, property_name, setter_func_name) GENERATE_DUK_SETTER(class, property_name, setter_func_name, duk_to_uint)
#define GENERATE_DUK_BOOLEAN_GETTER(class, property_name, getter_func_name) GENERATE_DUK_GETTER(class, property_name, getter_func_name, duk_push_boolean)
#define GENERATE_DUK_BOOLEAN_SETTER(class, property_name, setter_func_name) GENERATE_DUK_SETTER(class, property_name, setter_func_name, duk_to_boolean)
#define GENERATE_DUK_FLOAT_GETTER(class, property_name, getter_func_name) GENERATE_DUK_GETTER(class, property_name, getter_func_name, duk_push_number)
#define GENERATE_DUK_FLOAT_SETTER(class, property_name, setter_func_name) GENERATE_DUK_SETTER(class, property_name, setter_func_name, duk_to_number)

#define GENERATE_DUK_MEMBER_FUNCTION(class, func_name)                             \
    static duk_ret_t JS##func_name##Wrapper(duk_context* ctx)                      \
    {                                                                              \
        duk_push_this(ctx);                                                        \
        duk_get_prop_string(ctx, -1, JSObject::C_OBJECT_POINTER_KEY);              \
        class* component = static_cast<class*>(duk_to_pointer(ctx, -1));           \
        if (component) {                                                           \
            component->func_name();                                                \
        }                                                                          \
                                                                                   \
        return 0;                                                                  \
    }

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
            : uniqueID{AssignUniqueID()}
        {
        }

        Component(int32_t x, int32_t y, int32_t width, int32_t height)
            : uniqueID{AssignUniqueID()}
            , X(x)
            , Y(y)
            , Height(height)
            , Width(width)
        {
        }

        virtual ~Component() = default;

        void SetID(const char* id);
        void SetParentID(const char* id);
        virtual void SetPosition(int32_t x, int32_t y);
        virtual void SetX(int32_t x);
        virtual void SetY(int32_t y);
        virtual void SetSize(int32_t width, int32_t height);
        virtual void SetWidth(int32_t height);
        virtual void SetHeight(int32_t width);

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
        const char* GetParentID();
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

        virtual void PrepareContent(ContentManager* contentManager) {}
        virtual void CancelPreparingContent(ContentManager* contentManager) {}

        virtual void OnPress();
        virtual void OnRelease();
        virtual void OnClick();

        void SetOnPressEvent(const Event& event);
        void SetOnReleaseEvent(const Event& event);
        void SetOnClickEvent(const Event& event);
        void SetOnLongPressEvent(const Event& event);
        void SetOnLongPressRepeatEvent(const Event& event);

        virtual Touch::TouchResponse ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY,
                                                  int32_t modificator = 0);
        virtual bool IsTouchPointInObject(int32_t x, int32_t y, int32_t modificator = 0);

        virtual void CheckPlacement();
        virtual void ClearTouch();

        virtual void Draw(Painter& painter, int32_t ParentRenderX, int32_t ParentRenderY) = 0;

        bool CheckSizeValidity(Component* child) const;

        virtual void PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder);

        GENERATE_DUK_STRING_GETTER(Component, ID, GetID)
        GENERATE_DUK_STRING_GETTER(Component, ParentID, GetParentID)

        GENERATE_DUK_INT_GETTER(Component, X, GetX)
        GENERATE_DUK_INT_SETTER(Component, X, SetX)

        GENERATE_DUK_INT_GETTER(Component, Y, GetY)
        GENERATE_DUK_INT_SETTER(Component, Y, SetY)

        GENERATE_DUK_INT_GETTER(Component, Width, GetWidth)
        GENERATE_DUK_INT_SETTER(Component, Width, SetWidth)

        GENERATE_DUK_INT_GETTER(Component, Height, GetHeight)
        GENERATE_DUK_INT_SETTER(Component, Height, SetHeight)

        GENERATE_DUK_UNSIGNED_INT_GETTER(Component, ForegroundColor, GetForegroundColor)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Component, ForegroundColor, SetForegroundColor)

        GENERATE_DUK_UNSIGNED_INT_GETTER(Component, ActiveForegroundColor, GetActiveBackgroundColor)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Component, ActiveForegroundColor, SetActiveBackgroundColor)

        GENERATE_DUK_UNSIGNED_INT_GETTER(Component, BackgroundColor, GetBackgroundColor)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Component, BackgroundColor, SetBackgroundColor)

        GENERATE_DUK_UNSIGNED_INT_GETTER(Component, ActiveBackgroundColor, GetActiveBackgroundColor)
        GENERATE_DUK_UNSIGNED_INT_SETTER(Component, ActiveBackgroundColor, SetActiveBackgroundColor)

        GENERATE_DUK_BOOLEAN_GETTER(Component, Visible, IsVisible)
        GENERATE_DUK_BOOLEAN_SETTER(Component, Visible, SetVisible)

    protected:
        uint64_t AssignUniqueID();

        inline static uint64_t firstAvailableUniqueID{0};
        uint64_t uniqueID{0};

        std::string ID{};
        std::string parentID{};

        int32_t X{0};
        int32_t Y{0};
        int32_t Height{0};
        int32_t Width{0};

        uint32_t ForegroundColor{COLOR_ARGB8888_GRAY};
        uint32_t ActiveForegroundColor{COLOR_ARGB8888_GRAY};
        uint32_t BackgroundColor{COLOR_ARGB8888_TRANSPARENT};
        uint32_t ActiveBackgroundColor{COLOR_ARGB8888_TRANSPARENT};

        ComponentState State{ComponentState::Off};
        bool isFocused{false};
        Event onPress{};
        Event onRelease{};
        Event onClick{};
        bool touchActive{false};
        bool childDroppedTouch{false};
        Touch::TouchResponse previousResponse{Touch::TouchNotApplicable};
        uint64_t TouchActivatedTimestamp{0};
        bool longTouchActive{false};
        Event onLongPress{};
        Event onLongPressRepeat{};

        bool Visible{true};

        virtual Touch::TouchResponse ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY);
        Touch::TouchResponse OnMoveCase(const Touch& tp, int32_t ParentX, int32_t ParentY);

        virtual void InitFromXML(tinyxml2::XMLElement* xmlElement);
    };

} /* namespace grvl */

#endif /* GRVL_COMPONENT_H_ */
