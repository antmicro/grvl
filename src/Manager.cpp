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

#include "Manager.h"
// #include "ListItem.h"
#include "ParsingUtils.h"

#include <JSEngine.h>

#ifdef __ZEPHYR__
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MANAGER, LOG_LEVEL_INF);
#endif

char* strdup (const char* s) // TODO: probably temporary
{
  size_t slen = strlen(s);
  char* result = (char*)malloc(slen + 1);
  if(result == NULL)
  {
    return NULL;
  }

  memcpy(result, s, slen+1);
  return result;
}

namespace grvl {

    // Widget list
    WIDGET(Button);
    WIDGET(Clock);
    WIDGET(Label);
    WIDGET(Image);
    WIDGET(CircleProgressBar);
    WIDGET(GridView);
    WIDGET(CustomView);
    WIDGET(ListView);
    WIDGET(ListItem);
    WIDGET(GridRow);
    WIDGET(Panel);
    WIDGET(ProgressBar);
    WIDGET(Slider);
    WIDGET(SwitchButton);
    WIDGET(ScrollPanel);

    static inline float MotionFunc(float x)
    {
        return (sin(3.1415 * x - 1.57) + 1) / 2; //NOLINT(readability-magic-numbers)
    }

    static void ClosePopupCallback(void* sender, const Event::ArgVector& Args)
    {
        Manager* man = &Manager::GetInstance();
        man->ClosePopup();
    }

    static void ShowPopupCallback(void* sender, const Event::ArgVector& Args)
    {
        Manager* man = &Manager::GetInstance();
        man->ShowPopup(Args[0].c_str());
    }

    static void ChangeScreenCallback(void* sender, const Event::ArgVector& Args)
    {
        int8_t screen_animation = 0;
        Manager* man = &Manager::GetInstance();

        if(Args.size() > 1) {
            char* arg1 = strdup(Args[1].c_str());
            string_to_lower(arg1);
            // TODO: convert to lowercase and then compare
            if(strcmp(arg1, "animate_left") == 0)
                screen_animation = -1;
            if(strcmp(arg1, "animate_right") == 0)
                screen_animation = 1;
            if(strcmp(arg1, "animate_none") == 0)
                screen_animation = 0;
            if(strcmp(arg1, "animate_crossfade") == 0)
                screen_animation = 2;
            free(arg1);
        }
        man->SetActiveScreen(Args[0].c_str(), screen_animation);
    }

    void* Manager::operator new(size_t size)
    {
        return (void*)grvl::Callbacks()->malloc(size);
    }

    void Manager::operator delete(void* ptr)
    {
        grvl::Callbacks()->free(ptr);
    }
    static constexpr auto defaultScrollingDuration = 400;
    Manager::Manager(uint32_t xSize, uint32_t ySize, int bpp, bool rotate90, uint8_t* framebuffer)
        : flips(0)
        , TopPanel(0)
        , BottomPanel(NULL)
        , StartHeader(NULL)
        , TargetHeader(NULL)
        , ActiveScreen(NULL)
        , TargetScreen(NULL)
        , AnimationWindowOffset(0)
        , CurrentTimestamp(0)
        , BeginTimestamp(0)
        , PointerTimestamp(0)
        , KeyPressTimestamp(0)
        , ManagerState(Loading)
        , LoadingImageVisible(true)
        , CurrentPopup(NULL)
        , ScrollingDuration(defaultScrollingDuration)
        , previousTouchState(false)
        , touchToPopup(false)
        , timeoutedPopupMode(false)
        , longPressActive(false)
        , keyActive(false)
        , touchAreaModificator(0)
        , dotColor(0x00000000)
        , dotActiveColor(0x00000000)
        , dotDistance(0)
        , dotRadius(0)
        , dotYPos(0)
        , debugDot(0)
        , initialTransparency(1.0)
        , desiredTransparency(1.0)
        , currentTransparency(1.0)
        , fadeBeginTimestamp(0)
        , fadeEndTimestamp(0)
    {

        width = xSize;
        height = ySize;

        DrawMutex = grvl::Callbacks()->mutex_create();

        painter.SetRotation(rotate90);

        painter.SetBackgroundImage(&BackgroundImage);
        painter.SetContentManager(&contentManager);
        painter.SetDisplaySize(xSize, ySize);

        if (framebuffer)
            painter.CreateFramebuffersCollection(bpp, framebuffer);
        else
            painter.CreateFramebuffersCollection(bpp);

        painter.InitFramebuffersCollection();
        static constexpr auto defaultBGColor = 0xFF000000;
        painter.SetBackgroundColor(defaultBGColor);

        AddCallbackToContainer("ChangeScreen", ChangeScreenCallback);
        AddCallbackToContainer("ClosePopup", ClosePopupCallback);
        AddCallbackToContainer("ShowPopup", ShowPopupCallback);
    }

    Manager::EventQueue& Manager::GetEventsQueueInstance()
    {
        return eventsQueue;
    }

    uint32_t Manager::GetWidth() const
    {
        return width;
    }

    uint32_t Manager::GetHeight() const
    {
        return height;
    }

    Manager& Manager::SetBackgroundColor(uint32_t color)
    {
        painter.SetBackgroundColor(color);
        return *this;
    }

    Manager& Manager::SetTransparency(float value, uint32_t milliseconds)
    {
        if(value <= 1.0 && value >= 0.0) {
            desiredTransparency = value;
            initialTransparency = currentTransparency;
            fadeBeginTimestamp = grvl::Callbacks()->get_timestamp();
            if(milliseconds == 0) {
                desiredTransparency = currentTransparency = initialTransparency = value;
            } else {
                fadeEndTimestamp = fadeBeginTimestamp + milliseconds;
            }
        }
        return *this;
    }

    Manager& Manager::SetExternalContentRequestCallback(ContentManager::ContentCallback requestCallback)
    {
        contentManager.SetRequestCallback(requestCallback);
        return *this;
    }

    Manager& Manager::SetCancelExternalContentRequestCallback(ContentManager::ContentCallback cancelRequestCallback)
    {
        contentManager.SetCancelRequestCallback(cancelRequestCallback);
        return *this;
    }

    Manager::~Manager()
    {
        Screens.clear();
        FontContainer.clear();
    }

