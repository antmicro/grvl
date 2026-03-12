# On-screen keyboard

To provide an interactive experience for the user, you can add a keyboard, along with text inputs, to your grvl application. The keyboard can be specified in the application XML layout:

```xml
<Keyboard id="keyboard" backgroundColor="#FF0D0D0E" x="0" y="430" width="1024" height="338">

    <KeyboardKey id="g" text="g" font="mona26" secondaryText="a" secondaryTextFont="mona14" x="25" y="27" width="75" height="62" backgroundColor="#FF191A1C" textColor="#FFECEDEE" secondaryTextColor="#FF191A1C" onClick="AppendFromKeyboardKey" />
    <KeyboardKey id="r" text="r" font="mona26" secondaryText="n" secondaryTextFont="mona14" x="115" y="27" width="75" height="62" backgroundColor="#FF191A1C" textColor="#FFECEDEE" secondaryTextColor="#FF191A1C" onClick="AppendFromKeyboardKey" />
    <KeyboardKey id="v" text="v" font="mona26" secondaryText="t" secondaryTextFont="mona14" x="205" y="27" width="75" height="62" backgroundColor="#FF191A1C" textColor="#FFECEDEE" secondaryTextColor="#FF191A1C" onClick="AppendFromKeyboardKey" />
    <KeyboardKey id="l" text="l" font="mona26" secondaryText="m" secondaryTextFont="mona14" x="295" y="27" width="75" height="62" backgroundColor="#FF191A1C" textColor="#FFECEDEE" secondaryTextColor="#FF191A1C" onClick="AppendFromKeyboardKey" />

</Keyboard>
```

You can specify any layout you require by defining individual keys. The keyboard will be shown whenever a text input is used.

```xml
<TextInput id="name" basicText="Name" font="mona26"
    x="0" y="0" width="426" height="34"
    borderType="box" borderArcRadius="12"
    backgroundColor="#FF191A1C" borderColor="#FF2C2E32" textColor="#FF9EA2A6" />
```
