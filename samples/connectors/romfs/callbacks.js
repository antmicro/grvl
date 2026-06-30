const connectorBackgroundColor = 0xFF1B1B1B;
const selectedConnectorBackgroundColor = 0xFF345A85;
const connectorTextColor = 0xFFF2F2F2;
const selectedConnectorTextColor = 0xFFFFFFFF;

var selectedConnectorIndex = -1;
var modeItemNames = [];

function FormatConnector(display) {
    const kind = display.builtIn ? "built-in" : "external";
    return display.drmPath + " / connector " + display.connectorId + " (" + kind + ")";
}

function FormatMode(mode) {
    var result = mode.name + "  " + mode.width + "x" + mode.height + " @ " + mode.refresh + " Hz";
    if (mode.preferred) {
        result += "  preferred";
    }
    return result;
}

function InitializeDisplayLists() {
    const connectorsList = GetElementById("connectorsList");
    const connectorItemPrefab = GetPrefabById("connectorItem");

    for (var index = 0; index < connectedDisplays.length; ++index) {
        const item = connectorItemPrefab.Clone();
        item.AddMetadata("index", String(index));
        item.GetElementByIndex(0).text = FormatConnector(connectedDisplays[index]);
        item.GetElementByIndex(0).AddMetadata("index", String(index));
        connectorsList.AddElement(item);
    }

    if (connectedDisplays.length > 0) {
        SelectConnectorByIndex(0);
    }
}

function SelectConnector(caller) {
    SelectConnectorByIndex(Number(caller.GetMetadata("index")));
}

function SelectConnectorByIndex(index) {
    if (index < 0 || index >= connectedDisplays.length) {
        return;
    }

    selectedConnectorIndex = index;
    UpdateConnectorSelection();
    UpdateModeList();
}

function UpdateConnectorSelection() {
    const connectorsList = GetElementById("connectorsList");

    for (var index = 0; index < connectedDisplays.length; ++index) {
        const item = connectorsList.GetElementByIndex(index);
        const selected = index === selectedConnectorIndex;
        item.backgroundColor = selected ? selectedConnectorBackgroundColor : connectorBackgroundColor;
        item.textColor = selected ? selectedConnectorTextColor : connectorTextColor;
    }
}

function UpdateModeList() {
    const modesList = GetElementById("modesList");

    for (var index = 0; index < modeItemNames.length; ++index) {
        modesList.RemoveElement(modeItemNames[index]);
    }
    modeItemNames = [];

    const selectedDisplay = connectedDisplays[selectedConnectorIndex];
    GetElementById("modesTitle").text = "Modes: connector " + selectedDisplay.connectorId;

    const modeItemPrefab = GetPrefabById("modeItem");
    for (var index = 0; index < selectedDisplay.modes.length; ++index) {
        const item = modeItemPrefab.Clone();
        item.name = "mode_" + index;
        item.GetElementByIndex(0).text = FormatMode(selectedDisplay.modes[index]);
        modesList.AddElement(item);
        modeItemNames.push(item.name);
    }

    modesList.Refresh();
}

function OnExitButton(caller) {
  Print("Pressed exit button");
  Exit(0);
}