    Panel* Manager::GetTopPanel()
    {
        return TopPanel;
    }

    Panel* Manager::GetBottomPanel()
    {
        return BottomPanel;
    }

    vector<AbstractView*>& Manager::GetScreensCollection()
    {
        return Screens;
    }

    AbstractView* Manager::GetLastScreen()
    {
        return Screens.back();
    }

    Manager& Manager::AddScreen(AbstractView* screen)
    {
        Screens.push_back(screen);
        Screens.back()->SetSize(width, height - GetTopPanelHeight() - GetBottomPanelHeight());
        return *this;
    }

    Manager& Manager::AddPopup(Popup* popup)
    {
        if(popup) {
            PopupsContainer.push_back(popup);
        }
        return *this;
    }

    Manager& Manager::SetActiveScreen(const char* activeScreenId, int8_t direction)
    {
        if(!Screens.empty()) {
            AbstractView* NewScreen;

            for(uint32_t i = 0; i < Screens.size(); i++) {
                if(strcmp(Screens[i]->GetID(), activeScreenId) == 0) {
                    if(ActiveScreen) {
                        ActiveScreen->ClearTouch();
                        ActiveScreen->PrepareToClose();
                    }

                    NewScreen = Screens[i];
                    NewScreen->PrepareToOpen();

                    Screens[i]->CheckPlacement();
                    if(direction != 0 && ScrollingDuration > 0) {
                        Animate(ActiveScreen, NewScreen, direction);
                    } else {
                        ActiveScreen = NewScreen;

                        // In case of animation this happens after the animation ends
                        if(TopPanel) {
                            TopPanel->SetVisible(ActiveScreen->GetGlobalPanelVisibility());
                        }
                    }
                    return *this;
                }
            }
        }
        grvl::Log("[ERROR] Screen \"%s\" does not exist!", activeScreenId);
        return *this;
    }

    void Manager::Animate(AbstractView* startScreen, AbstractView* targetScreen, int8_t direction)
    {
        int32_t startHeadersHeight = 0;
        int32_t targetHeadersHeight = 0;
        int32_t headerHeight = 0;

        StartHeader = startScreen->GetHeader();
        TargetHeader = targetScreen->GetHeader();
        TargetScreen = targetScreen;

        if(TopPanel) {
            headerHeight = TopPanel->GetHeight();
        }

        if(StartHeader) {
            startHeadersHeight = StartHeader->GetHeight();
        }
        startHeadersHeight += headerHeight * startScreen->GetGlobalPanelVisibility();

        if(TargetHeader) {
            targetHeadersHeight = TargetHeader->GetHeight();
        }
        targetHeadersHeight += headerHeight * targetScreen->GetGlobalPanelVisibility();

        switch(direction) {
            case 1:
                ManagerState = ToTheRight;
                break;
            case -1:
                ManagerState = ToTheLeft;
                break;
            case 2:
                ManagerState = CrossFade;
                break;
            case 0:
            default:
                ManagerState = Refreshing;
                AnimationWindowOffset = 0;
                return;
        }

        startScreen->SetSize(width, height - GetBottomPanelHeight() - startHeadersHeight);
        targetScreen->SetSize(width, height - GetBottomPanelHeight() - targetHeadersHeight);

        AnimationWindowOffset = width;

        painter.SetActiveBuffer((direction == 1) ? 1 : 0);
        startScreen->Draw(
            painter, 0, startHeadersHeight);

        painter.SetActiveBuffer((direction == 1) ? 0 : 1);
        targetScreen->Draw(
            painter, 0, targetHeadersHeight);

        if(targetHeadersHeight) { // Fill space under header
            painter.FillRectangle(0, (uint32_t)GetTopPanelHeight(), (uint32_t)targetScreen->GetWidth(),
                                  (uint32_t)targetHeadersHeight, (uint32_t)targetScreen->GetBackgroundColor());
        }

        BeginTimestamp = grvl::Callbacks()->get_timestamp();
    }

    Manager& Manager::SetLoadingImage(const Image& image)
    {
        LoadingImage = image;
        return *this;
    }

    void Manager::InitializationFinished()
    {
        ManagerState = Refreshing;
        SetLoadingImageVisibility(0);
        Draw();

#ifdef ZEPHYR
        LOG_HEXDUMP_INF((void*)painter.GetBuffer(0), 64, "p0");
        LOG_HEXDUMP_INF((void*)painter.GetBuffer(1), 64, "p1");
        LOG_HEXDUMP_INF((void*)painter.GetBuffer(2), 64, "p2");
        LOG_HEXDUMP_INF((void*)painter.GetBuffer(3), 64, "p3");
#endif
    }

    Manager& Manager::SetLoadingImageVisibility(uint8_t visible)
    {
        LoadingImageVisible = visible;
        if(!LoadingImage.IsEmpty()) {
            LoadingImage.SetActiveFrame(0);
        }
        return *this;
    }

    void Manager::DrawNextLoadingFrame()
    {
        if(!LoadingImage.IsEmpty() && LoadingImageVisible) {
            int frames = LoadingImage.GetContent()->GetNumberOfFrames();
            if(frames > 1) {
                static constexpr auto freqTwicePerSecond = 500;
                uint32_t activeFrame = (grvl::Callbacks()->get_timestamp() % freqTwicePerSecond) / ceil((double)freqTwicePerSecond / frames);
                LoadingImage.SetActiveFrame(activeFrame);
            }
            LoadingImage.Draw(painter, 0, 0);
            if(!LogoImage.IsEmpty()) {
                LogoImage.Draw(painter, 0, 0);
            }
        }
    }

    Manager& Manager::SetBackgroundImage(const Image& image)
    {
        BackgroundImage = image;
        if(painter.IsRotated()) {
            if(!BackgroundImage.GetContent()->IsRotated()) {
                BackgroundImage.GetContent()->Rotate90();
            }
        }
        BackgroundImage.SetPosition(0, 0);
        painter.SetBackgroundImage(&BackgroundImage);
        return *this;
    }

    Manager& Manager::SetCollectionImage(const Image& image)
    {
        CollectionImage = image;
        CollectionImage.SetPosition(0, 0);
        dotRadius = CollectionImage.GetWidth() / 2;
        return *this;
    }

