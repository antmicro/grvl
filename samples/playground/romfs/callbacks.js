// simple_demo_callbacks.js

function ButtonCallback(caller) {
    var buttonId = caller.name
	Print("Button clicked! (" + buttonId + ")")
}

function SwitchCallback(caller) {
    callerName = caller.name
    if (caller.switchState) {
        Print("Switch is ON! (" + callerName + ")")
    } else {
        Print("Switch is OFF! (" + callerName + ")")
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
