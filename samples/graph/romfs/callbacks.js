
var overlay = false;

function GetGraph() {
    return GetElementById("test_graph");
}

function ToggleOverlay() {
    overlay = !overlay;
    SetDebugOverlay(overlay);
}

function OnExitButton(caller) {
    Exit(0);
}

function OnAddButton(caller) {
    var g = GetGraph();

    g.AddData(1.1);
    g.AddData(3.3);
    g.AddData(4.6);
    g.AddData(2.7);
    g.AddData(5.1);
    g.AddData(3.9);
    g.AddData(4.2);
    g.AddData(7.0);
    g.AddData(9.3);
    g.AddData(8.7);
    g.AddData(7.3);
    g.AddData(9.2);
    g.AddData(10.4);
    g.AddData(12.5);
    g.AddData(13.6);
    g.AddData(10.9);
    g.AddData(12.1);
    g.AddData(15.2);
}

function OnClearButton(caller) {
    var g = GetGraph();

    g.ClearData();
}
