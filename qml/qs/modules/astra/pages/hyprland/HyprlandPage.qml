import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Hyprland")

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        NavRow {
            first: true
            icon: "data_object"
            text: qsTr("Variables")
            subtext: qsTr("Manage your hyprland variables")
            onClicked: root.nState.openSubPage(1)
        }

        NavRow {
            icon: "keyboard"
            text: qsTr("Keybinds")
            subtext: qsTr("Manage your hyprland keybinds")
            onClicked: root.nState.openSubPage(2)
        }

        NavRow {
            last: true
            icon: "settings_suggest"
            text: qsTr("User configuration")
            subtext: qsTr("Advanced configuration options")
            onClicked: root.nState.openSubPage(3)
        }
    }
}
