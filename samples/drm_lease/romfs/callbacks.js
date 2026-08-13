function OnExitButton(caller) {
  Print("Pressed exit button");
  Exit(0);
}

function OnTextInput(caller) {
  GetElementById("input_status").text = "Input received: " + caller.text;
}

function OnTextSubmit(caller) {
  GetElementById("input_status").text = "Submitted: " + caller.text;
}