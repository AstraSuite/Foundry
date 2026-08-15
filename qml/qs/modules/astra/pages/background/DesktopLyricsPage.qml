import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components.controls
import qs.modules.astra.common

PageBase {
    id: root
    title: qsTr("Desktop lyrics")
    isSubPage: true

    readonly property list<string> positionValues: ["top-left", "top-center", "top-right", "middle-left", "middle-center", "middle-right", "bottom-left", "bottom-center", "bottom-right"]
    readonly property list<MenuItem> positionItems: [
        MenuItem { text: qsTr("Top Left") },
        MenuItem { text: qsTr("Top Center") },
        MenuItem { text: qsTr("Top Right") },
        MenuItem { text: qsTr("Middle Left") },
        MenuItem { text: qsTr("Middle Center") },
        MenuItem { text: qsTr("Middle Right") },
        MenuItem { text: qsTr("Bottom Left") },
        MenuItem { text: qsTr("Bottom Center") },
        MenuItem { text: qsTr("Bottom Right") }
    ]

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("General")
        }

        ToggleRow {
            first: true
            Layout.fillWidth: true
            text: qsTr("Enable desktop lyrics")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "enabled"
            checked: root.targetConfig.background.desktopLyrics.enabled
            onToggled: {
                root.targetConfig.background.desktopLyrics.enabled = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            text: qsTr("Overlay lyrics")
            subtext: qsTr("Render lyrics on top of all windows")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "overlay"
            checked: root.targetConfig.background.desktopLyrics.overlay
            onToggled: {
                root.targetConfig.background.desktopLyrics.overlay = checked;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.enabled
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            text: qsTr("Auto-hide for fullscreen windows")
            subtext: qsTr("Hide lyrics when a window is fullscreen")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "autoHideFullscreen"
            checked: root.targetConfig.background.desktopLyrics.autoHideFullscreen
            onToggled: {
                root.targetConfig.background.desktopLyrics.autoHideFullscreen = checked;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.enabled
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            text: qsTr("Auto-hide for tiled windows")
            subtext: qsTr("Hide lyrics when tiled windows are open")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "autoHideTiled"
            checked: root.targetConfig.background.desktopLyrics.autoHideTiled
            onToggled: {
                root.targetConfig.background.desktopLyrics.autoHideTiled = checked;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.enabled
        }

        SectionHeader {
            text: qsTr("Appearance")
        }

        SelectRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Position")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "position"
            menuItems: root.positionItems
            active: root.positionItems[root.positionValues.indexOf(root.targetConfig.background.desktopLyrics.position)] ?? root.positionItems[4]
            onSelected: item => {
                let idx = root.positionItems.indexOf(item);
                if (idx !== -1) {
                    root.targetConfig.background.desktopLyrics.position = root.positionValues[idx];
                    root.targetConfig.save();
                }
            }
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Scale")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "scale"
            value: (root.targetConfig.background.desktopLyrics.scale - 0.5) / 2.5
            valueLabel: (0.5 + value * 2.5).toFixed(1) + "x"
            onMoved: v => {
                root.targetConfig.background.desktopLyrics.scale = 0.5 + v * 2.5;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            text: qsTr("Invert colors")
            subtext: qsTr("Invert the lyrics color when using light wallpaper")
            configNode: root.targetConfig.background.desktopLyrics
            propertyName: "invertColors"
            checked: root.targetConfig.background.desktopLyrics.invertColors
            onToggled: {
                root.targetConfig.background.desktopLyrics.invertColors = checked;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Background")
        }

        ToggleRow {
            first: true
            Layout.fillWidth: true
            text: qsTr("Enable background")
            configNode: root.targetConfig.background.desktopLyrics.background
            propertyName: "enabled"
            checked: root.targetConfig.background.desktopLyrics.background.enabled
            onToggled: {
                root.targetConfig.background.desktopLyrics.background.enabled = checked;
                root.targetConfig.save();
            }
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Opacity")
            configNode: root.targetConfig.background.desktopLyrics.background
            propertyName: "opacity"
            value: root.targetConfig.background.desktopLyrics.background.opacity
            valueLabel: Math.round(value * 100) + "%"
            onMoved: v => {
                root.targetConfig.background.desktopLyrics.background.opacity = v;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.background.enabled
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            text: qsTr("Blur")
            configNode: root.targetConfig.background.desktopLyrics.background
            propertyName: "blur"
            checked: root.targetConfig.background.desktopLyrics.background.blur
            onToggled: {
                root.targetConfig.background.desktopLyrics.background.blur = checked;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.background.enabled
        }

        SectionHeader {
            text: qsTr("Shadow")
        }

        ToggleRow {
            first: true
            Layout.fillWidth: true
            text: qsTr("Enable shadow")
            configNode: root.targetConfig.background.desktopLyrics.shadow
            propertyName: "enabled"
            checked: root.targetConfig.background.desktopLyrics.shadow.enabled
            onToggled: {
                root.targetConfig.background.desktopLyrics.shadow.enabled = checked;
                root.targetConfig.save();
            }
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Opacity")
            configNode: root.targetConfig.background.desktopLyrics.shadow
            propertyName: "opacity"
            value: root.targetConfig.background.desktopLyrics.shadow.opacity
            valueLabel: Math.round(value * 100) + "%"
            onMoved: v => {
                root.targetConfig.background.desktopLyrics.shadow.opacity = v;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.shadow.enabled
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Blur strength")
            configNode: root.targetConfig.background.desktopLyrics.shadow
            propertyName: "blur"
            value: root.targetConfig.background.desktopLyrics.shadow.blur / 2.0
            valueLabel: (value * 2.0).toFixed(1)
            onMoved: v => {
                root.targetConfig.background.desktopLyrics.shadow.blur = v * 2.0;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.desktopLyrics.shadow.enabled
        }
    }
}
