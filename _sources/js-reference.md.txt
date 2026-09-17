# JavaScript reference

## JavaScript feature support

The JavaScript engine used in grvl is [duktape](https://github.com/svaarala/duktape). For more information about its capabilities, see [duktape docs](https://github.com/svaarala/duktape?tab=readme-ov-file#introduction).

## How to use JavaScript with grvl?

First, prepare your JavaScript source file:

```javascript
// callbacks.js

var currentDate = null

function ButtonCallback(caller) {
    const buttonId = caller.name
    Print("Button clicked! (" + buttonId + ")")
}

function InitializeApplication() {
    currentDate = new Date()
}
```

Then, include it in your application's XML layout:

```xml
<script src="callbacks.js"></script>
```

You can specify a non-default working directory for JavaScript files with:

```cpp
grvl::JSEngine::SetSourceCodeWorkingDirectory("Scripts/JavaScript/");
```

You can later bind JavaScript functions to GUI components by referencing a function name in callback parameters, e.g. `onClick`. The caller component will be passed in as the first argument:

```xml
<button id="button" x="10" y="140" width="200" height="80" onClick="ButtonCalback" text="I am a button" font="roboto" />
```

JavaScript functions can be also called from any place in code without involving GUI components like this:

```cpp
grvl::JSEngine::MakeJavaScriptFunctionCall("InitializeApplication");
```

## grvl functions available in JavaScript

You can call functions exposed by grvl to control your application from JavaScript. They are divided into two types: globally available functions and GUI component accessors.

### Globally available functions

These functions can be called from any place in JavaScript code:

- `GetElementById(componentID)` - returns component with given ID if found
- `Print(message)` - prints given message
- `ShowPopup(popupID)` - shows popup with given ID if available
- `ClosePopup()` - closes currently shown popup
- `SetActiveScreen(screenID)` - sets screen with given ID as active
- `GetTopPanel()` - returns top panel component
- `GetBottomPanel()` - returns bottom panel component
- `GetPrefabById(prefabID)` - returns prefab with given ID if available

### Members

These functions can be used to access GUI component parameters or call their member functions.

#### Properties

The simple parameters of a GUI component can be modified with properties exposed by each component, e.g.:

```javascript
Print(caller.name)
```

will print `caller`'s ID. There are other common properties such as:

- `x`, `y`, `width` and `height` for position and size dimensions respectively
- `foregroundColor` and `backgroundColor` for component's foreground and background color
- `visibility` which determines if a component is visible.

#### Metadata

Each component's metadata can be accessed by calling `AddMetadata(key, value)` or `GetMetadata(key)`:

```javascript
button.AddMetadata("description", "grvl is very cool")
...
const description = button.GetMetadata("description")
```

#### Cloning

GUI components can be cloned inside JavaScript code by calling:

```javascript
const clone = caller.Clone()
container.AddElement(clone)
```
