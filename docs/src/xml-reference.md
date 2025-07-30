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

Derives all attributes from [SwitchButton](#SwitchButon), but renders its state indicator from width and height size dimensions, instead of state indicator parameters.

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
