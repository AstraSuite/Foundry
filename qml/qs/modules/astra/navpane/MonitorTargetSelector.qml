pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Quickshell
import AstraMarket.Config
import qs.components
import qs.components.containers
import qs.components.controls
import qs.services
import qs.modules.astra

StyledRect {
    id: root

    required property AstraState nState

    Layout.fillWidth: true
    implicitHeight: layout.implicitHeight + layout.anchors.margins * 2

    color: Colours.layer(Colours.palette.m3surfaceContainerHigh, 2)
    radius: Tokens.rounding.extraLarge

    Behavior on color {
        CAnim {}
    }

    Variants {
        id: screenVariants
        model: ["", ...Screens.screens.map(s => s.name)]

        MenuItem {
            required property string modelData
            text: modelData === "" ? qsTr("Global") : modelData
            icon: modelData === "" ? "globe" : "desktop_windows"
            onClicked: root.nState.targetScreen = modelData
        }
    }

    RowLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: Tokens.padding.large
        spacing: Tokens.spacing.medium

        StyledRect {
            Layout.fillHeight: true
            implicitWidth: height
            radius: Tokens.rounding.full
            color: Colours.palette.m3secondaryContainer

            MaterialIcon {
                anchors.centerIn: parent
                text: root.nState.targetScreen === "" ? "globe" : "desktop_windows"
                color: Colours.palette.m3onSecondaryContainer
                fontStyle: Tokens.font.icon.builders.medium.weight(Font.Medium).build()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            StyledText {
                Layout.fillWidth: true
                text: root.nState.targetScreen === "" ? qsTr("Global Config") : qsTr("Monitor: %1").arg(root.nState.targetScreen)
                font: Tokens.font.body.medium
                elide: Text.ElideRight
            }

            StyledText {
                Layout.fillWidth: true
                text: root.nState.targetScreen === "" ? qsTr("Applies to all monitors") : qsTr("Per-monitor overrides")
                color: Colours.palette.m3onSurfaceVariant
                font: Tokens.font.label.small
                elide: Text.ElideRight
            }
        }

        SplitButton {
            id: splitButton

            type: SplitButton.Tonal
            fallbackIcon: root.nState.targetScreen === "" ? "globe" : "desktop_windows"
            fallbackText: root.nState.targetScreen === "" ? qsTr("Global") : root.nState.targetScreen

            menuItems: screenVariants.instances ?? []

            active: (screenVariants.instances ?? []).find(i => (root.nState.targetScreen === "" && i.text === qsTr("Global")) || i.text === root.nState.targetScreen) ?? null
        }
    }
}
