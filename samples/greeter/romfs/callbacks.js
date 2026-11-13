const CORRECT_PASSWORD = "Password";
const INCORRECT_PASSWORD_BG_COLOR = 0xFF220807;
const INCORRECT_PASSWORD_BORDER_COLOR = 0xFFFF5A59;
const INCORRECT_PASSWORD_BUTTON_COLOR = 0xFFFFACA3;
const INCORRECT_PASSWORD_TEXT_COLOR = 0xFFFFACA3;
var greeter_passed = false;

function PasswordSubmit() {
        if (greeter_passed) {
                return;
        }

        var text_input = GetElementById("answer");
        var proceed_button = GetElementById("proceed");
        var bad_password_label = GetElementById("bad_password_label");
        var good_password_label = GetElementById("good_password_label");
        var success_icon = GetElementById("success_icon");
        var avatar = GetElementById("avatar");
        var proceed_button = GetElementById("proceed");

        if (text_input.text == CORRECT_PASSWORD) {
                text_input.visibility = false;
                bad_password_label.visibility = false;
                avatar.visibility = false;
                proceed_button.visibility = false;
                good_password_label.visibility = true;
                success_icon.visibility = true;

                greeter_passed = true;
        } else {
                text_input.text = "";
                text_input.backgroundColor = INCORRECT_PASSWORD_BG_COLOR;
                text_input.borderColor = INCORRECT_PASSWORD_BORDER_COLOR;
                text_input.textColor = INCORRECT_PASSWORD_TEXT_COLOR;
                text_input.activeTextColor = INCORRECT_PASSWORD_TEXT_COLOR;

                proceed_button.backgroundColor = INCORRECT_PASSWORD_BUTTON_COLOR;
                proceed_button.borderColor = INCORRECT_PASSWORD_BUTTON_COLOR;

                bad_password_label.visibility = true;
        }
}
