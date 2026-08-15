import QtQuick

Item {
    property var command: []
    property bool running: false
    property var environment
    property var stdout
    property var stderr
    signal exited(int code)
    function run() {}
}
