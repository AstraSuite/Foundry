pragma Singleton
import QtQuick

QtObject {
    property var toplevels: []
    property var workspaces: []
    property var monitors: []
    property bool usingLua: false
    property var activeToplevel: null
    property var focusedWorkspace: null
    property var focusedMonitor: null

    signal rawEvent(var event)
}
