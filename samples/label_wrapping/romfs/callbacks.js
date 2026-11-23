function OnExitButton() {
    Exit(0);
}

function ToggleWrapping() {
    var label = GetElementById("dynamic_label");
    label.wrapping = !label.wrapping;
}

function ToggleWidth() {
    var label = GetElementById("dynamic_label");
    label.width = (label.width === 370) ? 180 : 370;
}