    Image& Manager::GetBackgroundImage()
    {
        return BackgroundImage;
    }

    AbstractView* Manager::GetActiveScreen()
    {
        return ActiveScreen;
    }

    Manager& Manager::AddImageContentToContainer(string name, ImageContent* image)
    {
        if(painter.IsRotated())
            if(!image->IsRotated())
                image->Rotate90();
        if(IsInitializationFinished()) {
            contentManager.AddExternalImageContent(name, image);
        } else {
            contentManager.AddInternalImageContent(name, image);
        }
        return *this;
    }

    Manager& Manager::BindImageContentToImage(const string& contentName, Image* image)
    {
        contentManager.BindImageContentToImage(contentName, image);
        return *this;
    }

    Manager& Manager::AddCallbackToContainer(const string& name, Event::CallbackPointer Callback)
    {
        if(Callback == NULL) {
            return *this;
        }
        EventsContainer[name] = Callback;
        return *this;
    }

    Event::CallbackPointer Manager::GetCallbackFromContainer(const string& name) const
    {
        EventsContainerMap::const_iterator searchImage = EventsContainer.find(name);
        if(searchImage != EventsContainer.end()) {
            return searchImage->second;
        }
        return NULL;
    }
    
    Event Manager::GetOrCreateCallback(const std::string& callbackFunctionName, const Event::ArgVector& callbackArgs)
    {
        if (Event::CallbackPointer foundCallback = GetCallbackFromContainer(callbackFunctionName)) {
            return Event{foundCallback, callbackArgs};
        }

        return JSEngine::CreateJavaScriptFunctionCallEvent(callbackFunctionName, callbackArgs);
    }

    Event Manager::GetOrCreateCallback(const CallbackDefinition& callbackDefinition)
    {
        return GetOrCreateCallback(callbackDefinition.functionName, callbackDefinition.args);
    }

    int8_t Manager::IsInitializationFinished()
    {
        return ManagerState != Loading;
    }

    Manager& Manager::AddFontToFontContainer(const string& name, Font* font)
    {
        FontContainer[name] = font;
        return *this;
    }

    void Manager::AddKeyMappingToContainer(uint16_t mask, const string& name, uint32_t repeat)
    {
        if(mask == 0x0) {
            return;
        }
        KeyData k;
        k.name = name; //NOLINT
        k.repeat = repeat;
        KeyMappingContainer[mask] = k;
    }

    Font const* Manager::GetFontFromContainer(const char* name) const
    {
        if(name == NULL || *name == 0) {
            return NULL;
        }
        FontContainerMap::const_iterator searchFont = FontContainer.find(name);
        if(searchFont != FontContainer.end()) {
            return searchFont->second;
        }
        return NULL;
    }

    Font const* Manager::GetDefaultFontFromContainer() const
    {
        if(!FontContainer.empty()) {
            FontContainerMap::const_iterator searchFont = FontContainer.find("default");
            if(searchFont != FontContainer.end()) {
                return searchFont->second;
            }
        }
        grvl::Log("[ERROR] Default font doesn't exist.");
        return NULL;
    }

    Font const* Manager::GetFontPointer(const char* fontName) const
    {
        Font const* tfont = 0;
        if(fontName) { // Font is defined
            tfont = GetFontFromContainer(fontName); // Most frequent case
        }
        if(!tfont) {
            grvl::Log("[WARNING] Font \"%s\" doesn't exist. Using \"normal\"", fontName);
            tfont = GetFontFromContainer("normal");
        }
        if(!tfont) {
            grvl::Log("[WARNING] Font \"normal\" doesn't exist. Using default font");
            tfont = GetDefaultFontFromContainer();
        }
        if(!tfont) {
            grvl::Log("[ERROR] Default font not found!");
        }
        return tfont;
    }

    AbstractView* Manager::GetScreen(const char* id)
    {
        AbstractView* Screen = NULL;
        for(uint32_t i = 0; i < Screens.size(); i++) {
            if(strcmp(Screens[i]->GetID(), id) == 0) {
                Screen = Screens[i];
                break;
            }
        }
        return Screen;
    }

    AbstractView* Manager::GetScreen(int id)
    {
        if((int)Screens.size() > id) {
            return Screens[id];
        }
        return NULL;
    }

    Popup* Manager::GetPopupInstance()
    {
        return CurrentPopup;
    }

    void Manager::ShowPopup(const char* Style, const char* Message)
    {
        Popup* tempPopup = GetPopupFromContainer(Style);
        if(CurrentPopup != NULL && tempPopup != CurrentPopup) {
            return; // TODO: implement popup queue
        }
        CurrentPopup = tempPopup;

        if(CurrentPopup) {
            CurrentPopup->SetMessage(Message);
            ShowPopup();
        }
    }

    void Manager::ShowPopup(const char* Style)
    {
        Popup* tempPopup = GetPopupFromContainer(Style);
        if(CurrentPopup != NULL && tempPopup != CurrentPopup) {
            return; // TODO: implement popup queue
        }
        CurrentPopup = tempPopup;

        if(CurrentPopup) {
            ShowPopup();
        }
    }

    void Manager::ShowPopup(const char* Style, uint32_t milliseconds)
    {
        Popup* tempPopup = GetPopupFromContainer(Style);
        if(CurrentPopup != NULL && tempPopup != CurrentPopup) {
            return; // TODO: implement popup queue
        }
        CurrentPopup = tempPopup;

        if(CurrentPopup) {
            timeoutedPopupMode = true;
            CurrentPopup->SetTimestamp(grvl::Callbacks()->get_timestamp() + milliseconds);
            ShowPopup();
        }
    }

    void Manager::ShowPopup(const char* Style, const char* Message, uint32_t milliseconds)
    {
        Popup* tempPopup = GetPopupFromContainer(Style);
        if(CurrentPopup != NULL && tempPopup != CurrentPopup) {
            return; // TODO: implement popup queue
        }
        CurrentPopup = tempPopup;

        if(CurrentPopup) {
            timeoutedPopupMode = true;
            CurrentPopup->SetTimestamp(grvl::Callbacks()->get_timestamp() + milliseconds);
            CurrentPopup->SetMessage(Message);
            ShowPopup();
        }
    }

