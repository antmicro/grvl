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

#include <grvl/JSEngine.h>
#include <grvl/JSObjectBuilder.h>
#include <grvl/Manager.h>
#include <grvl/Misc.h>
#include <grvl/component/Component.h>
#include <grvl/grvl.h>

#include <algorithm>
#include <cassert>

namespace grvl {

    static std::unordered_map<std::string, ComponentFactory>& get_widget_registry()
    {
        static std::unordered_map<std::string, ComponentFactory> factories {};
        return factories;
    }

    static std::string normalized_widget_name(std::string_view name)
    {
        std::string key { name };
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        return key;
    }

    void register_component_factory(ComponentFactory factory, const char* name)
    {
        get_widget_registry()[normalized_widget_name(name)] = factory;
    }

    Component* component_from_xml(XmlElement xml)
    {
        return create_named_component(xml.Name(), xml);
    }

    Component* create_named_component(const char* name, XmlElement xml)
    {
        auto& factories = get_widget_registry();
        auto key = normalized_widget_name(name);
        auto it = factories.find(key);

        if(it != factories.end()) {
            return it->second(xml);
        }

        Log(ERROR, "No such component '%s' found in registry! List of known components:", key.c_str());

        for(auto entry : factories) {
            Log(INFO, " -- %s", entry.first.c_str());
        }

        return nullptr;
    }

    uint64_t Component::AssignUniqueID()
    {
        return firstAvailableUniqueID++;
    }

    Component::Component(const Component& other)
        : uniqueID { AssignUniqueID() }
        , parentID { other.parentID }
        , ID { other.ID }
        , X { other.X }
        , Y { other.Y }
        , Height { other.Height }
        , Width { other.Width }
        , ForegroundColor { other.ForegroundColor }
        , ActiveForegroundColor { other.ActiveForegroundColor }
        , BackgroundColor { other.BackgroundColor }
        , ActiveBackgroundColor { other.ActiveBackgroundColor }
        , onPress { other.onPress }
        , onRelease { other.onRelease }
        , onClick { other.onClick }
        , TouchActivatedTimestamp { other.TouchActivatedTimestamp }
        , longTouchActive { other.longTouchActive }
        , onLongPress { other.onLongPress }
        , onLongPressRepeat { other.onLongPressRepeat }
        , touchActive { other.touchActive }
        , childDroppedTouch { other.childDroppedTouch }
        , previousResponse { other.previousResponse }
        , Visible { other.Visible }
        , BorderColor { other.BorderColor }
        , BorderType { other.BorderType }
        , BorderArcRadius { other.BorderArcRadius }
    {
        onPress.SetSenderPointer(this);
        onRelease.SetSenderPointer(this);
        onClick.SetSenderPointer(this);
        onLongPress.SetSenderPointer(this);
        onLongPressRepeat.SetSenderPointer(this);
    }

    Component& Component::operator=(const Component& other)
    {
        if(this == &other) {
            return *this;
        }

        uniqueID = AssignUniqueID();
        parentID = other.parentID;
        ID = other.ID;
        X = other.X;
        Y = other.Y;
        Height = other.Height;
        Width = other.Width;
        ForegroundColor = other.ForegroundColor;
        ActiveForegroundColor = other.ActiveForegroundColor;
        BackgroundColor = other.BackgroundColor;
        ActiveBackgroundColor = other.ActiveBackgroundColor;
        onPress = other.onPress;
        onRelease = other.onRelease;
        onClick = other.onClick;
        touchActive = other.touchActive;
        childDroppedTouch = other.childDroppedTouch;
        previousResponse = other.previousResponse;
        Visible = other.Visible;
        BorderColor = other.BorderColor;
        BorderType = other.BorderType;
        BorderArcRadius = other.BorderArcRadius;
        TouchActivatedTimestamp = other.TouchActivatedTimestamp;
        longTouchActive = other.longTouchActive;
        onLongPress = other.onLongPress;
        onLongPressRepeat = other.onLongPressRepeat;

        onPress.SetSenderPointer(this);
        onRelease.SetSenderPointer(this);
        onClick.SetSenderPointer(this);
        onLongPress.SetSenderPointer(this);
        onLongPressRepeat.SetSenderPointer(this);

        return *this;
    }

