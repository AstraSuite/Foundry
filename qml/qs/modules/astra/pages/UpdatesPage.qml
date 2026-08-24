pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Foundry.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.services
import qs.modules.astra.common
import Foundry.Market 1.0

PageBase {
    id: root

    title: qsTr("Updates")

    property var updatesList: []
    property var history: []
    property var selectedApp: null
    property bool isLoading: false
    property int visibleCount: 25

    function checkUpdates(): void {
        root.isLoading = true;
        root.visibleCount = 25;
        PackageManager.checkForUpdatesAsync();
    }

    function reloadHistory(): void {
        root.history = PackageManager.recentOperations(8);
    }

    Component.onCompleted: {
        checkUpdates();
        reloadHistory();
    }

    Item {
        Connections {
            target: root.flickable
            function onContentYChanged(): void {
                if (root.flickable && root.flickable.contentY + root.flickable.height >= root.flickable.contentHeight - 400) {
                    if (!root.isLoading && root.visibleCount < root.updatesList.length) {
                        root.visibleCount = Math.min(root.visibleCount + 20, root.updatesList.length);
                    }
                }
            }
        }
    }

    data: [
        Connections {
            target: root.nState

            function onRefreshRequested(): void {
                root.checkUpdates();
            }
        },
        Connections {
            target: PackageManager
            function onUpdatesCompleted(updates): void {
                root.updatesList = updates ? updates : [];
                root.isLoading = false;
            }
            function onOperationFinished(success, message): void {
                root.checkUpdates();
            }
            function onHistoryChanged(): void {
                root.reloadHistory();
            }
        }
    ]

    Column {
        width: root ? root.width : 0
        spacing: Tokens.spacing.extraSmall / 2

        RowLayout {
            width: root ? root.width : 0
            spacing: Tokens.spacing.extraSmall

            StyledText {
                text: root.updatesList.length > 0
                    ? qsTr("%1 Updates Available").arg(root.updatesList.length)
                    : qsTr("System is up to date")
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

            IconTextButton {
                icon: "terminal"
                text: qsTr("Logs")
                type: ButtonBase.Tonal
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.nState.openSubPage(2)
            }

            IconTextButton {
                visible: root.updatesList.length > 0
                icon: "system_update_alt"
                text: qsTr("Update All")
                type: ButtonBase.Filled
                enabled: !PackageManager.isOperationRunning
                Layout.alignment: Qt.AlignVCenter
                onClicked: {
                    PackageManager.updateAllPackages();
                }
            }
        }

        Repeater {
            model: PackageManager.missingBackendTools

            StyledRect {
                id: hintCard

                required property string modelData

                width: root ? root.width : 0
                implicitHeight: hintLayout.implicitHeight + Tokens.padding.medium * 2
                radius: Tokens.rounding.large
                color: Colours.palette.m3secondaryContainer

                RowLayout {
                    id: hintLayout

                    anchors.fill: parent
                    anchors.margins: Tokens.padding.medium
                    spacing: Tokens.padding.small

                    MaterialIcon {
                        text: "info"
                        fontStyle: Tokens.font.icon.small
                        color: Colours.palette.m3onSecondaryContainer
                    }

                    StyledText {
                        Layout.fillWidth: true
                        text: hintCard.modelData
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSecondaryContainer
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        Item {
            width: root ? root.width : 0
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
                    text: PackageManager.isBusy
                        ? (PackageManager.statusMessage || qsTr("Applying updates..."))
                        : qsTr("Checking for updates...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        Repeater {
            id: updatesRepeater
            model: root.isLoading ? [] : root.updatesList.slice(0, root.visibleCount)

            ConnectedRect {
                id: cardRect
                required property var modelData
                required property int index
                width: root ? root.width : 0
                first: index === 0
                last: index === updatesRepeater.count - 1
                implicitHeight: cardLayout.implicitHeight + Tokens.padding.medium * 2

                StateLayer {
                    id: cardStateLayer
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.nState.selectedApp = cardRect.modelData;
                        root.nState.openSubPage(1);
                    }
                }

                RowLayout {
                    id: cardLayout
                    anchors.fill: parent
                    anchors.margins: Tokens.padding.medium
                    spacing: Tokens.padding.medium

                    StyledRect {
                        width: 44
                        height: 44
                        radius: Tokens.rounding.medium
                        color: Colours.palette.m3surfaceContainerHigh
                        clip: true
                        Layout.alignment: Qt.AlignVCenter

                        Image {
                            id: appIconImg
                            anchors.fill: parent
                            anchors.margins: 4
                            asynchronous: true
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                            visible: appIconImg.status === Image.Ready && appIconImg.source.toString() !== ""
                            source: PackageManager.getIconPath(cardRect.modelData.icon || cardRect.modelData.id || "", cardRect.modelData.backend || "")
                        }

                        MaterialIcon {
                            anchors.centerIn: parent
                            visible: appIconImg.status !== Image.Ready || appIconImg.source.toString() === ""
                            text: cardRect.modelData.backend === "Flatpak" ? "deployed_code" : (cardRect.modelData.backend === "AppImage" ? "extension" : "package_2")
                            fontStyle: Tokens.font.icon.medium
                            color: Colours.palette.m3onPrimaryContainer
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        spacing: Tokens.padding.extraSmall

                        RowLayout {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            spacing: Tokens.padding.small

                            StyledText {
                                Layout.fillWidth: true
                                text: cardRect.modelData.name || cardRect.modelData.id
                                font: Tokens.font.title.medium
                                color: Colours.palette.m3onSurface
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                implicitWidth: badgeText.implicitWidth + Tokens.padding.small * 2
                                implicitHeight: badgeText.implicitHeight + Tokens.padding.extraSmall
                                radius: Tokens.rounding.small
                                color: Colours.palette.m3secondaryContainer

                                StyledText {
                                    id: badgeText
                                    anchors.centerIn: parent
                                    text: cardRect.modelData.backend
                                    font: Tokens.font.label.small
                                    color: Colours.palette.m3onSecondaryContainer
                                }
                            }
                        }

                        StyledText {
                            text: cardRect.modelData.version ? (qsTr("Update to ") + cardRect.modelData.version) : qsTr("Update available")
                            font: Tokens.font.body.small
                            color: Colours.palette.m3onSurfaceVariant
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    IconTextButton {
                        icon: "system_update_alt"
                        text: qsTr("Update")
                        type: ButtonBase.Tonal
                        enabled: !PackageManager.isOperationRunning
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        onClicked: {
                            PackageManager.updatePackage(cardRect.modelData.backend, cardRect.modelData.id, cardRect.modelData.scope || "");
                        }
                    }

                    MaterialIcon {
                        text: "chevron_right"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3onSurfaceVariant
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                }
            }
        }

        Item {
            width: 1
            height: Tokens.padding.large
            visible: root.history.length > 0
        }

        StyledText {
            text: qsTr("Recent activity")
            font: Tokens.font.label.medium
            color: Colours.palette.m3onSurfaceVariant
            visible: root.history.length > 0
            leftPadding: Tokens.padding.small
            bottomPadding: Tokens.spacing.extraSmall
        }

        Repeater {
            id: historyRepeater

            model: root.history

            ConnectedRect {
                id: historyRect

                required property var modelData
                required property int index

                width: root ? root.width : 0
                first: index === 0
                last: index === historyRepeater.count - 1
                implicitHeight: historyLayout.implicitHeight + Tokens.padding.medium * 2

                RowLayout {
                    id: historyLayout

                    anchors.fill: parent
                    anchors.margins: Tokens.padding.medium
                    spacing: Tokens.padding.medium

                    MaterialIcon {
                        text: historyRect.modelData.success ? "check_circle" : "error"
                        fontStyle: Tokens.font.icon.medium
                        color: historyRect.modelData.success ? Colours.palette.m3primary : Colours.palette.m3error
                        Layout.alignment: Qt.AlignVCenter
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        StyledText {
                            Layout.fillWidth: true
                            text: {
                                const name = historyRect.modelData.package || "";
                                switch (historyRect.modelData.action) {
                                case "install":
                                    return historyRect.modelData.success ? qsTr("Installed %1").arg(name) : qsTr("Failed to install %1").arg(name);
                                case "update":
                                    return historyRect.modelData.success ? qsTr("Updated %1").arg(name) : qsTr("Failed to update %1").arg(name);
                                case "remove":
                                    return historyRect.modelData.success ? qsTr("Removed %1").arg(name) : qsTr("Failed to remove %1").arg(name);
                                default:
                                    return historyRect.modelData.success ? qsTr("System update completed") : qsTr("System update failed");
                                }
                            }
                            font: Tokens.font.body.medium
                            color: Colours.palette.m3onSurface
                            elide: Text.ElideRight
                        }

                        StyledText {
                            Layout.fillWidth: true
                            text: {
                                const stamp = new Date(historyRect.modelData.time).toLocaleString(Qt.locale(), Locale.ShortFormat);
                                const backend = historyRect.modelData.backend || "";
                                return backend.length > 0 ? backend + " - " + stamp : stamp;
                            }
                            font: Tokens.font.label.small
                            color: Colours.palette.m3onSurfaceVariant
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
