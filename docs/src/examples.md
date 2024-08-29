# Examples

## Animating values

First, grab a component by ID:

```cpp
ProgressBar* progressBar = dynamic_cast<ProgressBar*>(displayManager->GetActiveScreen()->GetElement("progress_bar"));
```

Then execute a setter in the main loop:

```cpp
progressBar->SetProgressValue((SDL_GetTicks() / 100) % 100);
```

## Callbacks

Create the callbacks:

```cpp
void ButtonCallback(Button *Sender, const Event::ArgVector& Args) {
    printf("Button clicked! (%s)\n", Sender->GetID());
}

void SwitchCallback(SwitchButton *Sender, const Event::ArgVector& Args) {
    switch (Sender->GetSwitchState()) {
        case true:
            printf("Switch is ON! (%s)\n", Sender->GetID());
            break;
        default:
            printf("Switch is OFF! (%s)\n", Sender->GetID());
    }
}
```

Add them:

```cpp
displayManager->AddCallbackToContainer("ButtonCallback", (grvl::Event::CallbackPointer)ButtonCallback);
displayManager->AddCallbackToContainer("SwitchCallback", (grvl::Event::CallbackPointer)SwitchCallback);
                                      /*^^^^ choose an ID */
```

Then bind them in XML referencing the chosen ID:

```xml
<Button id="test_button" x="0" y="0" width="100" height="100" onClick="ButtonCallback" text="TEST" />
<SwitchButton id="test_switch" x="500" y="500" width="150" height="100" onSwitchON="SwitchCallback" onSwitchOFF="SwitchCallback" />
```

## Popups

Adding an element in XML with a callback to show a popup:

```xml
<Button id="btn" x="0" y="0" width="150" height="100" onClick="ShowPopup('example_popup')" text="Show Popup" />
```

Adding a popup in XML (with an element closing it on callback):

```xml
<Popup id="example_popup" x="0" y="0" width="300" height="200">
    <button id="close_popup_btn" x="100" y="10" width="100" height="50" onClick="ClosePopup" text="Close" />
    <label id="popup_label" font="roboto-medium" x="0" y="100" width="300" height="50" text="This is a Popup." />
</Popup>
```

## Adding images and fonts

```cpp
displayManager->AddFontToFontContainer("my_font", new Font(path_to_font));
displayManager->AddImageContentToContainer("my_image", new ImageContent(ImageContent::FromPNG(path_to_image)));
```
