import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Shell Element Sizes")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: TokenConfig.appearance.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("App Launcher")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Item Width")
            value: TokenConfig.sizes.launcher.itemWidth
            from: 200
            to: 1000
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.launcher.itemWidth = v
            onReset: { TokenConfig.sizes.launcher.itemWidth = TokenConfig.defaults().sizes.launcher.itemWidth; TokenConfig.sizes.launcher.resetOption("itemWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Item Height")
            value: TokenConfig.sizes.launcher.itemHeight
            from: 20
            to: 150
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.launcher.itemHeight = v
            onReset: { TokenConfig.sizes.launcher.itemHeight = TokenConfig.defaults().sizes.launcher.itemHeight; TokenConfig.sizes.launcher.resetOption("itemHeight"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Wallpaper Grid Width")
            value: TokenConfig.sizes.launcher.wallpaperWidth
            from: 100
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.launcher.wallpaperWidth = v
            onReset: { TokenConfig.sizes.launcher.wallpaperWidth = TokenConfig.defaults().sizes.launcher.wallpaperWidth; TokenConfig.sizes.launcher.resetOption("wallpaperWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Wallpaper Grid Height")
            value: TokenConfig.sizes.launcher.wallpaperHeight
            from: 80
            to: 400
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.launcher.wallpaperHeight = v
            onReset: { TokenConfig.sizes.launcher.wallpaperHeight = TokenConfig.defaults().sizes.launcher.wallpaperHeight; TokenConfig.sizes.launcher.resetOption("wallpaperHeight"); }
        }

        SectionHeader {
            text: qsTr("Notifications")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Notification Width")
            value: TokenConfig.sizes.notifs.width
            from: 200
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.notifs.width = v
            onReset: { TokenConfig.sizes.notifs.width = TokenConfig.defaults().sizes.notifs.width; TokenConfig.sizes.notifs.resetOption("width"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("App Image Size")
            value: TokenConfig.sizes.notifs.image
            from: 16
            to: 96
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.notifs.image = v
            onReset: { TokenConfig.sizes.notifs.image = TokenConfig.defaults().sizes.notifs.image; TokenConfig.sizes.notifs.resetOption("image"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Badge Indicator Size")
            value: TokenConfig.sizes.notifs.badge
            from: 8
            to: 48
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.notifs.badge = v
            onReset: { TokenConfig.sizes.notifs.badge = TokenConfig.defaults().sizes.notifs.badge; TokenConfig.sizes.notifs.resetOption("badge"); }
        }

        SectionHeader {
            text: qsTr("OSD & Volume Sliders")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Slider Width")
            value: TokenConfig.sizes.osd.sliderWidth
            from: 8
            to: 100
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.osd.sliderWidth = v
            onReset: { TokenConfig.sizes.osd.sliderWidth = TokenConfig.defaults().sizes.osd.sliderWidth; TokenConfig.sizes.osd.resetOption("sliderWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Slider Height")
            value: TokenConfig.sizes.osd.sliderHeight
            from: 50
            to: 400
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.osd.sliderHeight = v
            onReset: { TokenConfig.sizes.osd.sliderHeight = TokenConfig.defaults().sizes.osd.sliderHeight; TokenConfig.sizes.osd.resetOption("sliderHeight"); }
        }

        SectionHeader {
            text: qsTr("Desktop Popouts & Panels")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Session Menu Button Size")
            value: TokenConfig.sizes.session.button
            from: 40
            to: 150
            stepSize: 5
            showReset: true
            onMoved: v => TokenConfig.sizes.session.button = v
            onReset: { TokenConfig.sizes.session.button = TokenConfig.defaults().sizes.session.button; TokenConfig.sizes.session.resetOption("button"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Sidebar Width")
            value: TokenConfig.sizes.sidebar.width
            from: 200
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.sidebar.width = v
            onReset: { TokenConfig.sizes.sidebar.width = TokenConfig.defaults().sizes.sidebar.width; TokenConfig.sizes.sidebar.resetOption("width"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Utilities Drawer Width")
            value: TokenConfig.sizes.utilities.width
            from: 200
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.utilities.width = v
            onReset: { TokenConfig.sizes.utilities.width = TokenConfig.defaults().sizes.utilities.width; TokenConfig.sizes.utilities.resetOption("width"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Toast Notification Width")
            value: TokenConfig.sizes.utilities.toastWidth
            from: 200
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.utilities.toastWidth = v
            onReset: { TokenConfig.sizes.utilities.toastWidth = TokenConfig.defaults().sizes.utilities.toastWidth; TokenConfig.sizes.utilities.resetOption("toastWidth"); }
        }
    }
}
