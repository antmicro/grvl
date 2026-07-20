function OnExitButton(caller) {
    Exit(0);
}

function OnRotate90(caller) {
    Rotate90();
}

function GetSpinner() {
    return GetElementById("spinner");
}

function SetAnimationEnabled(enabled) {
    const spinner = GetSpinner();
    spinner.animationEnabled = enabled;
}

function SetLoopingEnabled(enabled) {
    const spinner = GetSpinner();
    spinner.animationLoop = enabled;
}

function EnableAnimation() {
    SetAnimationEnabled(true);
}

function DisableAnimation() {
    SetAnimationEnabled(false);
}

function EnableLooping() {
    SetLoopingEnabled(true);
}

function DisableLooping() {
    SetLoopingEnabled(false);
}

function TriggerAnimation() {
    const spinner = GetSpinner();

    spinner.RestartAnimation();
    GetElementById("animation-switch").switchState = true;
}
