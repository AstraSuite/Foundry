import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Bar & Dashboard Sizes")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: TokenConfig.appearance.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Status Bar Widgets")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Inner Width")
            value: TokenConfig.sizes.bar.innerWidth
            from: 10
            to: 100
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.bar.innerWidth = v
            onReset: { TokenConfig.sizes.bar.innerWidth = TokenConfig.defaults().sizes.bar.innerWidth; TokenConfig.sizes.bar.resetOption("innerWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Window Preview Size")
            value: TokenConfig.sizes.bar.windowPreviewSize
            from: 100
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.bar.windowPreviewSize = v
            onReset: { TokenConfig.sizes.bar.windowPreviewSize = TokenConfig.defaults().sizes.bar.windowPreviewSize; TokenConfig.sizes.bar.resetOption("windowPreviewSize"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Tray Menu Width")
            value: TokenConfig.sizes.bar.trayMenuWidth
            from: 150
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.bar.trayMenuWidth = v
            onReset: { TokenConfig.sizes.bar.trayMenuWidth = TokenConfig.defaults().sizes.bar.trayMenuWidth; TokenConfig.sizes.bar.resetOption("trayMenuWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Battery Width")
            value: TokenConfig.sizes.bar.batteryWidth
            from: 100
            to: 400
            stepSize: 5
            showReset: true
            onMoved: v => TokenConfig.sizes.bar.batteryWidth = v
            onReset: { TokenConfig.sizes.bar.batteryWidth = TokenConfig.defaults().sizes.bar.batteryWidth; TokenConfig.sizes.bar.resetOption("batteryWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Network Width")
            value: TokenConfig.sizes.bar.networkWidth
            from: 150
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.bar.networkWidth = v
            onReset: { TokenConfig.sizes.bar.networkWidth = TokenConfig.defaults().sizes.bar.networkWidth; TokenConfig.sizes.bar.resetOption("networkWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Keyboard Layout Width")
            value: TokenConfig.sizes.bar.kbLayoutWidth
            from: 150
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.bar.kbLayoutWidth = v
            onReset: { TokenConfig.sizes.bar.kbLayoutWidth = TokenConfig.defaults().sizes.bar.kbLayoutWidth; TokenConfig.sizes.bar.resetOption("kbLayoutWidth"); }
        }

        SectionHeader {
            text: qsTr("Dashboard Widgets")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("User Card Width")
            value: TokenConfig.sizes.dashboard.userWidth
            from: 200
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.userWidth = v
            onReset: { TokenConfig.sizes.dashboard.userWidth = TokenConfig.defaults().sizes.dashboard.userWidth; TokenConfig.sizes.dashboard.resetOption("userWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Logo Size")
            value: TokenConfig.sizes.dashboard.logoSize
            from: 16
            to: 64
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.logoSize = v
            onReset: { TokenConfig.sizes.dashboard.logoSize = TokenConfig.defaults().sizes.dashboard.logoSize; TokenConfig.sizes.dashboard.resetOption("logoSize"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Uptime Text Size")
            value: TokenConfig.sizes.dashboard.uptimeSize
            from: 16
            to: 64
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.uptimeSize = v
            onReset: { TokenConfig.sizes.dashboard.uptimeSize = TokenConfig.defaults().sizes.dashboard.uptimeSize; TokenConfig.sizes.dashboard.resetOption("uptimeSize"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Date/Time Widget Width")
            value: TokenConfig.sizes.dashboard.dateTimeWidth
            from: 80
            to: 300
            stepSize: 5
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.dateTimeWidth = v
            onReset: { TokenConfig.sizes.dashboard.dateTimeWidth = TokenConfig.defaults().sizes.dashboard.dateTimeWidth; TokenConfig.sizes.dashboard.resetOption("dateTimeWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Weather Width")
            value: TokenConfig.sizes.dashboard.weatherWidth
            from: 150
            to: 400
            stepSize: 5
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.weatherWidth = v
            onReset: { TokenConfig.sizes.dashboard.weatherWidth = TokenConfig.defaults().sizes.dashboard.weatherWidth; TokenConfig.sizes.dashboard.resetOption("weatherWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Media Widget Width")
            value: TokenConfig.sizes.dashboard.mediaWidth
            from: 100
            to: 400
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.mediaWidth = v
            onReset: { TokenConfig.sizes.dashboard.mediaWidth = TokenConfig.defaults().sizes.dashboard.mediaWidth; TokenConfig.sizes.dashboard.resetOption("mediaWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Media Section Width")
            value: TokenConfig.sizes.dashboard.mediaSectionWidth
            from: 150
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.mediaSectionWidth = v
            onReset: { TokenConfig.sizes.dashboard.mediaSectionWidth = TokenConfig.defaults().sizes.dashboard.mediaSectionWidth; TokenConfig.sizes.dashboard.resetOption("mediaSectionWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Media Cover Art Size")
            value: TokenConfig.sizes.dashboard.mediaCoverArtSize
            from: 100
            to: 300
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.mediaCoverArtSize = v
            onReset: { TokenConfig.sizes.dashboard.mediaCoverArtSize = TokenConfig.defaults().sizes.dashboard.mediaCoverArtSize; TokenConfig.sizes.dashboard.resetOption("mediaCoverArtSize"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Media Tab Width")
            value: TokenConfig.sizes.dashboard.mediaTabWidth
            from: 400
            to: 1500
            stepSize: 20
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.mediaTabWidth = v
            onReset: { TokenConfig.sizes.dashboard.mediaTabWidth = TokenConfig.defaults().sizes.dashboard.mediaTabWidth; TokenConfig.sizes.dashboard.resetOption("mediaTabWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Media Tab Height")
            value: TokenConfig.sizes.dashboard.mediaTabHeight
            from: 200
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.dashboard.mediaTabHeight = v
            onReset: { TokenConfig.sizes.dashboard.mediaTabHeight = TokenConfig.defaults().sizes.dashboard.mediaTabHeight; TokenConfig.sizes.dashboard.resetOption("mediaTabHeight"); }
        }
    }
}
