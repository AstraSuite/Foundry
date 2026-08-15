pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Caelestia
import Caelestia.Components
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.components.images
import qs.services
import qs.utils
import qs.modules.astra.common

PageBase {
    id: root

    readonly property list<MenuItem> hwDecoderItems: [
        MenuItem { text: qsTr("Auto") },
        MenuItem { text: qsTr("Software") },
        MenuItem { text: "VAAPI" },
        MenuItem { text: "VDPAU" },
        MenuItem { text: "CUDA" },
        MenuItem { text: "Vulkan" },
        MenuItem { text: "DRM" }
    ]
    readonly property list<string> hwDecoderValues: ["auto", "none", "vaapi", "vdpau", "cuda", "vulkan", "drm"]
    readonly property var hwDecoderIndexMap: ({
        "auto": 0, "none": 1, "vaapi": 2, "vdpau": 3,
        "cuda": 4, "vulkan": 5, "drm": 6
    })

    function hwDecoderToIndex(val: string): int {
        const v = (val ?? "none").toLowerCase();
        return v in hwDecoderIndexMap ? hwDecoderIndexMap[v] : 1;
    }

    title: qsTr("Wallpaper & style")

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.large

        StyledClippingRect {
            id: wallWrapper

            Layout.alignment: Qt.AlignHCenter
            implicitWidth: {
                const screen = root.nState.screen;
                return implicitHeight / screen.height * screen.width;
            }
            implicitHeight: {
                const screen = root.nState.screen;
                const cWidth = root.cappedWidth;
                return Math.min(Math.round(cWidth * 0.4), cWidth / screen.width * screen.height);
            }

            color: Colours.tPalette.m3surfaceContainer
            radius: Tokens.rounding.large

            Loader {
                anchors.centerIn: parent
                opacity: Config.background.wallpaperEnabled ? 0 : 1
                active: opacity > 0

                sourceComponent: ColumnLayout {
                    spacing: Tokens.spacing.extraSmall

                    MaterialIcon {
                        Layout.alignment: Qt.AlignHCenter
                        text: "hide_image"
                        color: Colours.palette.m3onSurfaceVariant
                        fontStyle: Tokens.font.icon.extraLarge
                    }

                    StyledText {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Wallpaper disabled")
                        color: Colours.palette.m3onSurfaceVariant
                        font: Tokens.font.body.large
                    }
                }

                Behavior on opacity {
                    Anim {
                        type: Anim.SlowEffects
                    }
                }
            }

            Item {
                anchors.fill: parent
                opacity: Config.background.wallpaperEnabled ? 1 : 0

                Behavior on opacity {
                    Anim {
                        type: Anim.SlowEffects
                    }
                }

                Loader {
                    id: wallIndicatorLoader

                    anchors.centerIn: parent

                    opacity: 0
                    active: opacity > 0

                    sourceComponent: StyledRect {
                        implicitWidth: wallLoadingIndicator.implicitSize + Tokens.padding.largeIncreased * 2
                        implicitHeight: wallLoadingIndicator.implicitSize + Tokens.padding.largeIncreased * 2

                        color: Colours.palette.m3primaryContainer
                        radius: Tokens.rounding.full

                        LoadingIndicator {
                            id: wallLoadingIndicator

                            anchors.centerIn: parent
                            containsIcon: true
                            implicitSize: Math.min(wallWrapper.implicitWidth, wallWrapper.implicitHeight) * 0.4
                        }
                    }

                    Behavior on opacity {
                        Anim {
                            type: Anim.DefaultEffects
                        }
                    }
                }

                Timer {
                    id: wallLoadDebounceTimer

                    interval: 100
                    onTriggered: {
                        if (wallImg.status !== Image.Ready)
                            wallIndicatorLoader.opacity = 1;
                    }
                }

                FadeImage {
                    id: wallImg

                    anchors.fill: parent
                    source: Wallpapers.getThumbnailPath(Wallpapers.current)
                    preventInit: wallIndicatorLoader.opacity > 0
                    fadeOutAnim: Anim.DefaultEffects
                    fadeInAnim: Anim.SlowEffects

                    onSourceChanged: wallLoadDebounceTimer.restart()

                    onStatusChanged: {
                        if (status === Image.Ready) {
                            wallLoadDebounceTimer.stop();
                            wallIndicatorLoader.opacity = 0;
                        }
                    }
                }
            }

            IconTextButton {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: Tokens.padding.medium

                icon: "save"
                text: qsTr("Save Recolored")
                font: Tokens.font.body.large
                isRound: true
                shapeMorph: true
                type: IconTextButton.Filled
                horizontalPadding: Tokens.padding.large
                verticalPadding: Tokens.padding.medium

                visible: Config.background.wallpaperEnabled && Config.background.wallpaperRecolor
                opacity: visible ? 1 : 0

                Behavior on opacity {
                    Anim {
                        type: Anim.DefaultEffects
                    }
                }

                onClicked: {
                    const bgWin = ShellState.componentsFor(root.nState.screen).background;
                    if (bgWin && bgWin.wallpaperLoader && bgWin.wallpaperLoader.item && bgWin.wallpaperLoader.item.current) {
                        const path = Paths.home + "/Downloads/recolored_wallpaper.png";
                        CUtils.saveItem(bgWin.wallpaperLoader.item.current, "file://" + path, function() {
                            Notifs.sendToast("Wallpaper Saved", "Saved to ~/Downloads/recolored_wallpaper.png", "image", null, null);
                        });
                    }
                }
            }
        }

        ButtonRow {
            Layout.alignment: Qt.AlignHCenter
            spacing: Tokens.spacing.small

            IconTextButton {
                icon: "wallpaper"
                text: qsTr("Wallpapers")
                font: Tokens.font.body.large
                isRound: true
                shapeMorph: true
                type: IconTextButton.Tonal
                horizontalPadding: Tokens.padding.extraLarge
                verticalPadding: Tokens.padding.medium
                disabled: !Config.background.wallpaperEnabled
                onClicked: root.nState.openSubPage(1)
            }

            IconTextButton {
                icon: "palette"
                text: qsTr("Colours")
                font: Tokens.font.body.large
                isRound: true
                shapeMorph: true
                type: IconTextButton.Tonal
                horizontalPadding: Tokens.padding.extraLarge
                verticalPadding: Tokens.padding.medium
                onClicked: root.nState.openSubPage(3)
            }
        }

        SectionHeader {
            text: qsTr("Wallpaper")
        }

        ToggleRow {
            Layout.fillWidth: true
            first: true
            text: qsTr("Display wallpaper")
            checked: Config.background.wallpaperEnabled
            onToggled: GlobalConfig.background.wallpaperEnabled = checked
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            text: qsTr("Recolor wallpaper")
            subtext: qsTr("Tint the wallpaper to match static color schemes")
            checked: Config.background.wallpaperRecolor
            onToggled: GlobalConfig.background.wallpaperRecolor = checked
            enabled: Config.background.wallpaperEnabled
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            icon: ""
            label: qsTr("Recolor strength")
            valueLabel: Math.round(value * 100) + "%"
            value: Config.background.wallpaperRecolorStrength
            enabled: Config.background.wallpaperRecolor && Config.background.wallpaperEnabled
            onMoved: v => GlobalConfig.background.wallpaperRecolorStrength = v
        }

        SelectRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true

            label: qsTr("Video Hardware Decoder")
            menuOnTop: true
            menuItems: root.hwDecoderItems
            active: root.hwDecoderItems[root.hwDecoderToIndex(WallpaperPauser.hwDecoder)]
            onSelected: item => WallpaperPauser.hwDecoder = root.hwDecoderValues[root.hwDecoderItems.indexOf(item)]
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true

            text: qsTr("Pause animated wallpapers on battery")
            checked: WallpaperPauser.pauseOnBattery
            onToggled: WallpaperPauser.pauseOnBattery = checked
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true

            text: qsTr("Pause animated wallpapers behind windows")
            checked: WallpaperPauser.pauseOnWindowOverlap
            onToggled: WallpaperPauser.pauseOnWindowOverlap = checked
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true

            last: true
            text: qsTr("Animate wallpaper transitions")
            checked: Wallpapers.enableAnimation
            onToggled: Wallpapers.enableAnimation = checked
        }

        SectionHeader {
            text: qsTr("Appearance")
        }

        ToggleRow {
            first: true
            Layout.fillWidth: true
            text: qsTr("Bezel mode (Pitch black)")
            subtext: qsTr("Make the shell pitch black to blend with display bezels")
            checked: Config.appearance.pitchBlack
            onToggled: GlobalConfig.appearance.pitchBlack = checked
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            text: qsTr("Islands")
            subtext: qsTr("Everything appears as its own floating widget (Very Experimental)")
            checked: GlobalConfig.appearance.islands
            onToggled: GlobalConfig.appearance.islands = checked
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            text: qsTr("Transparency")
            checked: Colours.transparency.enabled
            onToggled: {
                GlobalConfig.appearance.transparency.enabled = checked;
                GlobalConfig.utilities.toasts.transparency = checked;
            }
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Base transparency level")
            value: Colours.transparency.base
            valueLabel: Math.round(value * 100) + "%"
            onMoved: v => GlobalConfig.appearance.transparency.base = v
            enabled: Colours.transparency.enabled
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Layer transparency level")
            value: Colours.transparency.layers
            valueLabel: Math.round(value * 100) + "%"
            onMoved: v => GlobalConfig.appearance.transparency.layers = v
            enabled: Colours.transparency.enabled
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            last: true
            text: qsTr("Dark theme")
            checked: !Colours.light
            onToggled: Colours.setMode(checked ? "dark" : "light")
        }
    }
}
