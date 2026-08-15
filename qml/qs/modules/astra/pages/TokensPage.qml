import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Shell Tokens")

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: TokenConfig.appearance.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("General")
        }

        NavRow {
            first: true
            icon: "grid_view"
            text: qsTr("Rounding & Spacing")
            subtext: qsTr("Adjust outer margins, paddings, and corner rounding")
            onClicked: root.nState.openSubPage(1)
        }

        NavRow {
            icon: "format_size"
            text: qsTr("Font Sizes")
            subtext: qsTr("Set typography scaling for interface text")
            onClicked: root.nState.openSubPage(2)
        }

        NavRow {
            icon: "space_dashboard"
            text: qsTr("Bar & Dashboard Sizes")
            subtext: qsTr("Adjust widths of top bar widgets and dashboard tiles")
            onClicked: root.nState.openSubPage(3)
        }

        NavRow {
            icon: "desktop_windows"
            text: qsTr("Window & Lock Sizes")
            subtext: qsTr("Modify proportions for main settings window and lock screen")
            onClicked: root.nState.openSubPage(4)
        }

        NavRow {
            last: true
            icon: "widgets"
            text: qsTr("Shell Element Sizes")
            subtext: qsTr("Configure launcher grid, notifications, OSD, and sidebars")
            onClicked: root.nState.openSubPage(5)
        }
    }
}