    Component::~Component()
    {
        if(JavascriptObject != nullptr) {
            JSEngine::ForgetObject(JavascriptObject);
        }
    }

    Component* Component::Clone() const
    {
        assert(false && "Clone not implemented in derived class");
        return nullptr;
    }

    void Component::SetID(const char* id)
    {
        ID = std::string(id);
    }

    void Component::SetParentID(const char* id)
    {
        parentID = std::string(id);
    }

    const char* Component::GetID()
    {
        return ID.c_str();
    }

    const char* Component::GetParentID()
    {
        return parentID.c_str();
    }

    void Component::SetPosition(int32_t x, int32_t y)
    {
        if(X != x || Y != y) {
            X = x;
            Y = y;
        }
    }
    void Component::SetX(int32_t x)
    {
        X = x;
    }

    void Component::SetY(int32_t y)
    {
        Y = y;
    }

    void Component::SetWidth(int32_t width)
    {
        Width = width;
    }

    void Component::SetHeight(int32_t height)
    {
        Height = height;
    }

    uint32_t Component::GetBackgroundColor() const
    {
        return BackgroundColor;
    }

    uint32_t Component::GetForegroundColor() const
    {
        return ForegroundColor;
    }

    void Component::SetForegroundColor(uint32_t color)
    {
        ForegroundColor = color;
    }

    void Component::SetBackgroundColor(uint32_t color)
    {
        BackgroundColor = color;
    }

    void Component::SetActiveBackgroundColor(uint32_t color)
    {
        ActiveBackgroundColor = color;
    }

    void Component::SetActiveForegroundColor(uint32_t color)
    {
        ActiveForegroundColor = color;
    }

    uint32_t Component::GetActiveBackgroundColor() const
    {
        return ActiveBackgroundColor;
    }

    uint32_t Component::GetActiveForegroundColor() const
    {
        return ActiveForegroundColor;
    }

    void Component::SetBorderColor(uint32_t color)
    {
        BorderColor = color;
    }

    void Component::SetActiveBorderColor(uint32_t color)
    {
        ActiveBorderColor = color;
    }

    void Component::SetBorderType(BorderTypeBits type)
    {
        BorderType = type;
    }

    void Component::SetBorderArcRadius(uint32_t radius)
    {
        BorderArcRadius = radius;
    }

    void Component::SetVisible(bool state)
    {
        Visible = state;
    }
    bool Component::IsVisible() const
    {
        return Visible;
    }

    void Component::Hide()
    {
        Visible = false;
    }

    void Component::Show()
    {
        Visible = true;
    }

    uint32_t Component::GetCurrentBackgroundColor()
    {
        if(State == Released) {
            if(State == Off) {
                return BackgroundColor;
            }
            if(State == On) {
                return ActiveBackgroundColor;
            }
        }

        return BackgroundColor;
    }

    uint32_t Component::GetCurrentForegroundColor()
    {
        if(State == Off) {
            return ForegroundColor;
        }
        if(State == On) {
            return ActiveForegroundColor;
        }
        return ForegroundColor;
    }

    int32_t Component::GetX() const
    {
        return X;
    }

    int32_t Component::GetY() const
    {
        return Y;
    }

    int32_t Component::GetWidth() const
    {
        return Width;
    }

    int32_t Component::GetHeight() const
    {
        return Height;
    }

    void Component::SetSize(int32_t width, int32_t height)
    {
        if(Width != width || Height != height) {
            Width = width;
            Height = height;
        }
    }

    bool Component::IsTouchPointInObject(int32_t x, int32_t y, int32_t modificator)
    {
        if(!Visible) {
            return false;
        }

        return (bool)(x > X - modificator && x < (X + Width + modificator) && y > Y - modificator
                      && y < (Y + Height + modificator));
    }

