import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Background elements")

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        NavRow {
            first: true
            icon: "schedule"
            text: qsTr("Desktop clock")
            subtext: root.targetConfig.background.desktopClock.enabled ? qsTr("Enabled") : qsTr("Disabled")
            onClicked: root.nState.openSubPage(1)
        }

        NavRow {
            icon: "lyrics"
            text: qsTr("Desktop lyrics")
            subtext: root.targetConfig.background.desktopLyrics.enabled ? qsTr("Enabled") : qsTr("Disabled")
            onClicked: root.nState.openSubPage(2)
        }

        NavRow {
            icon: "equalizer"
            text: qsTr("Background visualiser")
            subtext: root.targetConfig.background.visualiser.enabled ? qsTr("Enabled") : qsTr("Disabled")
            onClicked: root.nState.openSubPage(3)
        }

        NavRow {
            last: true
            icon: "pets"
            text: qsTr("Shimeji characters")
            subtext: root.targetConfig.shimeji.enabled ? qsTr("Enabled") : qsTr("Disabled")
            onClicked: root.nState.openSubPage(4)
        }
    }
}
