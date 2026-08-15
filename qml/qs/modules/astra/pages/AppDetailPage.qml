import QtQuick
import QtQuick.Layouts
import Quickshell
import Quickshell.Widgets
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

    title: (nState && nState.selectedApp && (nState.selectedApp.name || nState.selectedApp.id)) ? (nState.selectedApp.name || nState.selectedApp.id) : qsTr("App Details")
    isSubPage: true

    readonly property var appData: nState ? nState.selectedApp : null
    readonly property bool isInstalled: root.appData ? (root.appData.isInstalled === true || root.appData.installed === true) : false
    property var infoMap: null
    readonly property var screenshotsList: (root.infoMap && root.infoMap.screenshots) ? root.infoMap.screenshots : []
    readonly property var dependsList: (root.infoMap && root.infoMap.depends) ? root.infoMap.depends : []
    readonly property var requiredByList: (root.infoMap && root.infoMap.requiredBy) ? root.infoMap.requiredBy : []

    property bool dependsExpanded: true
    property bool requiredByExpanded: true

    property bool descExpanded: false
    property bool isLoadingDetails: true
    property string selectedScope: (root.appData && root.appData.scope) ? root.appData.scope : "user"

    function loadDetails(): void {
        if (root.appData && root.appData.id) {
            root.isLoadingDetails = true;
            PackageManager.fetchPackageDetailsAsync(root.appData.id, root.appData.backend || "");
        } else {
            root.isLoadingDetails = false;
        }
    }

    onAppDataChanged: {
        loadDetails();
    }

    Component.onCompleted: {
        loadDetails();
    }

    Column {
        id: pageContent
        width: root.width
        spacing: Tokens.padding.medium

        Connections {
            target: PackageManager
            function onPackageDetailsReady(details): void {
                root.infoMap = details;
                root.isLoadingDetails = false;
            }
        }

        Item {
            width: parent.width
            implicitHeight: Math.max(400, root.height - 100)
            visible: root.isLoadingDetails

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.medium

                LoadingIndicator {
                    implicitSize: 56
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    text: qsTr("Loading package details...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        ColumnLayout {
            width: parent.width
            spacing: Tokens.padding.medium
            visible: !root.isLoadingDetails

        RowLayout {
            Layout.fillWidth: true
            spacing: Tokens.padding.medium

            Item {
                implicitWidth: 64
                implicitHeight: 64

                Item {
                    id: detailCookieShape
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
                    id: pageIconImg
                    anchors.fill: parent
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    mipmap: true
                    antialiasing: true
                    visible: pageIconImg.status === Image.Ready && pageIconImg.source.toString() !== ""
                    source: (root.infoMap && root.infoMap.iconUrl) ? root.infoMap.iconUrl : (PackageManager.getIconPath(root.appData ? (root.appData.icon || root.appData.id || "") : "", root.appData ? root.appData.backend : ""))

                    layer.enabled: true
                    layer.smooth: true
                    layer.samples: 4
                    layer.effect: Mask {
                        maskSource: detailCookieShape
                    }
                }

                MaterialIcon {
                    anchors.centerIn: parent
                    visible: pageIconImg.status !== Image.Ready || pageIconImg.source.toString() === ""
                    text: (root.appData && root.appData.backend === "Flatpak") ? "deployed_code" : ((root.appData && root.appData.backend === "AppImage") ? "extension" : "package_2")
                    fontStyle: Tokens.font.icon.large
                    color: Colours.palette.m3onPrimaryContainer
                }
            }

            ColumnLayout {
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
                        text: (root.appData && root.appData.backend) ? root.appData.backend : ""
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                    }

                    StyledText {
                        text: "•"
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                        visible: root.appData && root.appData.backend === "Flatpak"
                    }

                    StyledText {
                        text: qsTr("Official")
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                        visible: root.appData && root.appData.backend === "Flatpak"
                    }

                    MaterialIcon {
                        text: "verified"
                        fontStyle: Tokens.font.icon.small
                        color: Colours.palette.m3primary
                        visible: root.appData && root.appData.backend === "Flatpak"
                    }
                }

                RowLayout {
                    spacing: Tokens.padding.extraSmall
                    visible: root.appData && root.appData.backend === "Flatpak"

                    StyledText {
                        text: "★★★★★"
                        font: Tokens.font.body.small
                        color: "#f59e0b"
                    }

                    StyledText {
                        text: "4.9 (1.2k ratings)"
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                    }
                }
            }

            Item { Layout.fillWidth: true }

            RowLayout {
                spacing: Tokens.padding.small
                Layout.alignment: Qt.AlignVCenter

                CircularIndicator {
                    visible: PackageManager.isBusy
                    running: PackageManager.isBusy
                    implicitSize: 32
                    fgColour: Colours.palette.m3primary
                }

                IconButton {
                    visible: root.isInstalled && !PackageManager.isBusy
                    type: ButtonBase.Text
                    icon: "delete"
                    inactiveOnColour: Colours.palette.m3error
                    activeOnColour: Colours.palette.m3error
                    onClicked: {
                        if (root.appData) {
                            PackageManager.uninstallPackage(root.appData.backend || "", root.appData.id || "");
                        }
                    }
                }

                IconTextButton {
                    visible: root.isInstalled && !PackageManager.isBusy
                    icon: "open_in_new"
                    text: qsTr("Open")
                    onClicked: {
                        if (root.appData) {
                            PackageManager.launchApp(root.appData.backend || "", root.appData.id || "");
                        }
                    }
                }

                SplitButton {
                    id: installSplitBtn
                    visible: !root.isInstalled && !PackageManager.isBusy
                    mainText: qsTr("Install")
                    mainIcon: "download"
                    isSplit: root.appData && root.appData.backend === "Flatpak"
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
                        if (root.appData) {
                            PackageManager.installPackage(root.appData.backend || "", root.appData.id || "", root.selectedScope);
                        }
                    }
                }
            }
        }

        Item { implicitHeight: Tokens.padding.small }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Tokens.padding.small
            visible: root.screenshotsList.length > 0

            StyledText {
                text: qsTr("Previews")
                font: Tokens.font.title.large
                color: Colours.palette.m3onSurface
            }

            Flickable {
                Layout.fillWidth: true
                implicitHeight: 280
                contentWidth: screenshotRow.implicitWidth
                contentHeight: 280
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                RowLayout {
                    id: screenshotRow
                    spacing: Tokens.padding.medium
                    height: parent.height

                    Repeater {
                        model: root.screenshotsList

                        StyledRect {
                            id: screenshotCard
                            required property string modelData
                            implicitWidth: 460
                            implicitHeight: 270
                            radius: Tokens.rounding.large
                            color: Colours.tPalette.m3surfaceContainer

                            Item {
                                id: previewMaskWrapper
                                anchors.fill: parent
                                layer.enabled: true
                                layer.smooth: true
                                layer.samples: 4

                                Rectangle {
                                    anchors.fill: parent
                                    radius: Tokens.rounding.large
                                    color: "black"
                                    antialiasing: true
                                    smooth: true
                                }
                            }

                            Image {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                smooth: true
                                mipmap: true
                                antialiasing: true
                                source: modelData

                                layer.enabled: true
                                layer.smooth: true
                                layer.samples: 4
                                layer.effect: Mask {
                                    maskSource: previewMaskWrapper
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Tokens.padding.small
            visible: root.appData && root.appData.backend === "Flatpak"

            StyledRect {
                Layout.fillWidth: true
                implicitHeight: 88
                topLeftRadius: Tokens.rounding.extraLarge
                bottomLeftRadius: Tokens.rounding.extraLarge
                topRightRadius: Tokens.rounding.extraSmall
                bottomRightRadius: Tokens.rounding.extraSmall
                color: Colours.tPalette.m3surfaceContainer

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    MaterialIcon {
                        text: "download"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: "235.5 MB"
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Download Size")
                        font: Tokens.font.label.small
                        color: Colours.palette.m3onSurfaceVariant
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            StyledRect {
                Layout.fillWidth: true
                implicitHeight: 88
                radius: Tokens.rounding.extraSmall
                color: Colours.tPalette.m3surfaceContainer

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    MaterialIcon {
                        text: "security"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Verified Safe")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Security Status")
                        font: Tokens.font.label.small
                        color: Colours.palette.m3onSurfaceVariant
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            StyledRect {
                Layout.fillWidth: true
                implicitHeight: 88
                radius: Tokens.rounding.extraSmall
                color: Colours.tPalette.m3surfaceContainer

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    MaterialIcon {
                        text: "computer"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Desktop")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Platform")
                        font: Tokens.font.label.small
                        color: Colours.palette.m3onSurfaceVariant
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            StyledRect {
                Layout.fillWidth: true
                implicitHeight: 88
                topRightRadius: Tokens.rounding.extraLarge
                bottomRightRadius: Tokens.rounding.extraLarge
                topLeftRadius: Tokens.rounding.extraSmall
                bottomLeftRadius: Tokens.rounding.extraSmall
                color: Colours.tPalette.m3surfaceContainer

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    MaterialIcon {
                        text: "star"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Everyone")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Content Rating")
                        font: Tokens.font.label.small
                        color: Colours.palette.m3onSurfaceVariant
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Tokens.padding.small

            StyledText {
                text: root.appData ? (root.appData.name || root.appData.id) : ""
                font: Tokens.font.title.large
                color: Colours.palette.m3onSurface
            }

            Item {
                Layout.fillWidth: true
                implicitHeight: root.descExpanded ? descText.implicitHeight : Math.min(descText.implicitHeight, 72)
                clip: true

                Behavior on implicitHeight {
                    NumberAnimation {
                        duration: 280
                        easing.type: Easing.OutCubic
                    }
                }

                StyledText {
                    id: descText
                    width: parent.width
                    text: (root.infoMap && (root.infoMap.description || root.infoMap.summary)) ? (root.infoMap.description || root.infoMap.summary) : ((root.appData && (root.appData.summary || root.appData.description)) ? (root.appData.summary || root.appData.description) : qsTr("An essential desktop application for your Linux system."))
                    font: Tokens.font.body.medium
                    color: Colours.palette.m3onSurfaceVariant
                    wrapMode: Text.WordWrap
                }
            }

            RowLayout {
                spacing: Tokens.padding.extraSmall
                visible: descText.implicitHeight > 72

                StyledText {
                    text: root.descExpanded ? qsTr("Show less") : qsTr("Show more")
                    font: Tokens.font.label.medium
                    color: Colours.palette.m3primary
                }

                IconButton {
                    type: ButtonBase.Text
                    icon: "expand_more"
                    rotation: root.descExpanded ? 180 : 0
                    Behavior on rotation {
                        Anim {}
                    }
                    onClicked: root.descExpanded = !root.descExpanded
                }
            }
        }

        StyledRect {
            Layout.fillWidth: true
            implicitHeight: pacmanInfoCol.implicitHeight + Tokens.padding.medium * 2
            visible: root.appData && (root.appData.backend === "Pacman" || root.appData.backend === "AUR" || root.appData.backend === "AppImage")
            radius: Tokens.rounding.large
            color: Colours.tPalette.m3surfaceContainer

            ColumnLayout {
                id: pacmanInfoCol
                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                spacing: Tokens.padding.small

                RowLayout {
                    spacing: Tokens.padding.small
                    MaterialIcon {
                        text: "info"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                    }
                    StyledText {
                        text: qsTr("Information")
                        font: Tokens.font.title.large
                        color: Colours.palette.m3onSurface
                    }
                }

                GridLayout {
                    columns: 2
                    columnSpacing: Tokens.padding.large
                    rowSpacing: Tokens.padding.small
                    Layout.fillWidth: true

                    StyledText {
                        text: qsTr("Version")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.version) ? root.infoMap.version : (root.appData ? (root.appData.version || "N/A") : "N/A")
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        text: qsTr("Repository")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.repository) ? root.infoMap.repository : (root.appData ? root.appData.backend : "N/A")
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        text: qsTr("Installed Size")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.installedSize) ? root.infoMap.installedSize : "N/A"
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        text: qsTr("Build Date")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.buildDate) ? root.infoMap.buildDate : "N/A"
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        text: qsTr("Install Reason")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.installReason) ? root.infoMap.installReason : "Explicitly installed"
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        text: qsTr("URL")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.url) ? root.infoMap.url : "N/A"
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3primary
                        elide: Text.ElideRight
                        Layout.fillWidth: true

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (root.infoMap && root.infoMap.url) Qt.openUrlExternally(root.infoMap.url);
                            }
                        }
                    }

                    StyledText {
                        text: qsTr("Licenses")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.licenses) ? root.infoMap.licenses : "Proprietary / Open Source"
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        text: qsTr("Provides")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        text: (root.infoMap && root.infoMap.provides) ? root.infoMap.provides : (root.appData ? root.appData.id : "N/A")
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.extraSmall
                    visible: root.dependsList.length > 0

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.extraSmall

                        StyledText {
                            text: qsTr("Depends On (%1)").arg(root.dependsList.length)
                            font: Tokens.font.title.small
                            color: Colours.palette.m3onSurface
                        }

                        IconButton {
                            type: ButtonBase.Text
                            icon: "expand_more"
                            rotation: root.dependsExpanded ? 180 : 0
                            Behavior on rotation {
                                Anim {}
                            }
                            onClicked: root.dependsExpanded = !root.dependsExpanded
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: root.dependsExpanded ? dependsFlow.implicitHeight : 0
                        clip: true

                        Behavior on implicitHeight {
                            NumberAnimation {
                                duration: 250
                                easing.type: Easing.OutCubic
                            }
                        }

                        Flow {
                            id: dependsFlow
                            width: parent.width
                            spacing: Tokens.padding.extraSmall

                            Repeater {
                                model: root.dependsList
                                Rectangle {
                                    required property string modelData
                                    implicitWidth: depTxt.implicitWidth + Tokens.padding.small * 2
                                    implicitHeight: depTxt.implicitHeight + Tokens.padding.extraSmall
                                    radius: Tokens.rounding.small
                                    color: Colours.palette.m3surfaceContainerHighest

                                    StyledText {
                                        id: depTxt
                                        anchors.centerIn: parent
                                        text: modelData
                                        font: Tokens.font.label.small
                                        color: Colours.palette.m3onSurface
                                    }
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.extraSmall
                    visible: root.requiredByList.length > 0

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Tokens.padding.extraSmall

                        StyledText {
                            text: qsTr("Required By (%1)").arg(root.requiredByList.length)
                            font: Tokens.font.title.small
                            color: Colours.palette.m3onSurface
                        }

                        IconButton {
                            type: ButtonBase.Text
                            icon: "expand_more"
                            rotation: root.requiredByExpanded ? 180 : 0
                            Behavior on rotation {
                                Anim {}
                            }
                            onClicked: root.requiredByExpanded = !root.requiredByExpanded
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: root.requiredByExpanded ? requiredByFlow.implicitHeight : 0
                        clip: true

                        Behavior on implicitHeight {
                            NumberAnimation {
                                duration: 250
                                easing.type: Easing.OutCubic
                            }
                        }

                        Flow {
                            id: requiredByFlow
                            width: parent.width
                            spacing: Tokens.padding.extraSmall

                            Repeater {
                                model: root.requiredByList
                                Rectangle {
                                    required property string modelData
                                    implicitWidth: reqTxt.implicitWidth + Tokens.padding.small * 2
                                    implicitHeight: reqTxt.implicitHeight + Tokens.padding.extraSmall
                                    radius: Tokens.rounding.small
                                    color: Colours.palette.m3surfaceContainerHighest

                                    StyledText {
                                        id: reqTxt
                                        anchors.centerIn: parent
                                        text: modelData
                                        font: Tokens.font.label.small
                                        color: Colours.palette.m3onSurface
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Tokens.padding.small

            StyledText {
                text: qsTr("Version 2.4.1")
                font: Tokens.font.label.small
                color: Colours.palette.m3onSurfaceVariant
            }

            Item { Layout.fillWidth: true }

            StyledText {
                text: qsTr("Updated recently")
                font: Tokens.font.label.small
                color: Colours.palette.m3onSurfaceVariant
            }
        }
    }
}
}
