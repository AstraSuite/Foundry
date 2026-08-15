import QtQuick
import QtQuick.Layouts
import M3Shapes
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.components.effects
import qs.services
import qs.modules.astra.common
import AstraMarket.Market 1.0

PageBase {
    id: root

    title: qsTr("Updates")

    property var updatesList: []
    property var selectedApp: null
    property bool isLoading: false

    function checkUpdates(): void {
        root.isLoading = true;
        PackageManager.checkForUpdatesAsync();
    }

    Component.onCompleted: {
        checkUpdates();
    }

    data: [
        Connections {
            target: PackageManager
            function onUpdatesCompleted(updates): void {
                root.updatesList = updates ? updates : [];
                root.isLoading = false;
            }
            function onOperationFinished(success, message): void {
                root.checkUpdates();
            }
        }
    ]

    Column {
        width: root.width
        spacing: Tokens.spacing.extraSmall / 2

        RowLayout {
            width: parent.width
            spacing: Tokens.spacing.extraSmall

            StyledText {
                text: root.updatesList.length > 0 ? qsTr("%1 Updates Available").arg(root.updatesList.length) : qsTr("System is up to date")
                font: Tokens.font.body.medium
                color: Colours.palette.m3onSurfaceVariant
                Layout.alignment: Qt.AlignVCenter
            }

            IconButton {
                icon: "refresh"
                type: IconButton.Text
                padding: 2
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.checkUpdates()
            }

            Item { Layout.fillWidth: true }

            IconButton {
                visible: root.updatesList.length > 0
                icon: "system_update_alt"
                type: IconButton.Filled
                enabled: !PackageManager.isBusy
                onClicked: {
                    PackageManager.updateAllPackages();
                }
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        Item {
            width: parent.width
            implicitHeight: Math.max(350, root.height - 180)
            visible: root.isLoading || (PackageManager.isBusy && root.updatesList.length === 0)

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.medium

                LoadingIndicator {
                    implicitSize: 52
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    text: qsTr("Checking for updates...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        Repeater {
            id: updatesRepeater
            model: root.updatesList

            ConnectedRect {
                required property var modelData
                required property int index
                width: parent ? parent.width : root.width
                first: index === 0
                last: index === updatesRepeater.count - 1
                implicitHeight: cardLayout.implicitHeight + Tokens.padding.medium * 2

                StateLayer {
                    id: cardStateLayer
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.nState.selectedApp = modelData;
                        root.nState.openSubPage(1);
                    }
                }

                RowLayout {
                    id: cardLayout
                    anchors.fill: parent
                    anchors.margins: Tokens.padding.medium
                    spacing: Tokens.padding.medium

                    Item {
                        implicitWidth: 40
                        implicitHeight: 40
                        Layout.alignment: Qt.AlignVCenter

                        Item {
                            id: updateCookieShapeWrapper
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
                            source: PackageManager.getIconPath(modelData.icon || modelData.id || "", modelData.backend || "")

                            layer.enabled: true
                            layer.smooth: true
                            layer.samples: 4
                            layer.effect: Mask {
                                maskSource: updateCookieShapeWrapper
                            }
                        }

                        MaterialIcon {
                            anchors.centerIn: parent
                            visible: appIconImg.status !== Image.Ready || appIconImg.source.toString() === ""
                            text: modelData.backend === "Flatpak" ? "deployed_code" : (modelData.backend === "AppImage" ? "extension" : "update")
                            fontStyle: Tokens.font.icon.medium
                            color: Colours.palette.m3onPrimaryContainer
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: Tokens.padding.extraSmall

                        RowLayout {
                            spacing: Tokens.padding.small
                            StyledText {
                                text: modelData.name || modelData.id
                                font: Tokens.font.title.medium
                                color: Colours.palette.m3onSurface
                            }

                            Rectangle {
                                implicitWidth: badgeText.implicitWidth + Tokens.padding.small * 2
                                implicitHeight: badgeText.implicitHeight + Tokens.padding.extraSmall
                                radius: Tokens.rounding.small
                                color: Colours.palette.m3secondaryContainer

                                StyledText {
                                    id: badgeText
                                    anchors.centerIn: parent
                                    text: modelData.backend
                                    font: Tokens.font.label.small
                                    color: Colours.palette.m3onSecondaryContainer
                                }
                            }
                        }

                        StyledText {
                            text: qsTr("Update available")
                            font: Tokens.font.body.small
                            color: Colours.palette.m3onSurfaceVariant
                        }
                    }

                    MaterialIcon {
                        text: "chevron_right"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3onSurfaceVariant
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }
        }
    }
}