    Manager& Manager::KeepPopupVisible(uint32_t milliseconds)
    {
        if(CurrentPopup && timeoutedPopupMode) {
            CurrentPopup->SetTimestamp(grvl::Callbacks()->get_timestamp() + milliseconds);
        }
        return *this;
    }

    void Manager::ShowPopup()
    {
        if(CurrentPopup) {
            ActiveScreen->ClearTouch();
            CurrentPopup->Show();
            Draw();
        }
    }

    bool Manager::GetGlobalTopPanelVisibility()
    {
        if(ActiveScreen) {
            return ActiveScreen->GetGlobalPanelVisibility();
        }
        return true;
    }
    int32_t Manager::GetTopPanelHeight()
    {
        if(TopPanel && TopPanel->IsVisible() && GetGlobalTopPanelVisibility()) {
            return TopPanel->GetHeight();
        }
        return 0;
    }

    int32_t Manager::GetTotalHeadersHeight()
    {
        int32_t topHeight = GetTopPanelHeight();
        int32_t headerHeight = 0;
        if(ActiveScreen && ActiveScreen->GetHeader() && ActiveScreen->GetHeader()->IsVisible()) {
            headerHeight = ActiveScreen->GetHeader()->GetHeight();
        }

        return topHeight + headerHeight;
    }

    int32_t Manager::GetBottomPanelHeight()
    {
        if(BottomPanel && BottomPanel->IsVisible()) {
            return BottomPanel->GetHeight();
        }
        return 0;
    }

    Manager& Manager::SetTouchAreaModificator(int32_t modificator)
    {
        touchAreaModificator = modificator;
        return *this;
    }

    int32_t Manager::GetTouchAreaModificator(void) const
    {
        return touchAreaModificator;
    }

    void Manager::ClosePopup()
    {
        if(CurrentPopup && CurrentPopup->IsVisible()) {
            CurrentPopup->Hide();
            CurrentPopup = NULL;
            timeoutedPopupMode = false;
            Draw();
        }
    }

    Manager& Manager::SetPopupPointer(Popup* popup)
    {
        CurrentPopup = popup;
        return *this;
    }
    bool Manager::IsPopupVisible()
    {
        return CurrentPopup->IsVisible();
    }

    Manager& Manager::SetScrollingDuration(uint32_t milliseconds)
    {
        static constexpr auto maxScrollingDuration = 30000;
        if(milliseconds > maxScrollingDuration) {
            milliseconds = maxScrollingDuration;
        }
        ScrollingDuration = milliseconds;
        return *this;
    }

    uint32_t Manager::GetScrollingDuration(void) const
    {
        return ScrollingDuration;
    }

    Manager::State Manager::GetState()
    {
        return ManagerState;
    }

    uintptr_t Manager::GetFramebufferPointer(Manager::BufferName name) const
    {
        return painter.GetBuffer(name);
    }

    int Manager::LockDrawMutex()
    {
        if(grvl::Callbacks()->mutex_lock != NULL) {
            return grvl::Callbacks()->mutex_lock(DrawMutex);
        }
        return 0;
    }

    void Manager::UnlockDrawMutex()
    {
        if(grvl::Callbacks()->mutex_unlock != NULL) {
            grvl::Callbacks()->mutex_unlock(DrawMutex);
        }
    }

