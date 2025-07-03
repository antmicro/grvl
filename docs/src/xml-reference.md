# XML reference

## XML validity

A [schema](../../xml/grvl.xsd) is available in grvl along with an example that it validates.
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

## Generic component/container attributes

Each component/container has the following attributes:

* id
* x (absolute, in pixels)
* y (absolute, in pixels)
* width
* height
* visible (true/false)
* backgroundColor
* onClick (callback)
* onPress (callback)

## Generic screen attributes

Each screen is a container, so it has all of the attributes from the [previous section](#generic-componentcontainer-attributes) as well as:

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

## List of components/containers

### Label

#### Attributes

* text
* font
* color
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
* activeTextColor
* icoFont
* icoColor
*

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
* seconds (true/false)

#### Example

```xml
<clock id="test_clock" x="150" y="10" width="100" height="100" font="roboto-medium" foregroundColor="#fffff000" seconds="true" />
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
* isDiscrete
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

### SwitchButon

#### Attributes

* text
* font
* frameColor
* selectedFrameColor
* switchColor
* activeSwitchColor
* onSwitchON
* onSwitchOFF
* onLongPress
* onLongPressRepeat
* image

#### Example

```xml
<SwitchButton id="test_switch" x="0" y="60" width="80" height="50" textColor="#ffff00ff" backgroundColor="#ffffffff" switchColor="#fffcba03" font="roboto-medium" onSwitchON="SwitchCallback" onSwitchOFF="SwitchCallback" />
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

## List of screens

### GridView

#### Attributes

* overscrollEnabled
* overscrollHeight
* scrollingEnabled
* overscrollColor
* elementWidth
* elementHeight
* verticalOffset

#### Example

### ListView

#### Attributes

* overscrollEnabled
* overscrollHeight
* splitLineColor
* overscrollColor
* scrollIndicatorColor

#### Example

### Other screens

Screens with no special attributes

#### CustomView

```
<customView id="start" backgroundColor="#FF4287F5">
        <button id="test_button" image="button_image" x="10" y="10" width="100" height="100" textColor="#ff00ff00" activeTextColor="#ffff00ff" onClick="ButtonCallback" text="TEST" font="roboto-medium" />
    <clock id="test_clock" x="150" y="10" width="100" height="100" font="roboto-medium" foregroundColor="#fffff000" seconds="true" />
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

Contains key elements with the following attributes:

* id
* code (int)
* repeat (int)

#### Example

```xml
<keypadMapping>
    <key id="space" code="456" repeat="200"/>
    <key id="enter" code="789" repeat="100"/>
</keypadMapping>
```
