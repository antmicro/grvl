# XML reference

## XML validity

A [schema](../grvl.xsd) is available in grvl along with an example that it validates.
To check the validity, use an external tool, for example `xmllint`.

```sh
$ xmllint --schema <path_to_schema.xsd> <path_to_my_xml_grvl_file.xml>
```

## How to construct a grvl XML document?

First define the root of the document.

```xml
<doc>
    <!-- screens/special components -->
</doc>
```

Then include the screens/special components in the document.

## Expressions

Each attribute value and each text node can hold `${...}` spans.
grvl evaluates the content of a span as a JavaScript expression and puts the
result in its place. The evaluation occurs one time, before any widget is built.

```xml
<button x="${100 + 20}" width="${Math.round(screenWidth / 3)}" text="${screenWidth}x${screenHeight}"/>
```

The expressions use the same Duktape context as the `<script>` elements, thus
they can call the global functions of the GUI. grvl defines `screenWidth` and
`screenHeight` before the evaluation. This makes one template sufficient for
more than one display resolution.

Write `$$` for a dollar sign that is followed by a brace.

### Scripts and defines

A `<script>` element is loaded before anything is expanded, thus a `${...}`
span can call the functions it declares. The same script is what the GUI uses
at run time, so a helper is written once and serves both.

A `<defines>` block holds plain constants. Each `<define>` is evaluated in
document order and can use the names above it, and the block is removed from
the document afterwards.

```xml
<doc>
    <script>
        function px(value) { return Math.round(value * scale); }
    </script>

    <defines>
        <define name="scale" value="screenWidth / 1920"/>
        <define name="gutter" value="Math.round(16 * scale)"/>
    </defines>

    <customView id="home">
        <button id="ok" x="${gutter}" y="${gutter}" width="${px(240)}" height="${px(48)}"/>
    </customView>
</doc>
```

The order inside the document does not matter for a function, because `px()`
reads `scale` when the span calls it, which is after the defines are
evaluated. It does matter for code that runs immediately: a statement at the
top level of a script cannot read a `<define>`, because the scripts run first.

`${...}` is not expanded inside a `<script>` element. A script is JavaScript
already, thus it reads `screenWidth` and the defines as plain variables.

To keep the resolution-dependent values separate from the layout, put the
`<defines>` block in its own file and include it:

```xml
<doc xmlns:xi="http://www.w3.org/2001/XInclude">
    <xi:include href="metrics-1920.xml"/>
    <!-- the layout -->
</doc>
```

### Operators and quotation marks

Write the expression in plain JavaScript. grvl escapes the XML special
characters inside a span before the parse, thus `>`, `<`, `&&` and the
quotation marks are permitted with no entity:

```text
<label text="${screenWidth > 1280 ? "wide" : "narrow"}"/>
```

An XML entity inside a span is **not** decoded first. Write `>`, not `&gt;`.

### Limits

* The engine is Duktape, thus the language is ECMAScript 5.1. There are no
  arrow functions, no `let` and no template literals.
* A regular expression literal in a span is not supported, because the span
  scanner cannot tell it from a division. Use `new RegExp("...")`.
* The expressions share one global object and run in document order. Thus an
  expression that has a side effect changes the result of the expressions
  below it.
* A span in a comment, a CDATA section or a processing instruction is not
  expanded.

## Generic component/container attributes

Each component has the following attributes:

* id
* x (absolute, in pixels)
* y (absolute, in pixels)
* width
* height
* visible (true/false)
* foregroundColor
* activeForegroundColor
* backgroundColor
* activeBackgroundColor
* borderColor
* activeBorderColor
* borderType (one of: none, box, top, right, bottom, left)
* borderArcRadius
* onClick (callback)
* onPress (callback)
* onRelease (callback)

Containers can also be specified as `selection`, which makes it a single-choice container, meaning that only one child component can be active and will remain active until another component from that container is activated.

## Generic screen attributes