    void Manager::Draw()
    {
        if(LockDrawMutex() != 0) {
            return;
        }

        painter.ResetDrawingBounds();

        switch(ManagerState) {
            case Loading: {
                painter.SetActiveBuffer(painter.GetSwapperValue() ? 2 : 3);
                painter.FillRectangle(0, 0, width, height, painter.GetBackgroundColor());
                DrawNextLoadingFrame();
                ApplyTransparency();
                painter.FlipSynchronizeBuffers();
                UnlockDrawMutex();
                return;
                break;
            }
            case CrossFade: {
                if(UpdateAnimationWindowOffset()) {
                    // TODO: do the actual blending.
                }
                if(AnimationWindowOffset <= 0) {
                    ActiveScreen = TargetScreen;
                    if(TopPanel) {
                        TopPanel->SetVisible(ActiveScreen->GetGlobalPanelVisibility());
                    }
                    ManagerState = Refreshing;
                    grvl::Callbacks()->set_layer_pointer(painter.GetBuffer(2));
                    painter.FillRectangle(0, 0, width, height, COLOR_ARGB8888_TRANSPARENT);
                }
                break;
            }
            case ToTheRight:
            case ToTheLeft: {
                uint8_t bytesPerPixel = painter.GetBytesPerPixel();
                uint8_t displayBytesPerPixel = painter.GetDisplayBytesPerPixel();
                uint8_t pixelFormat = painter.GetPixelFormat();
                uint8_t displayPixelFormat = painter.GetDisplayPixelFormat();
                UpdateAnimationWindowOffset();
                if(painter.IsRotated()) {
                    uintptr_t framePtr = static_cast<uintptr_t>(bytesPerPixel) * GetTotalHeadersHeight();
                    if(ManagerState == ToTheRight) {
                        framePtr += static_cast<unsigned long>(AnimationWindowOffset * height * bytesPerPixel);
                    } else {
                        framePtr += static_cast<unsigned long>(width - AnimationWindowOffset) * height * bytesPerPixel;
                    }

                    // NOLINTBEGIN
                    painter.DmaOperationCLT((uintptr_t)(painter.GetBuffer(0) + framePtr),
                                            (uintptr_t)(BackgroundImage.GetContentData() + BackgroundImage.GetContentBytesPerPixel() * GetTotalHeadersHeight()),
                                            (uintptr_t)(painter.GetVisibleBuffer() + (displayBytesPerPixel * GetTotalHeadersHeight())),
                                            height - GetTotalHeadersHeight() - GetBottomPanelHeight(), width,
                                            GetTotalHeadersHeight() + GetBottomPanelHeight(), GetTotalHeadersHeight() + GetBottomPanelHeight(),
                                            GetTotalHeadersHeight() + GetBottomPanelHeight(), pixelFormat,
                                            BackgroundImage.GetContentColorFormat(), displayPixelFormat, BackgroundImage.GetContent() ? BackgroundImage.GetContent()->GetPLTE() : 0, 0);
                    // NOLINTEND
                } else {
                    if(((int)width >= AnimationWindowOffset) && (AnimationWindowOffset > 0)) {
                        uintptr_t w1;
                        uintptr_t w2;
                        if(ManagerState == ToTheRight) {
                            w1 = AnimationWindowOffset;
                            w2 = width - AnimationWindowOffset;
                        } else {
                            w1 = width - AnimationWindowOffset;
                            w2 = AnimationWindowOffset;
                        }
                        // NOLINTBEGIN
                        painter.DmaOperationCLT(painter.GetBuffer(0) + bytesPerPixel * (width * GetTotalHeadersHeight()),
                                                (uintptr_t)(BackgroundImage.GetContentData() + BackgroundImage.GetContentBytesPerPixel() * (width * GetTotalHeadersHeight() + w1)),
                                                painter.GetVisibleBuffer() + displayBytesPerPixel * (width * GetTotalHeadersHeight() + w1), w2,
                                                height - GetTotalHeadersHeight() - GetBottomPanelHeight(), w1, w1, w1, pixelFormat,
                                                BackgroundImage.GetContentColorFormat(), displayPixelFormat, BackgroundImage.GetContent() ? BackgroundImage.GetContent()->GetPLTE() : 0, 0);
                        painter.DmaOperationCLT(
                            painter.GetBuffer(1) + bytesPerPixel * (width * GetTotalHeadersHeight() + w2),
                            (uintptr_t)(BackgroundImage.GetContentData() + BackgroundImage.GetContentBytesPerPixel() * (width * GetTotalHeadersHeight())),
                            painter.GetVisibleBuffer() + displayBytesPerPixel * (width * GetTotalHeadersHeight()), w1,
                            height - GetTotalHeadersHeight() - GetBottomPanelHeight(), w2, w2, w2, pixelFormat,
                            BackgroundImage.GetContentColorFormat(), displayPixelFormat, BackgroundImage.GetContent() ? BackgroundImage.GetContent()->GetPLTE() : 0, 0);
                        // NOLINTEND
                    }
                }
                if(AnimationWindowOffset <= 0) {
                    ActiveScreen = TargetScreen;
                    if(TopPanel) {
                        TopPanel->SetVisible(ActiveScreen->GetGlobalPanelVisibility());
                    }
                    ManagerState = Refreshing;
                    UnlockDrawMutex();
                    return;
                }
                break;
            }

            case Refreshing: {
                // Select a buffer to draw on
                painter.SetActiveBuffer(0);
                if(ActiveScreen) {

                    // printf("Drawing the active screen! (%d %d)\n", width, height - GetTotalHeadersHeight() - GetBottomPanelHeight());

                    ActiveScreen->SetSize(width, height - GetTotalHeadersHeight() - GetBottomPanelHeight());
                    ActiveScreen->Draw(painter, 0, GetTotalHeadersHeight());
                }
                break;
            }
            default: {
                // unknown state-
                break;
            }
        }

        painter.SetActiveBuffer(0);

        if(TopPanel && TopPanel->IsVisible() && GetGlobalTopPanelVisibility()) {
            TopPanel->Draw(painter, 0, 0);
        }

        if(ActiveScreen && ActiveScreen->GetHeader() && ActiveScreen->GetHeader()->IsVisible()) {
            ActiveScreen->GetHeader()->Draw(painter, 0, GetTopPanelHeight());
        }

        if(BottomPanel && BottomPanel->IsVisible()) {
            BottomPanel->Draw(painter, 0, height - GetBottomPanelHeight());
        }

        if(ManagerState == Refreshing) { // Use unified background handling
            painter.MergeBuffers();
        } else if(GetTotalHeadersHeight() > 0 && !painter.IsRotated()) { // Use animation optimized background handling
            uint8_t pixelFormatBackground = painter.GetPixelFormat();
            uint8_t pixelFormatBackgroundDisplay = painter.GetDisplayPixelFormat();
            painter.DmaOperationCLT(painter.GetBuffer(0), (uintptr_t)(BackgroundImage.GetContentData()),
                                    painter.GetVisibleBuffer(), width, GetTotalHeadersHeight(), 0, 0, 0,
                                    pixelFormatBackground, BackgroundImage.GetContentColorFormat(), pixelFormatBackgroundDisplay,
                                    BackgroundImage.GetContent() ? BackgroundImage.GetContent()->GetPLTE() : 0, 0);
        }

        painter.SetActiveBuffer(painter.GetSwapperValue() ? 2 : 3);

        if(ActiveScreen && ActiveScreen->GetCollectionSize() > 0) {
            DrawDots(ActiveScreen->GetCollectionSize(), ActiveScreen->GetPositionInCollection());
        }

        if(CurrentPopup && CurrentPopup->IsVisible()) {
            painter.SetActiveBuffer(painter.GetSwapperValue() ? 2 : 3);
            static constexpr auto semiTransparent = 0x7F000000;
            painter.ShadowBuffer(painter.GetSwapperValue() ? 2 : 3, semiTransparent); // note: originally caused segfault
            CurrentPopup->Draw(painter, 0, 0);
        }

        ApplyTransparency();

        // Debug touch point
        static constexpr auto touchDelay = 2000;
        if(debugDot && (TouchEvent.GetState() != Touch::Idle || PointerTimestamp > (grvl::Callbacks()->get_timestamp() - touchDelay))) {
            static constexpr auto maxAlpha = 0xff;
            uint8_t alpha = maxAlpha - ((float)(grvl::Callbacks()->get_timestamp() - PointerTimestamp) / touchDelay) * maxAlpha;
            painter.FillCircle(TouchEvent.GetCurrentX(), TouchEvent.GetCurrentY(), 5, //NOLINT
                               alpha << 24 | (0x00FFFFFF & COLOR_ARGB8888_RED)); //NOLINT
        }

        painter.FlipSynchronizeBuffers();
        flips++;

        UnlockDrawMutex();
    }

