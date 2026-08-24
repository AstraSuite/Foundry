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

    title: qsTr("Installation logs")
    isSubPage: true

    ColumnLayout {
        width: root ? root.width : 0
        spacing: Tokens.padding.medium

        // Status banner
        StyledRect {
            Layout.fillWidth: true
            implicitHeight: bannerRow.implicitHeight + Tokens.padding.medium * 2
            radius: Tokens.rounding.large
            color: PackageManager.isOperationRunning ? Colours.palette.m3primaryContainer : Colours.palette.m3surfaceContainer

            RowLayout {
                id: bannerRow
                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                spacing: Tokens.padding.medium

                MaterialIcon {
                    visible: !PackageManager.isOperationRunning
                    text: "check_circle"
                    fontStyle: Tokens.font.icon.large
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignVCenter
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    StyledText {
                        text: PackageManager.isOperationRunning
                            ? (PackageManager.statusMessage || qsTr("Performing package operation..."))
                            : qsTr("System idle / Last operation completed")
                        font: Tokens.font.title.medium
                        color: PackageManager.isOperationRunning ? Colours.palette.m3onPrimaryContainer : Colours.palette.m3onSurface
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    StyledText {
                        text: PackageManager.isOperationRunning
                            ? qsTr("Progress: %1%").arg(PackageManager.currentProgress)
                            : qsTr("Total logged entries: %1").arg(PackageManager.logLines.length)
                        font: Tokens.font.body.small
                        color: PackageManager.isOperationRunning ? Colours.palette.m3onPrimaryContainer : Colours.palette.m3onSurfaceVariant
                    }
                }

                CircularIndicator {
                    visible: PackageManager.isOperationRunning
                    running: PackageManager.isOperationRunning
                    implicitSize: 32
                    fgColour: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignVCenter
                }

                IconTextButton {
                    visible: PackageManager.isCancellable
                    icon: "cancel"
                    text: qsTr("Cancel")
                    type: ButtonBase.Tonal
                    Layout.alignment: Qt.AlignVCenter
                    onClicked: PackageManager.cancelCurrentOperation()
                }

                IconButton {
                    icon: "delete_sweep"
                    type: IconButton.Tonal
                    onClicked: PackageManager.clearLogs()
                }
            }
        }

        // Clean Material 3 Container for Logs
        StyledRect {
            Layout.fillWidth: true
            implicitHeight: Math.max(400, root.height - 180)
            radius: Tokens.rounding.large
            color: Colours.palette.m3surfaceContainerLow
            border.color: Colours.palette.m3outlineVariant
            border.width: 1

            ListView {
                id: logListView
                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                clip: true
                model: PackageManager.logLines
                spacing: 4

                onCountChanged: {
                    Qt.callLater(() => {
                        logListView.positionViewAtEnd();
                    });
                }

                delegate: RowLayout {
                    required property string modelData
                    required property int index
                    width: logListView.width
                    spacing: Tokens.padding.medium

                    Text {
                        text: String(index + 1)
                        font.family: "Monospace, DejaVu Sans Mono, Courier New"
                        font.pixelSize: 12
                        color: Colours.palette.m3onSurfaceVariant
                        opacity: 0.5
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 36
                        Layout.alignment: Qt.AlignTop
                    }

                    Text {
                        text: modelData
                        font.family: "Monospace, DejaVu Sans Mono, Courier New"
                        font.pixelSize: 12
                        color: modelData.includes("FAILED") || modelData.includes("Error") || modelData.includes("error")
                            ? Colours.palette.m3error
                            : (modelData.includes("SUCCESS") || modelData.includes("Successfully")
                                ? Colours.palette.m3primary
                                : Colours.palette.m3onSurface)
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }

                Item {
                    anchors.centerIn: parent
                    visible: logListView.count === 0

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: Tokens.padding.extraSmall

                        MaterialIcon {
                            Layout.alignment: Qt.AlignHCenter
                            text: "receipt_long"
                            fontStyle: Tokens.font.icon.large
                            color: Colours.palette.m3onSurfaceVariant
                        }

                        StyledText {
                            Layout.alignment: Qt.AlignHCenter
                            text: qsTr("No logs yet. Output from installs, removals, and updates will appear here.")
                            font: Tokens.font.body.small
                            color: Colours.palette.m3onSurfaceVariant
                        }
                    }
                }
            }
        }
    }
}
