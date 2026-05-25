function OnSomeButton(caller) {
  Print("Pressed button");
}

function OnSomeCheckbox(caller) {
  Print("Pressed checkbox state=" + (caller.switchState ? "on" : "off"));
}

function OnExitButton(caller) {
  Print("Pressed exit button");
  Exit(0);
}