    void Manager::ApplyTransparency()
    {
        // Compute current transparency
        if(currentTransparency != desiredTransparency) {
            CurrentTimestamp = grvl::Callbacks()->get_timestamp();
            float deltaTransparency = desiredTransparency - initialTransparency;
            float deltaTime = (float)(CurrentTimestamp - fadeBeginTimestamp) / (float)(fadeEndTimestamp - fadeBeginTimestamp);
            if(deltaTime > 1.0) {
                currentTransparency = desiredTransparency;
            } else {
                currentTransparency = initialTransparency + (deltaTransparency) * (deltaTime);
            }
        }
        // Apply transparency
        if(currentTransparency < 1.0) {
            painter.ShadowBuffer(painter.GetSwapperValue() ? 2 : 3, ((uint32_t)(0xff - 0xff * currentTransparency)) << 24); // NOLINT
        }
    }

    void Manager::DrawDots(uint32_t numberOfDots, uint32_t activeDot)
    {
        if(numberOfDots < 0 || dotYPos - dotRadius < 0 || dotYPos + dotRadius > (int32_t)height) {
            return;
        }

        int32_t XCenter;
        int XBegin;
        XCenter = width / 2;
        // odd parity
        if(numberOfDots % 2) {
            XBegin = XCenter - CollectionImage.GetWidth() / 2 - (dotDistance * (numberOfDots - 1) / 2);
        }
        // even parity
        else {
            XBegin = XCenter - CollectionImage.GetWidth() / 2 - (dotDistance / 2) - (dotDistance * ((numberOfDots / 2) - 1));
        }

        // Drawing dots
        for(uint32_t i = 0; i < numberOfDots; i++) {
            int32_t pos = XBegin + (dotDistance * i);
            if(pos - dotRadius < 0 || pos + dotRadius > (int32_t)width) {
                return;
            }
            // Empty dot
            if(i + 1 != activeDot) {
                if(CollectionImage.IsEmpty()) {
                    painter.FillCircle(pos, dotYPos, dotRadius, dotColor);
                } else {
                    CollectionImage.SetActiveFrame(0);
                    CollectionImage.Draw(painter, pos, dotYPos - CollectionImage.GetHeight() / 2);
                }
            }
            // Filled dot
            else {
                if(CollectionImage.IsEmpty()) {
                    painter.FillCircle(pos, dotYPos, dotRadius, dotActiveColor);
                } else {
                    CollectionImage.SetActiveFrame(1);
                    CollectionImage.Draw(painter, pos, dotYPos - CollectionImage.GetHeight() / 2);
                }
            }
        }
    }

    int8_t Manager::IsLoadingIcoVisible() const
    {
        return LoadingImageVisible;
    }

    Image& Manager::GetLoadingImage()
    {
        return LoadingImage;
    }

    Popup* Manager::GetPopupFromContainer(const char* name)
    {
        if(!name) {
            return NULL;
        }
        Popup* TempPopup = NULL;
        for(uint32_t i = 0; i < PopupsContainer.size(); i++) {
            if(strcmp(PopupsContainer[i]->GetID(), name) == 0) {
                TempPopup = PopupsContainer[i];
                break;
            }
        }
        return TempPopup;
    }

    void Manager::ProcessTouchPoint(bool touched, uint32_t touchX, uint32_t touchY)
    {
        if(!previousTouchState && !touched) { // Idle
            TouchEvent.SetState(Touch::Idle);
        } else if(previousTouchState && touched) { // Moving
            PointerTimestamp = grvl::Callbacks()->get_timestamp();
            TouchEvent.SetState(Touch::Moving);
            TouchEvent.SetCurrentPosition(touchX, touchY);
        } else if(!previousTouchState && touched) { // Pressed
            PointerTimestamp = grvl::Callbacks()->get_timestamp();
            TouchEvent.SetState(Touch::Pressed);
            TouchEvent.SetStartPosition(touchX, touchY);
            TouchEvent.SetCurrentPosition(touchX, touchY);
        } else if(previousTouchState && !touched) { // Released
            TouchEvent.SetState(Touch::Released);
            TouchEvent.SetCurrentPosition(touchX, touchY);
        }

        if(CurrentPopup) {
            if(CurrentPopup->IsVisible() && touchToPopup) {
                CurrentPopup->ProcessTouch(TouchEvent, 0, 0, 0);
            } else if(CurrentPopup->IsVisible() && !touchToPopup) {
                touchToPopup = true;
            } else if(!CurrentPopup->IsVisible() && touchToPopup) {
                touchToPopup = false;
            }
        } else if(touchToPopup && !CurrentPopup) {
            touchToPopup = false;
        }

        if(!touchToPopup) {
            if(TopPanel && TopPanel->IsVisible()) {
                TopPanel->ProcessTouch(TouchEvent, 0, 0, 0);
            }

            if(ManagerState == Refreshing) {
                // Push touch to header
                if(ActiveScreen && ActiveScreen->GetHeader() && ActiveScreen->GetHeader()->IsVisible()) {
                    ActiveScreen->GetHeader()->ProcessTouch(TouchEvent, 0, GetTopPanelHeight(), touchAreaModificator);
                }

                // Push touch to screen
                if(ActiveScreen && TouchEvent.GetState() != Touch::Idle) {
                    ActiveScreen->ProcessTouch(TouchEvent, 0, GetTotalHeadersHeight(), touchAreaModificator);
                }
            }
        }
        previousTouchState = touched;
    }

    void Manager::ProcessKeyInput(bool pressed, uint16_t code)
    {
        if(ActiveScreen == NULL) {
            return;
        }

        if(pressed && !keyActive) { // Press
            KeyMappingMap::const_iterator searchKey = KeyMappingContainer.find(code);
            if(searchKey != KeyMappingContainer.end() && activeKey.name != searchKey->second.name) {
                activeKey = searchKey->second;
                ActiveScreen->PressKey(activeKey.name.c_str());
                keyActive = true;
                KeyPressTimestamp = grvl::Callbacks()->get_timestamp();
            }
        } else if(!pressed && keyActive) { // Release
            ActiveScreen->ReleaseKey(activeKey.name.c_str());
            activeKey.name = "";
            keyActive = false;
            longPressActive = false;
        }
    }

    Key::KeyState Manager::PressKey(const char* id)
    {
        return ActiveScreen->PressKey(id);
    }

    Key::KeyState Manager::ReleaseKey(const char* id)
    {
        return ActiveScreen->ReleaseKey(id);
    }

