// example JavaScript callbacks

function ButtonCallback(caller) {
    const button_id = caller.name
	Print("Button clicked! (" + button_id + ")")
}

function SwitchCallback(caller) {
    caller_name = caller.name
    if (caller.switch_state) {
        Print("Switch is ON! (" + caller_name + ")")
    } else {
        Print("Switch is OFF! (" + caller_name + ")")
    }
}

function ShowPopupCallback(caller, popup_text) {
	Print("Popup shown!")
    ShowPopup(popup_text)
}

function ClosePopupCallback() {
	Print("Popup closed!")
	ClosePopup()
}
