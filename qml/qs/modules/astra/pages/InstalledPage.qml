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

    title: qsTr("Installed apps")

    property var installedList: []
    property var selectedApp: null
    property int visibleCount: 20
    property bool isLoading: false

    function refreshList(): void {
        root.visibleCount = 20;
        root.isLoading = true;
        PackageManager.getInstalledPackagesAsync();
    }

    Component.onCompleted: {
        refreshList();
    }

    Item {
        Connections {
            target: root.flickable
            function onContentYChanged(): void {
                if (root.flickable && root.flickable.contentY + root.flickable.height >= root.flickable.contentHeight - 400) {
                    if (!root.isLoading && root.visibleCount < root.installedList.length) {
                        root.visibleCount = Math.min(root.visibleCount + 15, root.installedList.length);
                    }
                }
            }
        }
    }

    data: [
        Connections {
            target: root.nState

            function onRefreshRequested(): void {
                root.refreshList();
            }
        },
        Connections {
            target: PackageManager
            function onInstalledCompleted(results): void {
                root.installedList = results ? results : [];
                root.isLoading = false;
            }
            function onOperationFinished(success, message): void {
                root.refreshList();
            }
        }
    ]

    Column {
        width: root.width
        spacing: Tokens.spacing.extraSmall / 2

        RowLayout {
            width: root.width
            spacing: Tokens.spacing.extraSmall

            StyledText {
                text: qsTr("%1 Installed Applications").arg(root.installedList.length)
                font: Tokens.font.body.medium
                color: Colours.palette.m3onSurfaceVariant
                Layout.alignment: Qt.AlignVCenter
            }

            IconButton {
                icon: "refresh"
                type: IconButton.Text
                padding: 2
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.refreshList()
            }

            Item { Layout.fillWidth: true }

            IconTextButton {
                icon: "terminal"
                text: qsTr("Logs")
                type: ButtonBase.Tonal
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.nState.openSubPage(2)
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        Item {
            width: parent.width
            implicitHeight: Math.max(350, root.height - 180)
            visible: root.isLoading || (PackageManager.isBusy && root.installedList.length === 0)

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.medium

                LoadingIndicator {
                    implicitSize: 52
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    text: qsTr("Loading installed applications...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        Repeater {
            id: installedRepeater
            model: root.isLoading ? [] : root.installedList.slice(0, root.visibleCount)

            ConnectedRect {
                required property var modelData
                required property int index
                width: root.width
                first: index === 0
                last: index === installedRepeater.count - 1
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
                            source: PackageManager.getIconPath(modelData.icon || modelData.id || "", modelData.backend || "")

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
                            text: modelData.backend === "Flatpak" ? "deployed_code" : (modelData.backend === "AppImage" ? "extension" : "package_2")
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
                            text: (modelData.backend === "Pacman" || modelData.backend === "AUR") ? (((modelData.repository ? modelData.repository : (modelData.backend === "AUR" ? "local" : "extra")) + "/" + (modelData.id || modelData.name)) + " • " + (modelData.version || "installed")) : (modelData.summary || modelData.id)
                            font: Tokens.font.body.small
                            color: Colours.palette.m3onSurfaceVariant
                            elide: Text.ElideRight
                            Layout.fillWidth: true
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

        ColumnLayout {
            width: parent.width
            visible: !root.isLoading && root.visibleCount < root.installedList.length
            spacing: Tokens.padding.small

            Item { implicitHeight: Tokens.padding.small }

            LoadingIndicator {
                implicitSize: 36
                color: Colours.palette.m3primary
                Layout.alignment: Qt.AlignHCenter
            }

            StyledText {
                text: qsTr("Loading more installed applications...")
                font: Tokens.font.label.medium
                color: Colours.palette.m3onSurfaceVariant
                Layout.alignment: Qt.AlignHCenter
            }

            Item { implicitHeight: Tokens.padding.medium }
        }
    }
}
