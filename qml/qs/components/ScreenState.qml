import QtQuick
import Quickshell

PersistentProperties {
    required property var modelData

    property bool bar
    property bool osd
    property bool session
    property bool launcher
    property bool dashboard
    property bool utilities
    property bool sidebar
    property bool workspaceDrawer

    property int dashboardTab
    property date dashboardDate: new Date()
}
