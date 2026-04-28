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

There are two ways to add callbacks to GUI components. You can write them in either C++ or JavaScript.

### C++

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

Add them to the callbacks container:

```cpp
displayManager->AddCallbackToContainer("ButtonCallback", (grvl::Event::CallbackPointer)ButtonCallback);
displayManager->AddCallbackToContainer("SwitchCallback", (grvl::Event::CallbackPointer)SwitchCallback);
                                      /*^^^^ choose a callback name */
```

### JavaScript

Create a file with JavaScript callbacks, e.g.:

```javascript
// callbacks.js

function ButtonCallback(caller) {
    const buttonId = caller.name
    Print("Button clicked! (" + buttonId + ")")
}

function SwitchCallback(caller) {
    const callerName = caller.name
    if (caller.switchState) {
        Print("Switch is ON! (" + callerName + ")")
    } else {
        Print("Switch is OFF! (" + callerName + ")")
    }
}
```

Then include it in your application's XML layout:

```xml
<script src="callbacks.js"></script>
```

You can specify a non-default working directory for JavaScript files with:

```cpp
JSEngine::SetSourceCodeWorkingDirectory("Scripts/JavaScript/");
```

See [JavaScript documentation](js-reference) for further reference.

### Binding callbacks

Bind callbacks to GUI components in XML by referencing the callback name:

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

Grvl support two font types **Grvl Baked Fonts** (.gbf) and **True Type Fonts** (.ttf). Both formats have their valid uses.
You can create a grvl baked font using the provided `gbf` CLI utility, that converts TTFs to GBFs.
The baking process allows you to select which codepoints should be included, refer to `gbf --help` for more information.

```sh
# Build GBF utility application
cmake -B build
cmake --build build --target gbf

# Bake characters into GBF
./build/gbf --ttf ./romfs/fonts/MyFont.ttf --size 18 --gbf ./romfs/fonts/MyFont.gbf --range ascii,0x100-0x200
```

If you want to use TTF fonts in a memory contained enviroment it may be beneficial to create a smaller TTF fonts with some, unused, glyphs removed.
You can use the open source `pyftsubset` utility for that. Unused glyphs can also be deleted from a font in [Font Forge](https://fontforge.org) graphically.

```sh
# Create a ASCII-only version of the font
pyftsubset ./romfs/fonts/MyFont.ttf --unicodes=U+0020-007E --output-file=./romfs/fonts/MyFont-ascii.ttf
```

Loading both font types is shown below.

```cpp
displayManager->AddFontToFontContainer("my_font_gbf", new GrvlBakedFont(path_to_font));
displayManager->AddImageContentToContainer("my_image", new ImageContent(ImageContent::FromPNG(path_to_image)));

// using True Type Fonts
auto data = std::make_shared<grvl::TrueTypeData>(path_to_font);
displayManager->AddFontToFontContainer("my_font_ttf", new TrueTypeFont(data, 18));
```

## Default fonts

When a font is not specified for an element (or the font is not found) grvl will try using the *normal* font - and if that is not present - the *default* font.
You may see this in the logs:

```
[WARNING] Font "normal" doesn't exist. Using default font!
[ERROR] Default font doesn't exist.
[WARNING] Font "normal" doesn't exist. Using default font!
[ERROR] Default font doesn't exist.
[WARNING] Font "normal" doesn't exist. Using default font!
[ERROR] Default font doesn't exist.
```

To fix those errors you just need to provide a *normal* (and/or *default*) font, like so:

```cpp
manager.AddFontToFontContainer("normal", new grvl::TrueTypeFont( /* ... */ ));
manager.AddFontToFontContainer("default", new grvl::TrueTypeFont( /* ... */ ));
```