    bool Manager::UpdateAnimationWindowOffset()
    {
        int32_t originalAnimationWindowOffset = AnimationWindowOffset;
        CurrentTimestamp = grvl::Callbacks()->get_timestamp();
        float progress = ((float)(CurrentTimestamp - BeginTimestamp)) / ScrollingDuration; // 0 - 1
        if(progress < 0.0) {
            progress = 0.0;
        } else if(progress > 1.0) {
            progress = 1.0;
        } else {
            progress = MotionFunc(progress);
        }
        AnimationWindowOffset = (1.0 - progress) * width;
        return AnimationWindowOffset != originalAnimationWindowOffset;
    }

    int32_t Manager::BuildFromXML(const char* filename)
    {
        File* fl = new File(filename);
        uint8_t* data = (uint8_t*)grvl::Callbacks()->malloc(fl->GetSize());
        fl->ReadToBuffer(data);
        int32_t result = BuildFromXML((char*)data, fl->GetSize());
        delete fl;
        if (!File::noFS) {
            grvl::Callbacks()->free(data);
        }
        return result;
    }

    void Manager::ParseGuiConfiguration(XMLElement* ConfigNode)
    {
        static constexpr auto dotDistanceDefault = 20;
        static constexpr auto dotRadiusDefault = 5;
        touchAreaModificator = (int32_t)XMLSupport::GetAttributeOrDefault(
            ConfigNode, "touchRegionModificator", (uint32_t)0);
        dotColor = (int32_t)XMLSupport::GetAttributeOrDefault(ConfigNode, "dotColor", (uint32_t)0);
        dotActiveColor = (int32_t)XMLSupport::GetAttributeOrDefault(ConfigNode, "dotActiveColor", (uint32_t)0);
        dotDistance = (int32_t)XMLSupport::GetAttributeOrDefault(ConfigNode, "dotDistance", (uint32_t)dotDistanceDefault);
        dotRadius = (int32_t)XMLSupport::GetAttributeOrDefault(ConfigNode, "dotRadius", (uint32_t)dotRadiusDefault);
        dotYPos = (int32_t)XMLSupport::GetAttributeOrDefault(ConfigNode, "dotYPos", (uint32_t)0);
        debugDot = (int32_t)XMLSupport::GetAttributeOrDefault(ConfigNode, "debugDot", (uint32_t)0);
    }

    void Manager::ParseKeypadMapping(XMLElement* KeypadNode)
    {
        XMLElement* nextElement = KeypadNode->FirstChildElement("key");
        while(nextElement != 0) {
            const char* id = nextElement->Attribute("id");

            uint32_t code_int = -1;
            nextElement->QueryUnsignedAttribute("code", (unsigned int*)&code_int);

            static constexpr auto defaultRepeat = 100;
            uint32_t repeat = defaultRepeat;
            nextElement->QueryUnsignedAttribute("repeat", (unsigned int*)&repeat);

            AddKeyMappingToContainer(code_int, string(id), repeat);

            nextElement = nextElement->NextSiblingElement("key");
        }
    }
    // NOLINTBEGIN
    void Manager::ParseStyle(char* active_object, char* active_parameter, char* active_parameter_value)
    {
        char aobject[255];
        char* value;
        int type = 0;
        if(active_object[0] == '.')
            type = 1;
        if(active_object[0] == '#')
            type = 2;
        memmove(aobject, active_object + ((type > 0) ? 1 : 0), strlen(active_object) + ((type > 0) ? 0 : 1));

        string key(aobject);
        key.append("+");
        key.append(active_parameter);

        size_t value_len = strlen(active_parameter_value) + 1;
        value = (char*)grvl::Callbacks()->malloc(value_len);
        strncpy(value, active_parameter_value, value_len);

        switch(type) {
            case 0:
                widgetAttributes[key] = value;
                break;
            case 1:
                classAttributes[key] = value;
                break;
            case 2:
                idAttributes[key] = value;
                break;
        }
        // TODO: actually add this style to a list, and interpret those.
    }

    void Manager::ParseStylesheet(XMLElement* stylesheet)
    {
        if(!stylesheet)
            return;
        char* data = (char*)stylesheet->GetText();
        if(!data)
            return;

        size_t SIZE = 255;

        char active_object[SIZE];
        char active_parameter[SIZE];
        char active_parameter_value[SIZE];
        char active[SIZE];
        int count = 0;
        int level = 0;

        while(data[0] != 0x0) {
            switch(data[0]) {
                case '{': {
                    if(level != 0)
                        break;
                    strncpy(active_object, active, SIZE);
                    count = 0;
                    level++;
                    break;
                }
                case ':': {
                    if(level != 1)
                        break;
                    strncpy(active_parameter, active, SIZE);
                    count = 0;
                    level++;
                    break;
                }
                case ';': {
                    if(level == 2) {
                        level--;
                        strncpy(active_parameter_value, active, SIZE);
                        ParseStyle(active_object, active_parameter, active_parameter_value);
                    }
                    count = 0;
                    break;
                }
                case '}': {
                    if(level == 2) {
                        level--;
                        strncpy(active_parameter_value, active, SIZE);
                        ParseStyle(active_object, active_parameter, active_parameter_value);
                    }
                    if(level != 1)
                        break;
                    level--;
                    count = 0;
                    break;
                }
                case ' ':
                case '\n':
                case '\t': {
                    count = 0;
                    break;
                }
                default: {
                    active[count++] = data[0];
                    if(count > SIZE)
                        return;
                    active[count] = 0;
                }
            }
            data++;
        }
    }
    // NOLINTEND