Each screen is a container, so it has all of the attributes from the [previous section](#generic-component-container-attributes) as well as:

* onSlideToLeft (callback)
* onSlideToRight (callback)
* onLongPress (callback)
* onLongPressRepeat(callback)
* collection (string)
* globalPanelVisible (true/false)

It can also contain `<key>` that can have the following attributes:

* id
* onPress
* onLongPress (callback)
* onLongPressRepeat(callback)
* onRelease

## Special attributes

### Colors

Uses the `#aarrggbb` format color.

### Alignment

One of: `Center`, `Left`, `Right`.

### Border type

One of: `none`, `box`, `top`, `right`, `bottom`, `left`.

## List of components/containers

### Label

#### Attributes

* text
* font
* textColor
* alignment

#### Example

```xml
<label id="popup_label" font="roboto-medium" x="0" y="100" width="300" height="50" text="This is a label." />
```

### Button

#### Attributes

* text
* font
* image
* textColor
* activeTextColor
* icoChar
* icoFont
* icoColor
* alignment
* contentLayout
* imageTextGap
* padding
* onLongPress
* onLongPressRepeat

#### Example

```xml
<button id="close_popup_btn" x="100" y="10" width="100" height="50" backgroundColor="#ffffffff" textColor="#ff0000ff" activeTextColor="#ffff00ff" onClick="ClosePopup" text="Close" font="roboto-medium" />
```

### Clock

#### Attributes

* font
* alignment
* onRelease (callback)
* foregroundColor
* format (strftime syntax)

#### Example

```xml
<clock id="test_clock" x="150" y="10" width="100" height="100" font="roboto-medium" foregroundColor="#fffff000" format="%a %b %d %l:%M" />
```

### Slider

#### Attributes

* frameColor
* selectedFrameColor
* barColor
* activeBarColor
* activeScrollColor
* maxValue
* minValue
* keepBoundaries
* type - Continuous (default), Discrete (divided into steps - see division), List (list indices)
* division
* font
* image
* onValueChange

#### Example

```xml
<slider id="vertical_slider" x="300" y="10" width="20" height="100" scrollColor="#ffff0000" activeBarColor="#ff00ff00" />
```

### ListItem

#### Attributes

* text
* font
* description
* descriptionFont
* ActiveTextColor
* descriptionColor
* activeDescriptionColor
* image
* additionalImage
* roundingImage
* onLongPress
* onLongPressRepeat
* type (one of: StdListField, LeftArrowField, RightArrowField, EmptyField, Dots, DoubleImageField, AlarmField)

#### Example

```xml
<ListItem backgroundColor="#ffff0000" id="item_1" height="50" type="StdListField" text="first" textColor="#ffffffff" font="roboto-medium" />
```

### ProgressBar

#### Attributes

* progressBarColor

#### Example

```xml
<ProgressBar id="progress_bar" x="90" y="60" width="50" height="10" progressBarColor="#ffffff00" />
```

### CircleProgressBar

#### Attributes

* progressBarColor
* startColor
* endColor
* startAngle
* endAngle
* radius
* thickness
* staticGradient

#### Example

```xml
<CircleProgressBar id="circle_progress_bar" x="175" y="120" width="50" height="50" radius="20" thickness="5" startColor="#fffcba03" endColor="#ffa903fc" />
```

(switch-button)=
### SwitchButton

#### Attributes

Derives all attributes from `Button` with the addition of:

* onSwitchON - callback executed when switching to active state
* onSwitchOFF - callback executed when switching to inactive state
* stateIndicatorWidth/stateIndicatorHeight - size dimensions of state indicator
* stateIndicatorArcRadius - arc radius of state indicator

#### Example

```xml
<SwitchButton id="test_switch" x="0" y="60" width="80" height="50" stateIndicatorArcRadius="5" stateIndicatorWidth="20" stateIndicatorHeight="20" foregroundColor="#fffcba03" backgroundColor="#ffffffff" font="roboto" onSwitchON="SwitchCallback" onSwitchOFF="SwitchCallback" />
```

### Checkbox

#### Attributes

Derives all attributes from [SwitchButton](#switch-button), but renders its state indicator from width and height size dimensions, instead of state indicator parameters.

#### Example

```xml
<Checkbox id="active" x="0" y="0" width="18" height="18" backgroundColor="#FFAAAAAA" activeBackgroundColor="#FFFFFFFF" onClick="NotifyAboutStateChange" />
```

### Image

#### Attributes

* contentId

#### Example

```xml
<image contentId="minus" x="253" y="131" />
```

### GridRow

#### Example

```
<GridRow id="row_1" backgroundColor="#ffff0000">
        <button id="row_1_btn_1" text="1" font="roboto-medium" width="50" height="50" backgroundColor="#ff00ffff" />
        <button id="row_1_btn_2" text="2" font="roboto-medium" width="50" height="50" backgroundColor="#ffff00ff" />
</GridRow>
```

### Division

Component designed to be used as a container for other components, with the purpose of making XML layout clean and well-organized. It has a similar purpose to HTML's `div`.

#### Example

```xml
<Division x="0" y="0" width="128" height="32" backgroundColor="#FF0E0F10" >
    <Button id="first" text="First" x="0" y="0" width="32" height="32" font="roboto" backgroundColor="#FF0E0F10" textColor="#FF9EA2A6" />
    <Button id="second" text="Second" x="42" y="0" width="32" height="32" font="roboto" backgroundColor="#FF0E0F10" textColor="#FF9EA2A6" />
    <Button id="third" text="Third" x="86" y="0" width="32" height="32" font="roboto" backgroundColor="#FF0E0F10" textColor="#FF9EA2A6" />
</Division>
```

### Separator

Used for horizontal separation of content.

#### Example

```xml
<Separator id="separator" x="0" y="20" width="128" foregroundColor="#FF2E2E2E" />
```

### TextInput

A single-line text field. The keyboard is shown when the field is clicked.

#### Attributes

* text
* basicText (placeholder, shown while the field is empty)
* font
* textColor
* activeTextColor
* type (`text` or `password`)
* cursor (`true`/`false`, default `false`)
* cursorColor (default: the value of `textColor`)
* cursorBlink (`true`/`false`, default `true`)
* autoFocus (`true`/`false`, default `false`)
* onTextInput
* onSubmit

The caret is a one-pixel line drawn after the typed text. The gap between the
text and the caret, and the height the caret adds above and below the font,
are both a quarter of the font height, with a minimum of one pixel. Thus an 8
pixel font gets a gap of 2 and a caret of 12, and a 26 pixel font gets a gap
of 6 and a caret of 38.

It appears only while the field has focus. A placeholder does not move it, so
an empty field shows the caret at the left edge. When the text is wider than
the field, the caret stops at the right edge.

The caret blinks every 500 ms. Typing restarts the cycle, so the caret is lit
while the user types. Blinking asks for a new frame on every draw, which
defeats the idle throttle of `SetIdleThrottle()`. Set `cursorBlink="false"` on
a device where the screen should go idle.

#### Focus

A text input receives the keys of `Manager::ProcessTextInput()` only while it
is the active input. Clicking the field makes it active, and `SetActiveInput`
does the same from JavaScript or from C++.

A device with no touchscreen has nothing to click, so `autoFocus="true"` marks
the field that becomes active when a screen holding it is opened. The element
is found by its id at that moment, thus a field inside a prefab gives the focus
to the instance on the screen and not to the prefab. Only one element can hold
the request; a second `autoFocus` replaces the first and logs a warning. When
the opening screen does not hold the element, the current focus stays as it is.

Note that `autoFocus` triggers the keyboard popup, exactly as a click does,
when the keyboard has `autoPopup` set.

#### Example

```xml
<TextInput id="name" basicText="Name" font="mona26"
    x="0" y="0" width="426" height="34"
    borderType="box" borderArcRadius="12"
    backgroundColor="#FF191A1C" textColor="#FF9EA2A6"
    cursor="true" cursorColor="#FFECEDEE" autoFocus="true" />
```

## List of screens

### GridView

Arranges its chilren elements in a grid layout.

#### Attributes

* overscrollEnabled
* overscrollHeight
* scrollingEnabled
* overscrollColor
* elementWidth
* elementHeight
* verticalOffset

#### Example

```xml
<GridView id="grid" x="0" y="0" width="140" height="140" backgroundColor="#FF0A0A0A" elementWidth="40" elementHeight="40" horizontalOffset="5" verticalOffset="5" selection="true" >

    <GridRow id="monthdays1" >
        <button id="0" text="0" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
        <button id="1" text="1" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
        <button id="2" text="2" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
    </GridRow>

    <GridRow id="monthdays2" >
        <button id="3" text="3" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
        <button id="4" text="4" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
        <button id="5" text="5" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
    </GridRow>

    <GridRow id="monthdays3" >
        <button id="6" text="6" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
        <button id="7" text="7" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
        <button id="8" text="8" font="mona14" backgroundColor="#FF0A0A0A" activeBackgroundColor="#FF0059EC" textColor="#FFECEDEE" onClick="SomeCallback" />
    </GridRow>

</GridView>
```

### ListView

Arranges its chilren elements into a list.

#### Attributes

* overscrollEnabled
* overscrollHeight
* splitLineColor
* overscrollColor
* scrollIndicatorColor

#### Example

```xml
<ListView id="list" x="0" y="0" width="200" height="200">

    <ListItem backgroundColor="#ffff0000" id="item1" height="50" type="StdListField" text="first" textColor="#ffffffff" font="mona12" />
    <ListItem backgroundColor="#ff00ff00" id="item2" height="50" type="StdListField" text="second" textColor="#ffffffff" font="mona12" />
    <ListItem backgroundColor="#ff0000ff" id="item3" height="50" type="StdListField" text="third" textColor="#ffffffff" font="mona12" />

</ListView>
```

### ScrollPanel

Allows to arrange its children elements inside the container in a custom way, defined by each component's `x` and `y`.

#### Attributes

* overscrollEnabled
* overscrollHeight
* splitLineColor
* overscrollColor
* scrollIndicatorColor

#### Example

```xml
<ScrollPanel id="scroll" x="0" y="0" width="100" height="100" overscrollBarColor="#FF0E0F10" >

    <Button id="add" text="Add" font="mona12" x="5" y="5" width="40" height="40" textColor="#FFFF575E" onClick="SomeCallback" />
    <Button id="new" text="New" font="mona12" x="55" y="5" width="40" height="40" textColor="#FFFF575E" onClick="SomeCallback" />
    <Button id="cancel" text="Cancel" font="mona12" x="5" y="50" width="90" height="40" textColor="#FFFF575E" onClick="SomeCallback" />

</ScrollPanel>
```

### Other screens

Screens with no special attributes

#### CustomView

```
<customView id="start" backgroundColor="#FF4287F5">
        <button id="test_button" image="button_image" x="10" y="10" width="100" height="100" textColor="#ff00ff00" activeTextColor="#ffff00ff" onClick="ButtonCallback" text="TEST" font="roboto-medium" />
    <clock id="test_clock" x="150" y="10" width="100" height="100" font="roboto-medium" foregroundColor="#fffff000" format="%H:%M:%S" />
    <slider id="vertical_slider" x="300" y="10" width="20" height="100" scrollColor="#ffff0000" activeBarColor="#ff00ff00" />
    <slider id="horizontal_slider" x="340" y="10" width="100" height="20" scrollColor="#fffcba03" activeBarColor="#ffa903fc" />
</customView>
```

#### Header

```
<header height="50" backgroundColor="#ff000000">
    <label id="h_label" text="Header (Label)" font="roboto-medium" alignment="Center" x="0" y="0" width="800" height="50" />
</header>
```

#### Popup

```
<popup id="test_popup" backgroundColor="#ff000000" x="250" y="200" width="300" height="200">
        <button id="close_popup_btn" x="100" y="10" width="100" height="50" backgroundColor="#ffffffff" textColor="#ff0000ff" activeTextColor="#ffff00ff" onClick="ClosePopup" text="Close" font="roboto-medium" />
        <label id="popup_label" font="roboto-medium" x="0" y="100" width="300" height="50" text="This is a Popup." />
</popup>
```

## List of special components

### guiConfig

#### Attributes

* touchRegionModificator
* dotColor
* dotActiveColor
* dotDistance
* dotRadius
* dotYPos
* debugDot

#### Example

```xml
<guiConfig dotColor="#ffffffff"></guiConfig>
```

### stylesheet (UNIMPLEMENTED)

The current version of grvl only implements parsing, the stylesheet is not applied.

#### Example

```xml
<stylesheet>
    label {
        bgColor: "red"
    }
</stylesheet>
```

### keypadMapping

SDL (LinuxDesktop) and LinuxNative install default keyboard mappings during
application setup, before XML is loaded. Standard keys therefore need only an
action declaration inside a view, for example:

```xml
<key id="backspace" onPress="Backspace"/>
<key id="escape" onPress="QuitApplication"/>
```

`Backspace` and `QuitApplication` here are application-provided callbacks, not
built-in actions.
Default IDs are identical across these backends:

* `a`–`z`, `0`–`9`, `space`, `tab`, `enter`, `backspace`, `escape`
* `minus`, `equals`, `leftbracket`, `rightbracket`, `backslash`, `semicolon`,
  `apostrophe`, `grave`, `comma`, `period`, `slash`
* `leftshift`, `rightshift`, `leftctrl`, `rightctrl`, `leftalt`, `rightalt`,
  `leftmeta`, `rightmeta`, `menu`, `capslock`
* `f1`–`f24`, `printscreen`, `scrolllock`, `pause`
* `insert`, `delete`, `home`, `end`, `pageup`, `pagedown`, `left`, `right`, `up`, `down`
* `numlock`, `kp_0`–`kp_9`, `kp_period`, `kp_divide`, `kp_multiply`, `kp_minus`,
  `kp_plus`, `kp_enter`, `kp_equals`, `kp_comma`
* `mute`, `volumeup`, `volumedown`, `playpause`, `stop`, `nexttrack`, `previoustrack`

Letter and punctuation IDs follow SDL key symbols on desktop and physical evdev
keys (US key names) on LinuxNative. Other platforms and applications that
initialize `Manager` directly must provide their own mappings.

`keypadMapping` is optional. Its entries override defaults **by numeric code**;
unmentioned defaults remain. Giving another code the same ID adds an alias rather
than removing the original binding. Codes retain their backend-specific meaning:
SDL2 key symbols on desktop, evdev codes on LinuxNative. For example, Backspace
is `8` on SDL and `14` on LinuxNative. Codes support unsigned 32-bit values, including
full SDL special-key symbols (e.g. Left is `1073741904`). Explicit legacy bindings
using truncated 16-bit SDL codes are still honored ahead of backend defaults;
an explicit full-width binding takes precedence over a legacy binding. New XML
should use full-width values to avoid the ambiguity of truncated codes.

LinuxNative retains its existing modifier encoding: OR the evdev code with
`1024` for Ctrl, `2048` for Alt, and `4096` for Shift. Exact modified mappings take
precedence, falling back to the unmodified code. SDL continues to send key symbols
without this modifier encoding.

Contains key elements with the following attributes:

* id
* code (int)
* repeat (int, optional; repeat interval in milliseconds, default `100`)

#### Example

```xml
<keypadMapping>
    <key id="space" code="456" repeat="200"/>
    <key id="enter" code="789" repeat="100"/>
</keypadMapping>
```