    Touch::TouchResponse Component::ProcessTouch(const Touch& tp, int32_t ParentX, int32_t ParentY, int32_t modificator)
    {
        if(tp.GetState() == Touch::Pressed) { // init state
            childDroppedTouch = false;
            touchActive = false;
        }

        if(!touchActive && IsTouchPointInObject(tp.GetStartX() - ParentX, tp.GetStartY() - ParentY, modificator)) { // Focus activated - element clicked
            touchActive = true;
        }

        if(!touchActive || childDroppedTouch) { // Touch not applicable
            return Touch::TouchNotApplicable;
        }

        Touch::TouchResponse response = Touch::TouchHandled;

        // Touch Processing
        switch(tp.GetState()) {
            case Touch::Pressed: // First OnPress() then OnMoveCase()
                OnPress();
            case Touch::Moving:
                response = OnMoveCase(tp, ParentX, ParentY);
                break;
            case Touch::Released:
                touchActive = false;
                OnRelease();
                if(previousResponse != Touch::LongTouchHandled
                   && IsTouchPointInObject(tp.GetCurrentX() - ParentX, tp.GetCurrentY() - ParentY, modificator)) {
                    OnClick();
                }
                break;
            default:
                break;
        }
        if(!touchActive)
            response = Touch::TouchReleased;

        previousResponse = response;

        return response;
    }

    Touch::TouchResponse Component::OnMoveCase(const Touch& tp, int32_t ParentX, int32_t ParentY)
    {

        Touch::TouchResponse response = Touch::TouchHandled;
        response = ProcessMove(tp.GetStartX() - ParentX - X, tp.GetStartY() - ParentY - Y, tp.GetDeltaX(), tp.GetDeltaY());
        if(response != Touch::TouchHandled && response != Touch::LongTouchHandled) { // Move not handled
            OnRelease();
            childDroppedTouch = true;
            response = Touch::TouchReleased;
        }
        return response;
    }

    bool Component::ShouldCancelTouchOnMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY) const
    {
        static constexpr auto moveScale = 10;
        return DeltaX > ceil(Width / moveScale + 1)
            || DeltaX < -ceil(Width / moveScale + 1)
            || DeltaY > ceil(Height / moveScale + 1)
            || DeltaY < -ceil(Height / moveScale + 1);
    }

    Touch::TouchResponse Component::ProcessMove(int32_t StartX, int32_t StartY, int32_t DeltaX, int32_t DeltaY)
    {
        static constexpr auto moveScale = 10;
        if(ShouldCancelTouchOnMove(StartX, StartY, DeltaX, DeltaY)) {
            return Touch::TouchReleased;
        }

        static constexpr auto longpressThreshold = 1000;
        const int64_t now = grvl::Callbacks()->get_timestamp();
        if(!longTouchActive
           && onLongPress.IsSet()
           && TouchActivatedTimestamp < (now - longpressThreshold)) { // Long press
            longTouchActive = true;
            TouchActivatedTimestamp = now;
            Manager::GetInstance().GetEventsQueueInstance().push(&onLongPress);
        }

        static constexpr auto longpressRepeatOffset = 500;
        if(longTouchActive && TouchActivatedTimestamp < (grvl::Callbacks()->get_timestamp() - longpressRepeatOffset)) { // Long press repeat
            TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
            Manager::GetInstance().GetEventsQueueInstance().push(&onLongPressRepeat);
        }

        if(longTouchActive) {
            return Touch::LongTouchHandled;
        }
        return Touch::TouchHandled;
    }

    void Component::SetState(ComponentState state)
    {
        State = state;
    }

    Component::ComponentState Component::GetState()
    {
        return State;
    }

    void Component::OnPress()
    {
        State = On;
        TouchActivatedTimestamp = grvl::Callbacks()->get_timestamp();
        Manager::GetInstance().GetEventsQueueInstance().push(&onPress);
    }

    void Component::OnRelease()
    {
        State = Off;
        TouchActivatedTimestamp = 0;
        longTouchActive = false;
        Manager::GetInstance().GetEventsQueueInstance().push(&onRelease);
    }

    void Component::OnClick()
    {
        Manager::GetInstance().GetEventsQueueInstance().push(&onClick);
    }

    void Component::SetOnPressEvent(const Event& event)
    {
        onPress = event;
        onPress.SetSenderPointer(this);
    }

    void Component::SetOnReleaseEvent(const Event& event)
    {
        onRelease = event;
        onRelease.SetSenderPointer(this);
    }

    void Component::SetOnClickEvent(const Event& event)
    {
        onClick = event;
        onClick.SetSenderPointer(this);
    }

    void Component::SetOnLongPressEvent(const Event& event)
    {
        onLongPress = event;
        onLongPress.SetSenderPointer(this);
    }

    void Component::SetOnLongPressRepeatEvent(const Event& event)
    {
        onLongPressRepeat = event;
        onLongPressRepeat.SetSenderPointer(this);
    }

    void Component::ClearTouch()
    {
        longTouchActive = false;
        OnRelease();
    }

    void Component::CheckPlacement()
    {
    }

    bool Component::CheckSizeValidity(Component* child) const
    {
        if(!child) {
            return false;
        }
        if(child->GetX() < 0
           || child->GetY() < 0
           || (child->GetX() + child->GetWidth()) > GetWidth()
           || (child->GetY() + child->GetHeight()) > GetHeight()) {
            return false;
        }
        return true;
    }

    void Component::InitFromXML(XmlElement xml)
    {
        Manager* man = &Manager::GetInstance();

        this->X = xml.Attribute("x").ElseStyle().Parse<uint32_t>().GetOr(0);
        this->Y = xml.Attribute("y").ElseStyle().Parse<uint32_t>().GetOr(0);
        this->Width = xml.Attribute("width").ElseStyle().Parse<uint32_t>().GetOr(0);
        this->Height = xml.Attribute("height").ElseStyle().Parse<uint32_t>().GetOr(0);

        if(auto id = xml.Attribute("id")) {
            this->SetID(id.Get());
        }

        this->SetVisible(xml.Attribute("visible").ElseStyle().Parse<bool>().GetOr(true));

        SetBackgroundColor(xml.Attribute("backgroundColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_TRANSPARENT));
        SetActiveBackgroundColor(xml.Attribute("activeBackgroundColor").ElseStyle().ParseColor().GetOr(GetBackgroundColor()));
        SetForegroundColor(xml.Attribute("foregroundColor").ElseStyle().ParseColor().GetOr(COLOR_ARGB8888_WHITE));
        SetActiveForegroundColor(xml.Attribute("activeForegroundColor").ElseStyle().ParseColor().GetOr(GetForegroundColor()));

        SetBorderColor(xml.Attribute("borderColor").ElseStyle().ParseColor().GetOr(GetBackgroundColor()));
        SetActiveBorderColor(xml.Attribute("activeBorderColor").ElseStyle().ParseColor().GetOr(GetActiveBackgroundColor()));
        SetBorderType(xml.Attribute("borderType").ElseStyle().ParseEnum(BORDER_TYPE_BITS_ENUM).GetOr(BorderTypeBits::NONE));
        SetBorderArcRadius(xml.Attribute("borderArcRadius").ElseStyle().Parse<uint32_t>().GetOr(0));

        this->SetOnClickEvent(man->GetOrCreateCallback(xml.Attribute("onClick").ParseCallback().GetOr({})));
        this->SetOnReleaseEvent(man->GetOrCreateCallback(xml.Attribute("onRelease").ParseCallback().GetOr({})));
        this->SetOnPressEvent(man->GetOrCreateCallback(xml.Attribute("onPress").ParseCallback().GetOr({})));
    }

    void Component::AddMetadata(std::string key, std::string value)
    {
        metadata.emplace(std::move(key), std::move(value));
    }

    const char* Component::GetMetadata(const char* key)
    {
        if(metadata.count(key) == 0) {
            return "";
        }

        return metadata.at(key).c_str();
    }

    void Component::PushJSObjectOnStack(duk_context* ctx)
    {
        if(JavascriptObject != nullptr) {
            duk_push_heapptr(ctx, JavascriptObject);
            return;
        }
        JSObjectBuilder jsObjectBuilder { ctx, this };
        JavascriptObject = duk_get_heapptr(ctx, -1);
        JSEngine::RememberObject(JavascriptObject);
        PopulateJavaScriptObject(jsObjectBuilder);
    }

    void Component::PopulateJavaScriptObject(JSObjectBuilder& jsObjectBuilder)
    {
        jsObjectBuilder.AddProperty("name", Component::JSGetIDWrapper);
        jsObjectBuilder.AddProperty("parentName", Component::JSGetParentIDWrapper);
        jsObjectBuilder.AddProperty("x", Component::JSGetXWrapper, Component::JSSetXWrapper);
        jsObjectBuilder.AddProperty("y", Component::JSGetYWrapper, Component::JSSetYWrapper);
        jsObjectBuilder.AddProperty("width", Component::JSGetWidthWrapper, Component::JSSetWidthWrapper);
        jsObjectBuilder.AddProperty("height", Component::JSGetHeightWrapper, Component::JSSetHeightWrapper);
        jsObjectBuilder.AddProperty("foregroundColor", Component::JSGetForegroundColorWrapper, Component::JSSetForegroundColorWrapper);
        jsObjectBuilder.AddProperty("activeForegroundColor", Component::JSGetActiveForegroundColorWrapper, Component::JSSetActiveForegroundColorWrapper);
        jsObjectBuilder.AddProperty("backgroundColor", Component::JSGetBackgroundColorWrapper, Component::JSSetBackgroundColorWrapper);
        jsObjectBuilder.AddProperty("activeBackgroundColor", Component::JSGetActiveBackgroundColorWrapper, Component::JSSetActiveBackgroundColorWrapper);
        jsObjectBuilder.AddProperty("borderColor", Component::JSGetBorderColorWrapper, Component::JSSetBorderColorWrapper);
        jsObjectBuilder.AddProperty("activeBorderColor", Component::JSGetActiveBorderColorWrapper, Component::JSSetActiveBorderColorWrapper);
        jsObjectBuilder.AddProperty("visibility", Component::JSGetVisibleWrapper, Component::JSSetVisibleWrapper);
        jsObjectBuilder.AttachMemberFunction("Clone", Component::JSCloneWrapper);
        jsObjectBuilder.AttachMemberFunction("AddMetadata", Component::JSAddMetadataWrapper, 2);
        jsObjectBuilder.AttachMemberFunction("GetMetadata", Component::JSGetMetadataWrapper, 1);
    }

    duk_ret_t Component::JSCloneWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSEngine::C_OBJECT_POINTER_KEY);
        Component* component = static_cast<Component*>(duk_to_pointer(ctx, -1));
        if(component) {
            Component* clonedComponent = component->Clone();
            clonedComponent->PushJSObjectOnStack(ctx);

            return 1;
        }

        return 0;
    }

    duk_ret_t Component::JSAddMetadataWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSEngine::C_OBJECT_POINTER_KEY);
        Component* component = static_cast<Component*>(duk_to_pointer(ctx, -1));
        if(!component) {
            return 0;
        }

        const char* keyName = duk_to_string(ctx, 0);
        const char* valueName = duk_to_string(ctx, 1);
        component->AddMetadata(keyName, valueName);

        return 0;
    }

    duk_ret_t Component::JSGetMetadataWrapper(duk_context* ctx)
    {
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, JSEngine::C_OBJECT_POINTER_KEY);
        Component* component = static_cast<Component*>(duk_to_pointer(ctx, -1));
        if(!component) {
            return 0;
        }

        const char* keyName = duk_to_string(ctx, 0);
        duk_push_string(ctx, component->GetMetadata(keyName));

        return 1;
    }

    void Component::DrawBorderIfNecessary(Painter& painter, int32_t StartX, int32_t StartY, int32_t BorderWidth, int32_t BorderHeight)
    {
        uint32_t tempBorderColor = 0;

        if(State == On || State == Pressed || State == OnAndSelected) {
            tempBorderColor = ActiveBorderColor;
        } else if(State == Off || State == Released || State == OffAndSelected) {
            tempBorderColor = BorderColor;
        }

        switch(BorderType) {
            case BorderTypeBits::BOX: {
                if(BorderArcRadius > 0) {
                    painter.DrawRoundRectangle(StartX, StartY, BorderWidth, BorderHeight, tempBorderColor, BorderArcRadius);
                } else {
                    painter.DrawRectangle(StartX, StartY, BorderWidth, BorderHeight, tempBorderColor);
                }
                break;
            }
            case BorderTypeBits::TOP: {
                painter.DrawHLine(StartX, StartY, BorderWidth, tempBorderColor);
                break;
            }
            case BorderTypeBits::RIGHT: {
                painter.DrawVLine(StartX + BorderWidth - 1, StartY, BorderHeight, tempBorderColor);
                break;
            }
            case BorderTypeBits::BOTTOM: {
                painter.DrawHLine(StartX, StartY + BorderHeight - 1, BorderWidth, tempBorderColor);
                break;
            }
            case BorderTypeBits::LEFT: {
                painter.DrawVLine(StartX, StartY, BorderHeight, tempBorderColor);
                break;
            }
            default:
                break;
        }
    }

} /* namespace grvl */
