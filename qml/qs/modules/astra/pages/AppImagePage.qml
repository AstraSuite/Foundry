import QtQuick
import QtQuick.Layouts
import M3Shapes
import Foundry.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.components.effects
import qs.services
import qs.modules.astra.common
import Foundry.Market 1.0

PageBase {
    id: root

    title: qsTr("AppImage installer")

    property var appImageList: []

    function reloadAppImages(): void {
        appImageList = AppImageInstaller.listInstalledAppImages();
    }

    Component.onCompleted: {
        reloadAppImages();
    }

    Column {
        width: root.width
        spacing: Tokens.spacing.extraSmall / 2

        Connections {
            target: AppImageInstaller
            function onAppImageInstalled(appName, desktopPath): void {
                root.reloadAppImages();
            }
        }

        DropArea {
            id: dropArea
            width: parent.width
            implicitHeight: 180
            keys: ["text/uri-list"]

            onDropped: drop => {
                if (drop.hasUrls) {
                    for (var i = 0; i < drop.urls.length; i++) {
                        var url = drop.urls[i].toString();
                        if (url.toLowerCase().endsWith(".appimage")) {
                            AppImageInstaller.installAppImage(url);
                        }
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                radius: Tokens.rounding.large
                color: dropArea.containsDrag ? Colours.palette.m3primaryContainer : Colours.tPalette.m3surfaceContainer
                border.color: dropArea.containsDrag ? Colours.palette.m3primary : Colours.palette.m3outline
                border.width: dropArea.containsDrag ? 3 : 2
                scale: dropArea.containsDrag ? 1.02 : 1.0

                Behavior on scale {
                    Anim { type: Anim.FastSpatial }
                }

                Behavior on color {
                    CAnim {}
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: Tokens.padding.small

                    MaterialIcon {
                        Layout.alignment: Qt.AlignHCenter
                        text: AppImageInstaller.isInstalling ? "sync" : (dropArea.containsDrag ? "download" : "cloud_upload")
                        fontStyle: Tokens.font.icon.extraLarge
                        color: dropArea.containsDrag ? Colours.palette.m3primary : Colours.palette.m3onSurfaceVariant
                        scale: dropArea.containsDrag ? 1.3 : 1.0

                        Behavior on scale {
                            Anim { type: Anim.FastSpatial }
                        }
                    }

                    StyledText {
                        Layout.alignment: Qt.AlignHCenter
                        text: AppImageInstaller.isInstalling ? AppImageInstaller.statusMessage : (dropArea.containsDrag ? qsTr("Drop .AppImage File to Install!") : qsTr("Drag & Drop .AppImage file here to Install"))
                        font: dropArea.containsDrag ? Tokens.font.title.large : Tokens.font.title.medium
                        color: dropArea.containsDrag ? Colours.palette.m3primary : Colours.palette.m3onSurface
                    }

                    StyledText {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Extracts icon, sets executable permissions, & registers .desktop in ~/.local/share/applications")
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                    }
                }
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        StyledText {
            text: qsTr("Installed AppImages (%1)").arg(root.appImageList.length)
            font: Tokens.font.title.medium
            color: Colours.palette.m3onSurface
        }

        Item { width: 1; height: Tokens.padding.extraSmall }

        Repeater {
            id: appImageRepeater
            model: root.appImageList

            ConnectedRect {
                required property var modelData
                required property int index
                width: parent ? parent.width : root.width
                first: index === 0
                last: index === appImageRepeater.count - 1
                implicitHeight: cardRow.implicitHeight + Tokens.padding.medium * 2

                RowLayout {
                    id: cardRow
                    anchors.fill: parent
                    anchors.margins: Tokens.padding.medium
                    spacing: Tokens.padding.medium

                    Item {
                        width: 44
                        height: 44
                        Layout.alignment: Qt.AlignVCenter

                        Item {
                            id: cookieShapeWrapper
                            anchors.fill: parent
                            layer.enabled: true
                            layer.smooth: true
                            layer.samples: 4

                            MaterialShape {
                                anchors.fill: parent
                                shape: MaterialShape.Cookie9Sided
                                color: Colours.palette.m3primaryContainer
                                antialiasing: true
                                smooth: true
                            }
                        }

                        Image {
                            id: appIconImg
                            anchors.fill: parent
                            asynchronous: true
                            fillMode: Image.PreserveAspectCrop
                            smooth: true
                            mipmap: true
                            antialiasing: true
                            visible: appIconImg.status === Image.Ready && appIconImg.source.toString() !== ""
                            source: PackageManager.getIconPath(modelData.icon || modelData.name || "", "AppImage")

                            layer.enabled: true
                            layer.smooth: true
                            layer.samples: 4
                            layer.effect: Mask {
                                maskSource: cookieShapeWrapper
                            }
                        }

                        MaterialIcon {
                            anchors.centerIn: parent
                            visible: appIconImg.status !== Image.Ready || appIconImg.source.toString() === ""
                            text: "extension"
                            fontStyle: Tokens.font.icon.medium
                            color: Colours.palette.m3onPrimaryContainer
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: Tokens.padding.extraSmall

                        StyledText {
                            text: modelData.name
                            font: Tokens.font.title.medium
                            color: Colours.palette.m3onSurface
                        }

                        StyledText {
                            text: modelData.exec || modelData.desktopPath
                            font: Tokens.font.body.small
                            color: Colours.palette.m3onSurfaceVariant
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    IconButton {
                        Layout.alignment: Qt.AlignVCenter
                        icon: "delete"
                        type: IconButton.Text
                        inactiveOnColour: Colours.palette.m3error
                        activeOnColour: Colours.palette.m3error
                        onClicked: {
                            AppImageInstaller.uninstallAppImage(modelData.desktopPath || modelData.path || modelData.name);
                            root.reloadAppImages();
                        }
                    }

                    IconButton {
                        Layout.alignment: Qt.AlignVCenter
                        icon: "open_in_new"
                        type: IconButton.Tonal
                        onClicked: {
                            AppImageInstaller.launchAppImage(modelData.desktopPath || modelData.path || modelData.name);
                        }
                    }
                }
            }
        }
    }
}
