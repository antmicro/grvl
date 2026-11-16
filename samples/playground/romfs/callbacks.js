// simple_demo_callbacks.js

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

function CheckboxCallback(caller) {
    const callerName = caller.name
    if (caller.switchState) {
        Print("Checkbox is ON! (" + callerName + ")")
    } else {
        Print("Checkbox is OFF! (" + callerName + ")")
    }
}

function ShowPopupCallback(caller, popup) {
	Print("Popup shown!")
    ShowPopup(popup)
}

function ClosePopupCallback() {
	Print("Popup closed!")
	ClosePopup()
}
