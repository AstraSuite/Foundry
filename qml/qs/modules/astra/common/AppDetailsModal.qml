import QtQuick
import QtQuick.Layouts
import Quickshell
import Quickshell.Widgets
import M3Shapes
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.services
import AstraMarket.Market 1.0

Rectangle {
    id: root

    property var appData: null
    visible: appData !== null

    width: Window.width ? Window.width : 900
    height: Window.height ? Window.height : 700
    x: 0
    y: 0
    color: "#cc0a0f0f"
    z: 9999

    signal closed()

    readonly property var infoMap: (root.appData && root.appData.id) ? PackageManager.getPackageDetails(root.appData.id, root.appData.backend) : ({})
    readonly property var dependsList: infoMap.depends || []
    readonly property var requiredByList: infoMap.requiredBy || []

    property bool dependsExpanded: true
    property bool requiredByExpanded: true
    property string selectedScope: (root.appData && root.appData.scope) ? root.appData.scope : "user"

    function closeModal(): void {
        root.appData = null;
        root.closed();
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.closeModal()
    }

    ConnectedRect {
        width: Math.min(760, root.width - 40)
        height: Math.min(840, root.height - 40)
        anchors.centerIn: parent
        radius: Tokens.rounding.large
        color: Colours.tPalette.m3surfaceContainerLow
        border.color: Colours.palette.m3outline
        border.width: 1

        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: Tokens.padding.large
            contentWidth: flickable.width
            contentHeight: modalColumn.implicitHeight
            clip: true

            ColumnLayout {
                id: modalColumn
                width: flickable.width
                spacing: Tokens.padding.medium

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.small

                    IconButton {
                        icon: "arrow_back"
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: root.closeModal()
                    }

                    StyledText {
                        text: qsTr("App Details")
                        font: Tokens.font.title.medium
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Item { Layout.fillWidth: true }

                    IconButton {
                        icon: "close"
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: root.closeModal()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.medium
                    Layout.topMargin: Tokens.padding.small

                    Item {
                        implicitWidth: 64
                        implicitHeight: 64
                        Layout.alignment: Qt.AlignVCenter

                        MaterialShape {
                            anchors.fill: parent
                            visible: true
                            shape: MaterialShape.Cookie9Sided
                            color: Colours.palette.m3primaryContainer
                        }

                        Image {
                            id: modalIconImg
                            anchors.fill: parent
                            anchors.margins: 8
                            asynchronous: true
                            fillMode: Image.PreserveAspectFit
                            visible: modalIconImg.status === Image.Ready && modalIconImg.source.toString() !== ""
                            source: PackageManager.getIconPath(root.appData ? (root.appData.icon || root.appData.id || "") : "", root.appData ? root.appData.backend : "")
                        }

                        MaterialIcon {
                            anchors.centerIn: parent
                            visible: modalIconImg.status !== Image.Ready || modalIconImg.source.toString() === ""
                            text: (root.appData && root.appData.backend === "Flatpak") ? "deployed_code" : (root.appData ? (root.appData.icon || "package_2") : "package_2")
                            fontStyle: Tokens.font.icon.large
                            color: Colours.palette.m3onPrimaryContainer
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 2

                        StyledText {
                            text: root.appData ? (root.appData.name || root.appData.id) : ""
                            font: Tokens.font.headline.medium
                            color: Colours.palette.m3onSurface
                        }

                        RowLayout {
                            spacing: Tokens.padding.extraSmall

                            StyledText {
                                text: root.appData ? (root.appData.backend + " Official") : ""
                                font: Tokens.font.body.small
                                color: Colours.palette.m3onSurfaceVariant
                            }

                            MaterialIcon {
                                text: "check_circle"
                                fontStyle: Tokens.font.icon.small
                                color: Colours.palette.m3primary
                            }

                            StyledText {
                                text: qsTr("Verified")
                                font: Tokens.font.label.small
                                color: Colours.palette.m3primary
                            }
                        }

                        StyledText {
                            text: "★★★★★  4.9 (1.2k ratings)"
                            font: Tokens.font.label.medium
                            color: "#ffc107"
                        }
                    }

                    SplitButton {
                        id: modalSplitBtn
                        Layout.alignment: Qt.AlignVCenter
                        mainText: (root.appData && root.appData.isInstalled) ? qsTr("Open") : qsTr("Install")
                        mainIcon: (root.appData && root.appData.isInstalled) ? "open_in_new" : "download"
                        isSplit: root.appData && root.appData.backend === "Flatpak" && !root.appData.isInstalled
                        currentScope: root.selectedScope
                        currentSourceLabel: root.selectedScope === "system" ? qsTr("Flathub (System)") : qsTr("Flathub (User)")
                        sourcesList: [
                            { id: "user", label: qsTr("Flathub (User)"), desc: qsTr("Install in user directory (~/.local)") },
                            { id: "system", label: qsTr("Flathub (System)"), desc: qsTr("Install system-wide (/var/lib)") }
                        ]
                        onSourceSelected: function(scopeId, scopeLabel) {
                            root.selectedScope = scopeId;
                        }
                        onMainClicked: {
                            if (!root.appData) return;
                            if (root.appData.isInstalled) {
                                PackageManager.launchApp(root.appData.backend, root.appData.id, root.appData.exec || "");
                            } else {
                                PackageManager.installPackage(root.appData.backend, root.appData.id, root.selectedScope);
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.medium
                    visible: root.appData && (root.appData.backend === "Pacman" || root.appData.backend === "AUR" || root.appData.backend === "AppImage")

                    StyledText {
                        text: qsTr("Information")
                        font: Tokens.font.title.medium
                        color: Colours.palette.m3onSurface
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.small

                        Repeater {
                            model: [
                                { label: "Version", value: root.infoMap.version || (root.appData ? root.appData.version : "") || "N/A" },
                                { label: "Repository", value: root.infoMap.repository || (root.appData ? root.appData.backend.toLowerCase() : "local") },
                                { label: "Installed Size", value: root.infoMap.installedSize || "N/A" },
                                { label: "Build Date", value: root.infoMap.buildDate || "N/A" },
                                { label: "Install Reason", value: root.infoMap.installReason || "Explicitly installed" },
                                { label: "URL", value: root.infoMap.url || "N/A", isUrl: true },
                                { label: "Licenses", value: root.infoMap.licenses || "N/A" },
                                { label: "Provides", value: root.infoMap.provides || "N/A" }
                            ]

                            RowLayout {
                                required property var modelData
                                Layout.fillWidth: true

                                StyledText {
                                    text: modelData.label
                                    font: Tokens.font.body.medium
                                    color: Colours.palette.m3onSurfaceVariant
                                    Layout.preferredWidth: 160
                                }

                                StyledText {
                                    Layout.fillWidth: true
                                    text: modelData.value
                                    font: Tokens.font.body.medium
                                    color: modelData.isUrl ? Colours.palette.m3primary : Colours.palette.m3onSurface

                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: (modelData.isUrl === true) && modelData.value !== "N/A"
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: Qt.openUrlExternally(modelData.value)
                                    }
                                }
                            }
                        }
                    }

                    Item { width: 1; height: Tokens.padding.small }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.small
                        visible: root.dependsList.length > 0

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Tokens.padding.extraSmall

                            MaterialIcon {
                                text: root.dependsExpanded ? "arrow_drop_down" : "arrow_right"
                                fontStyle: Tokens.font.icon.small
                                color: Colours.palette.m3onSurfaceVariant
                            }

                            StyledText {
                                text: qsTr("Depends (%1)").arg(root.dependsList.length)
                                font: Tokens.font.title.small
                                color: Colours.palette.m3onSurface
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Tokens.padding.large
                            spacing: 4
                            visible: root.dependsExpanded

                            Repeater {
                                model: root.dependsList
                                StyledText {
                                    required property string modelData
                                    text: modelData
                                    font: Tokens.font.body.medium
                                    color: Colours.palette.m3primary
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.small
                        visible: root.requiredByList.length > 0

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Tokens.padding.extraSmall

                            MaterialIcon {
                                text: root.requiredByExpanded ? "arrow_drop_down" : "arrow_right"
                                fontStyle: Tokens.font.icon.small
                                color: Colours.palette.m3onSurfaceVariant
                            }

                            StyledText {
                                text: qsTr("Required By (%1)").arg(root.requiredByList.length)
                                font: Tokens.font.title.small
                                color: Colours.palette.m3onSurface
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Tokens.padding.large
                            spacing: 4
                            visible: root.requiredByExpanded

                            Repeater {
                                model: root.requiredByList
                                StyledText {
                                    required property string modelData
                                    text: modelData
                                    font: Tokens.font.body.medium
                                    color: Colours.palette.m3onSurface
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.medium
                    visible: !root.appData || root.appData.backend === "Flatpak"

                    RowLayout {
                        Layout.fillWidth: true
                        implicitHeight: 220
                        spacing: Tokens.padding.small

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 220
                            radius: Tokens.rounding.large
                            color: Colours.tPalette.m3surfaceContainerHigh
                            border.color: Colours.palette.m3outline
                            border.width: 1

                            Image {
                                anchors.fill: parent
                                anchors.margins: 4
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                source: (root.appData && root.appData.icon && root.appData.icon.startsWith("http")) ? root.appData.icon : ""
                                visible: status === Image.Ready && source.toString() !== ""

                                Rectangle {
                                    anchors.fill: parent
                                    radius: Tokens.rounding.large
                                    color: "#44000000"
                                }
                            }

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: Tokens.padding.small

                                MaterialShape {
                                    implicitWidth: 64
                                    implicitHeight: 64
                                    Layout.alignment: Qt.AlignHCenter
                                    shape: MaterialShape.Cookie9Sided
                                    color: Colours.palette.m3primaryContainer

                                    MaterialIcon {
                                        anchors.centerIn: parent
                                        text: "photo_library"
                                        fontStyle: Tokens.font.icon.large
                                        color: Colours.palette.m3onPrimaryContainer
                                    }
                                }

                                StyledText {
                                    text: (root.appData ? (root.appData.name || root.appData.id) : "") + " Preview & Screenshots"
                                    font: Tokens.font.title.medium
                                    color: Colours.palette.m3onSurface
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }

                        Rectangle {
                            implicitWidth: 70
                            implicitHeight: 220
                            radius: Tokens.rounding.large
                            color: Colours.tPalette.m3surfaceContainerLow
                            border.color: Colours.palette.m3outline
                            border.width: 1

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: Tokens.padding.extraSmall

                                MaterialIcon {
                                    text: "chevron_right"
                                    fontStyle: Tokens.font.icon.medium
                                    color: Colours.palette.m3primary
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                    }

                    Item { width: 1; height: Tokens.padding.small }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.small

                        StyledText {
                            text: root.appData ? (root.appData.summary || root.appData.name || "") : ""
                            font: Tokens.font.title.large
                            color: Colours.palette.m3onSurface
                        }

                        StyledText {
                            Layout.fillWidth: true
                            text: root.appData ? (root.appData.description || root.appData.summary || qsTr("An essential desktop application for your Linux system.")) : ""
                            font: Tokens.font.body.medium
                            color: Colours.palette.m3onSurfaceVariant
                            wrapMode: Text.WordWrap
                        }
                    }

                    Item { width: 1; height: Tokens.padding.small }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.small

                        Repeater {
                            model: [
                                { value: "235.5 MB", label: "Download Size", icon: "download" },
                                { value: "Verified Safe", label: "Security Status", icon: "verified_user" },
                                { value: "Desktop", label: "Platform", icon: "desktop_windows" },
                                { value: "Everyone", label: "Content Rating", icon: "grade" }
                            ]

                            Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                implicitHeight: 70
                                radius: Tokens.rounding.medium
                                color: Colours.tPalette.m3surfaceContainerLow
                                border.color: Colours.palette.m3outline
                                border.width: 1

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 2

                                    MaterialIcon {
                                        text: modelData.icon
                                        fontStyle: Tokens.font.icon.small
                                        color: Colours.palette.m3primary
                                        Layout.alignment: Qt.AlignHCenter
                                    }

                                    StyledText {
                                        text: modelData.value
                                        font: Tokens.font.label.large
                                        color: Colours.palette.m3onSurface
                                        Layout.alignment: Qt.AlignHCenter
                                    }

                                    StyledText {
                                        text: modelData.label
                                        font: Tokens.font.label.small
                                        color: Colours.palette.m3onSurfaceVariant
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                }
                            }
                        }
                    }

                    Item { width: 1; height: Tokens.padding.small }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.small

                        RowLayout {
                            Layout.fillWidth: true

                            StyledText {
                                text: qsTr("Version ") + (root.appData ? (root.appData.version || "1.0") : "1.0")
                                font: Tokens.font.title.small
                                color: Colours.palette.m3onSurface
                            }

                            Item { Layout.fillWidth: true }

                            StyledText {
                                text: qsTr("Updated recently")
                                font: Tokens.font.body.small
                                color: Colours.palette.m3onSurfaceVariant
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Tokens.padding.medium

                            RowLayout {
                                spacing: Tokens.padding.extraSmall
                                MaterialIcon {
                                    text: "language"
                                    fontStyle: Tokens.font.icon.small
                                    color: Colours.palette.m3primary
                                }
                                StyledText {
                                    text: qsTr("Project Website")
                                    font: Tokens.font.label.large
                                    color: Colours.palette.m3primary
                                }
                            }

                            RowLayout {
                                spacing: Tokens.padding.extraSmall
                                MaterialIcon {
                                    text: "bug_report"
                                    fontStyle: Tokens.font.icon.small
                                    color: Colours.palette.m3primary
                                }
                                StyledText {
                                    text: qsTr("Report Issue")
                                    font: Tokens.font.label.large
                                    color: Colours.palette.m3primary
                                }
                            }

                            RowLayout {
                                spacing: Tokens.padding.extraSmall
                                MaterialIcon {
                                    text: "help_outline"
                                    fontStyle: Tokens.font.icon.small
                                    color: Colours.palette.m3primary
                                }
                                StyledText {
                                    text: qsTr("Help & Support")
                                    font: Tokens.font.label.large
                                    color: Colours.palette.m3primary
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
