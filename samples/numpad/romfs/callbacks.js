const NORMAL_BACKGROUND = 0xff263548;
const ACTIVE_BACKGROUND = 0xff3977ec;
const NUMLOCK_BACKGROUND = 0xff195a51;
const TEXT_COLOR = 0xfff0f6ff;
var down = {};
var currentMode = null;

function PaintKey(id) {
    var widget = GetElementById(id);
    if (!widget) return;
    var color = NORMAL_BACKGROUND;
    if (id === 'pad_num' && currentMode === true) color = NUMLOCK_BACKGROUND;
    if (down[id]) color = ACTIVE_BACKGROUND;
    widget.backgroundColor = color;
    widget.textColor = TEXT_COLOR;
}

function OnKeyPressed(id, logicalKey) {
    down[id] = true;
    PaintKey(id);
    var info = 'PRESS ' + logicalKey + '  [Num Lock ' + (currentMode ? 'ON' : 'OFF') + ']';
    GetElementById('last_event').text = info;
    Print(info);
    ScheduleRedraw();
}

function OnKeyReleased(id, logicalKey) {
    down[id] = false;
    PaintKey(id);
    var info = 'RELEASE ' + logicalKey + '  [Num Lock ' + (currentMode ? 'ON' : 'OFF') + ']';
    GetElementById('last_event').text = info;
    Print(info);
    ScheduleRedraw();
}

function UpdateNumLockLabels() {
    var nextMode = Boolean(numLockEnabled);
    if (currentMode === nextMode) return;
    currentMode = nextMode;
    var captions = {
        'pad_7': ['Home', '7'],
        'pad_8': ['Up', '8'],
        'pad_9': ['PageUp', '9'],
        'pad_4': ['Left', '4'],
        'pad_5': ['', '5'], // KP5 has no navigation mapping in the GRVL
        'pad_6': ['Right', '6'],
        'pad_1': ['End', '1'],
        'pad_2': ['Down', '2'],
        'pad_3': ['PageDn', '3'],
        'pad_0': ['Insert', '0'],
        'pad_dot': ['Delete', '.']
    };
    for (var id in captions) {
        if (captions.hasOwnProperty(id)) {
            GetElementById(id).text = captions[id][nextMode ? 1 : 0];
        }
    }
    var status = GetElementById('mode_status');
    status.text = nextMode ? 'NUM LOCK ON  |  Numeric mode' : 'NUM LOCK OFF  |  Navigation mode';
    status.textColor = nextMode ? 0xff50e3b0 : 0xff93c5fd;
    PaintKey('pad_num');
    Print('Num Lock: ' + (nextMode ? 'ON' : 'OFF'));
    ScheduleRedraw();
}