    int32_t Manager::BuildFromXML(const char* XMLDoc, size_t len)
    {

        if(!XMLDoc) {
            return -1; // Parsing failed
        }

        XMLDocument doc;

        XMLError Error = doc.Parse(XMLDoc, len);

        if(Error == XML_SUCCESS) {
            XMLNode* Root = doc.LastChild();

            // Adding screens
            if(Root) {

                XMLElement* nextElement = Root->FirstChildElement("guiConfig");
                if(nextElement) {
                    ParseGuiConfiguration(nextElement);
                }

                nextElement = Root->FirstChildElement("stylesheet");
                if(nextElement) {
                    ParseStylesheet(nextElement);
                }

                nextElement = Root->FirstChildElement("script");
                if(nextElement) {
                    ParseScripts(nextElement);
                }

                nextElement = Root->FirstChildElement("keypadMapping");
                if(nextElement) {
                    ParseKeypadMapping(nextElement);
                }

                nextElement = Root->FirstChildElement("header");
                if(nextElement) {
                    TopPanel = Panel::BuildFromXML(nextElement);
                }

                nextElement = Root->FirstChildElement("footer");
                if(nextElement) {
                    BottomPanel = Panel::BuildFromXML(nextElement);
                }

                nextElement = Root->FirstChildElement("popup");
                while(nextElement) {
                    Popup* popup = Popup::BuildFromXML(nextElement);
                    if(popup) {
                        AddPopup(popup);
                    }
                    nextElement = nextElement->NextSiblingElement("popup");
                }

                nextElement = Root->FirstChildElement("customView");
                while(nextElement) {
                    AbstractView* Screen = CustomView::BuildFromXML(nextElement);
                    if(Screen) {
                        AddScreen(Screen);

                        printf("Added screen for CustomView\n");
                    }
                    nextElement = nextElement->NextSiblingElement("customView");
                }

                nextElement = Root->FirstChildElement("ListView");
                while(nextElement) {
                    AbstractView* Screen = ListView::BuildFromXML(nextElement);
                    if(Screen) {
                        AddScreen(Screen);
                    }
                    nextElement = nextElement->NextSiblingElement("ListView");
                }

                nextElement = Root->FirstChildElement("GridView");
                while(nextElement) {
                    AbstractView* Screen = GridView::BuildFromXML(nextElement);
                    if(Screen) {
                        AddScreen(Screen);
                    }
                    nextElement = nextElement->NextSiblingElement("GridView");
                }
            }
        } else { // Parsing failed
            grvl::Log("[ERROR] XML parsing failed.");
            doc.Clear();
            return -1;
        }
        grvl::Log("[INFO] Parsing done.");
        doc.Clear();
        if(GetScreen("start"))
            SetActiveScreen("start", 0);
        return 0;
    }

    void Manager::ParseScripts(XMLElement* script)
    {
        if (const char* embeddedJavaScriptCode = script->GetText()) {
            std::size_t embeddedJavaScriptCodeSize = strlen(embeddedJavaScriptCode);
            JSEngine::LoadJavaScriptCode(embeddedJavaScriptCode, embeddedJavaScriptCodeSize);
        }

        const char* linkedJavaScriptCodePath = XMLSupport::GetAttributeOrDefault(script, "src", "");
        JSEngine::LoadJavaScriptCode(linkedJavaScriptCodePath);
    }

    Event Manager::GetEventWithArguments(const char* eventName) const
    {
        if(!eventName) {
            return Event();
        }
            string FunctionName = ParsingUtils::GetFunctionName(string(eventName));
            Event::CallbackPointer callbackPtr = GetCallbackFromContainer(FunctionName);
            Event::ArgVector argVec = ParsingUtils::GetArguments(string(eventName));

        if(callbackPtr) {
            return Event(callbackPtr, argVec);
        }
        return Event();
    }

    Manager& Manager::SetLogoImage(const Image& logo)
    {
        LogoImage = logo;
        if(painter.IsRotated() && !LogoImage.GetContent()->IsRotated()) {
            LogoImage.GetContent()->Rotate90();
        }
        return *this;
    }

    void Manager::MainLoopIteration()
    {
        // process popups
        if(timeoutedPopupMode && CurrentPopup && CurrentPopup->GetTimetamp() < grvl::Callbacks()->get_timestamp()) {
            ClosePopup();
            timeoutedPopupMode = false;
        }

        // check key long press
        static constexpr auto keyLongPressDelay = 500;
        if(keyActive) {
            if(!longPressActive && KeyPressTimestamp < (grvl::Callbacks()->get_timestamp() - keyLongPressDelay)) { // Long press
                longPressActive = true;
                KeyPressTimestamp = grvl::Callbacks()->get_timestamp();
                ActiveScreen->LongPressKey(activeKey.name.c_str());
            } else if(longPressActive && KeyPressTimestamp < (grvl::Callbacks()->get_timestamp() - activeKey.repeat)) { // Long press repeat
                KeyPressTimestamp = grvl::Callbacks()->get_timestamp();
                ActiveScreen->LongPressRepeatKey(activeKey.name.c_str());
            }
        }

        // process events
        ProcessEvents();

        // redraw
        Draw();
    }

    void Manager::ProcessEvents()
    {
        Event* currentEvent;
        do {
            currentEvent = eventsQueue.pop();
            if(!currentEvent) {
                break;
            }
            currentEvent->Trigger();
        } while(currentEvent);
    }

    void Manager::Initialize(uint32_t xSize, uint32_t ySize, int bpp, bool rotate90)
    {

        delete instance;
        instance = new Manager(xSize, ySize, bpp, rotate90);
    }

    void Manager::Initialize(uint32_t xSize, uint32_t ySize, int bpp, bool rotate90, uint8_t* framebuffer)
    {

        delete instance;
        instance = new Manager(xSize, ySize, bpp, rotate90, framebuffer);
    }

    void Manager::ResetScreens()
    {
        ActiveScreen = NULL;
        CurrentPopup = NULL;
        vector<AbstractView*>::iterator it;
        for(it = Screens.begin(); it != Screens.end();) {
            delete *it;
            it = Screens.erase(it);
        }
        vector<Popup*>::iterator itp;
        for(itp = PopupsContainer.begin(); itp != PopupsContainer.end();) {
            delete *itp;
            itp = PopupsContainer.erase(itp);
        }
        if(TopPanel) {
            delete TopPanel;
            TopPanel = NULL;
        }
        if(BottomPanel) {
            delete BottomPanel;
            BottomPanel = NULL;
        }
    }

    void Manager::ClearBuffers()
    {
        painter.FillRectangle(0, 0, width, height, COLOR_ARGB8888_TRANSPARENT);
        painter.FlipBuffers();
        painter.FillRectangle(0, 0, width, height, COLOR_ARGB8888_TRANSPARENT);
    }

    Manager* Manager::instance = NULL;
    Manager& Manager::GetInstance()
    {
        return *instance;
    }

    Component* Manager::FindElementInTheActiveScreenById(const char* id)
    {
        return ActiveScreen->GetElement(id);
    }

} /* namespace grvl */
