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

    title: (nState && nState.selectedApp && (nState.selectedApp.name || nState.selectedApp.id)) ? (nState.selectedApp.name || nState.selectedApp.id) : qsTr("App Details")
    isSubPage: true

    readonly property var appData: nState ? nState.selectedApp : null
    property bool isInstalled: false
    property var infoMap: null
    readonly property var screenshotsList: (root.infoMap && root.infoMap.screenshots) ? root.infoMap.screenshots : []
    readonly property string detailUrl: (root.infoMap && (root.infoMap.url || root.infoMap.homepage)) ? (root.infoMap.url || root.infoMap.homepage) : ""
    readonly property string detailLicenses: (root.infoMap && (root.infoMap.licenses || root.infoMap.license)) ? (root.infoMap.licenses || root.infoMap.license) : ""
    readonly property string detailInstalledSize: (root.infoMap && (root.infoMap.installedSize || root.infoMap.size)) ? (root.infoMap.installedSize || root.infoMap.size) : ""
    readonly property string detailDownloadSize: (root.infoMap && root.infoMap.downloadSize) ? root.infoMap.downloadSize : ""
    readonly property var permissionList: (root.infoMap && root.infoMap.permissions) ? root.infoMap.permissions : []
    readonly property bool detailVerified: !!(root.infoMap && root.infoMap.verified)
    readonly property int detailContentFlags: (root.infoMap && root.infoMap.contentRatingFlags !== undefined) ? root.infoMap.contentRatingFlags : -1
    readonly property real detailInstalls: (root.infoMap && root.infoMap.installsTotal) ? root.infoMap.installsTotal : 0
    readonly property real detailInstallsLastMonth: (root.infoMap && root.infoMap.installsLastMonth) ? root.infoMap.installsLastMonth : 0
    readonly property string detailBuildDate: (root.infoMap && root.infoMap.buildDate) ? root.infoMap.buildDate : ""
    readonly property string detailInstallReason: (root.infoMap && root.infoMap.installReason) ? root.infoMap.installReason : ""
    readonly property string detailProvides: (root.infoMap && root.infoMap.provides) ? root.infoMap.provides : ""
    readonly property string detailMaintainer: (root.infoMap && root.infoMap.developer) ? root.infoMap.developer : ""
    readonly property int detailVotes: (root.infoMap && root.infoMap.votes) ? root.infoMap.votes : 0
    readonly property real detailPopularity: (root.infoMap && root.infoMap.popularity) ? root.infoMap.popularity : 0
    readonly property bool detailOutOfDate: !!(root.infoMap && root.infoMap.outOfDate)
    readonly property bool detailOrphaned: !!(root.infoMap && root.infoMap.orphaned)
    readonly property var dependsList: (root.infoMap && root.infoMap.depends) ? root.infoMap.depends : []
    readonly property var requiredByList: (root.infoMap && root.infoMap.requiredBy) ? root.infoMap.requiredBy : []

    property bool dependsExpanded: true
    property bool requiredByExpanded: true

    property bool descExpanded: false
    property bool buildScriptExpanded: false
    property bool buildScriptLoading: false
    property string buildScript: ""
    property bool isLoadingDetails: true
    property string selectedScope: (root.appData && root.appData.scope) ? root.appData.scope : "user"

    function formatDescription(rawText): string {
        if (!rawText) return qsTr("An essential desktop application for your Linux system.");
        var text = String(rawText).trim();
        if (text.length === 0) return qsTr("An essential desktop application for your Linux system.");

        if (text.indexOf("<p>") !== -1 || text.indexOf("<ul>") !== -1 || text.indexOf("<ol>") !== -1 || text.indexOf("<br") !== -1 || text.indexOf("<div>") !== -1) {
            return text;
        }

        return text.replace(/\n\n/g, "<br><br>").replace(/\n/g, "<br>");
    }

    function compactCount(value: real): string {
        if (value >= 1000000)
            return (value / 1000000).toFixed(1) + "M";
        if (value >= 1000)
            return Math.round(value / 1000) + "k";
        return String(value);
    }

    function toggleBuildScript(): void {
        root.buildScriptExpanded = !root.buildScriptExpanded;

        if (root.buildScriptExpanded && root.buildScript.length === 0 && !root.buildScriptLoading && root.appData) {
            root.buildScriptLoading = true;
            PackageManager.fetchBuildScriptAsync(root.appData.id || "");
        }
    }

    function loadDetails(): void {
        if (root.appData && root.appData.id) {
            root.isLoadingDetails = true;
            root.isInstalled = PackageManager.isPackageInstalled(root.appData.backend || "", root.appData.id) ||
                               Boolean(root.appData.isInstalled || root.appData.installed);
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
        width: root ? root.width : 0
        spacing: Tokens.padding.medium

        Connections {
            target: PackageManager
            function onPackageDetailsReady(details): void {
                root.infoMap = details;
                root.isLoadingDetails = false;
                if (root.appData && root.appData.id) {
                    root.isInstalled = PackageManager.isPackageInstalled(root.appData.backend || "", root.appData.id) ||
                                       Boolean(details.isInstalled || details.installed || root.appData.isInstalled || root.appData.installed);
                }
            }
            function onOperationFinished(success, message): void {
                if (success && root.appData && root.appData.id) {
                    root.isInstalled = PackageManager.isPackageInstalled(root.appData.backend || "", root.appData.id);
                    root.loadDetails();
                }
            }
            function onBuildScriptReady(packageId, script): void {
                if (!root.appData || packageId !== root.appData.id)
                    return;

                root.buildScriptLoading = false;
                root.buildScript = script;
            }
        }

        Item {
            width: root ? root.width : 0
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
            width: root ? root.width : 0
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
                        visible: root.detailVerified
                    }

                    StyledText {
                        text: qsTr("Verified")
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                        visible: root.detailVerified
                    }

                    MaterialIcon {
                        text: "verified"
                        fontStyle: Tokens.font.icon.small
                        color: Colours.palette.m3primary
                        visible: root.detailVerified
                    }
                }

                RowLayout {
                    spacing: Tokens.padding.extraSmall
                    visible: root.detailInstalls > 0

                    MaterialIcon {
                        text: "download"
                        fontStyle: Tokens.font.icon.small
                        color: Colours.palette.m3onSurfaceVariant
                    }

                    StyledText {
                        text: root.detailInstallsLastMonth > 0
                            ? qsTr("%1 installs, %2 in the last month").arg(root.compactCount(root.detailInstalls)).arg(root.compactCount(root.detailInstallsLastMonth))
                            : qsTr("%1 installs").arg(root.compactCount(root.detailInstalls))
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                    }
                }
            }

            Item { Layout.fillWidth: true }

            ColumnLayout {
                spacing: Tokens.padding.extraSmall
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                // Flatpak Scope SplitButton (Top)
                SplitButton {
                    id: scopeSplitBtn
                    visible: !root.isInstalled && !PackageManager.isOperationRunning && root.appData && root.appData.backend === "Flatpak"
                    Layout.alignment: Qt.AlignRight
                    type: SplitButton.Tonal
                    fallbackIcon: "person"
                    fallbackText: root.selectedScope === "system" ? qsTr("Flathub (System)") : qsTr("Flathub (User)")
                    menuItems: [
                        MenuItem {
                            text: qsTr("Flathub (User)")
                            icon: "person"
                            activeText: qsTr("Flathub (User)")
                            activeIcon: "person"
                            onClicked: {
                                root.selectedScope = "user";
                            }
                        },
                        MenuItem {
                            text: qsTr("Flathub (System)")
                            icon: "computer"
                            activeText: qsTr("Flathub (System)")
                            activeIcon: "computer"
                            onClicked: {
                                root.selectedScope = "system";
                            }
                        }
                    ]
                }

                // Install Button (Directly Below SplitButton)
                IconTextButton {
                    id: installBtn
                    visible: !root.isInstalled && !PackageManager.isOperationRunning
                    Layout.alignment: Qt.AlignRight
                    icon: "download"
                    text: qsTr("Install")
                    onClicked: {
                        if (root.appData) {
                            PackageManager.installPackage(root.appData.backend || "", root.appData.id || "", root.selectedScope);
                        }
                    }
                }

                // While installing / package operation in progress:
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: Tokens.padding.small
                    visible: PackageManager.isOperationRunning

                    CircularIndicator {
                        running: true
                        implicitSize: 28
                        fgColour: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignVCenter
                    }

                    IconTextButton {
                        icon: "terminal"
                        text: qsTr("View Logs")
                        type: ButtonBase.Tonal
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: root.nState.openSubPage(2)
                    }
                }

                // When installed:
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: Tokens.padding.small
                    visible: root.isInstalled && !PackageManager.isOperationRunning

                    IconButton {
                        type: ButtonBase.Text
                        icon: "delete"
                        inactiveOnColour: Colours.palette.m3error
                        activeOnColour: Colours.palette.m3error
                        onClicked: {
                            if (root.appData) {
                                PackageManager.uninstallPackage(root.appData.backend || "", root.appData.id || "", root.selectedScope);
                            }
                        }
                    }

                    IconTextButton {
                        icon: "open_in_new"
                        text: qsTr("Open")
                        onClicked: {
                            if (root.appData) {
                                PackageManager.launchApp(root.appData.backend || "", root.appData.id || "");
                            }
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
                        text: root.detailDownloadSize !== "" ? root.detailDownloadSize : "-"
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
                        text: root.detailVerified ? "verified" : "groups"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: root.detailVerified ? qsTr("Verified") : qsTr("Community")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Publisher")
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
                        text: "hard_drive"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: root.detailInstalledSize !== "" ? root.detailInstalledSize : (root.appData && root.appData.size ? root.appData.size : "-")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurface
                        Layout.alignment: Qt.AlignHCenter
                    }
                    StyledText {
                        text: qsTr("Installed Size")
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
                        text: root.detailContentFlags < 0 ? "-" : (root.detailContentFlags === 0 ? qsTr("Everyone") : qsTr("%1 flagged").arg(root.detailContentFlags))
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
                    textFormat: Text.RichText
                    text: root.formatDescription(
                        (root.infoMap && (root.infoMap.description || root.infoMap.summary))
                            ? (root.infoMap.description || root.infoMap.summary)
                            : ((root.appData && (root.appData.summary || root.appData.description))
                                ? (root.appData.summary || root.appData.description)
                                : "")
                    )
                    font: Tokens.font.body.medium
                    color: Colours.palette.m3onSurfaceVariant
                    wrapMode: Text.WordWrap
                    onLinkActivated: function(link) {
                        Qt.openUrlExternally(link);
                    }
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
            visible: root.permissionList.length > 0
            implicitHeight: permissionsColumn.implicitHeight + Tokens.padding.medium * 2
            radius: Tokens.rounding.large
            color: Colours.tPalette.m3surfaceContainer

            ColumnLayout {
                id: permissionsColumn

                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                spacing: Tokens.padding.small

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.small

                    MaterialIcon {
                        text: "shield"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                    }

                    StyledText {
                        text: qsTr("Permissions")
                        font: Tokens.font.title.large
                        color: Colours.palette.m3onSurface
                    }
                }

                StyledText {
                    Layout.fillWidth: true
                    text: qsTr("What this application is allowed to access outside its sandbox.")
                    font: Tokens.font.body.small
                    color: Colours.palette.m3onSurfaceVariant
                    wrapMode: Text.WordWrap
                }

                Repeater {
                    model: root.permissionList

                    RowLayout {
                        id: permissionRow

                        required property var modelData

                        Layout.fillWidth: true
                        spacing: Tokens.padding.small

                        MaterialIcon {
                            text: permissionRow.modelData.icon
                            fontStyle: Tokens.font.icon.small
                            color: permissionRow.modelData.sensitive ? Colours.palette.m3error : Colours.palette.m3onSurfaceVariant
                        }

                        StyledText {
                            Layout.fillWidth: true
                            text: permissionRow.modelData.label
                            font: Tokens.font.body.medium
                            color: permissionRow.modelData.sensitive ? Colours.palette.m3error : Colours.palette.m3onSurface
                            elide: Text.ElideRight
                        }
                    }
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
                        visible: root.detailInstalledSize !== ""
                        text: qsTr("Installed Size")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailInstalledSize !== ""
                        text: root.detailInstalledSize
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        visible: root.detailDownloadSize !== ""
                        text: qsTr("Download Size")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailDownloadSize !== ""
                        text: root.detailDownloadSize
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        visible: root.detailBuildDate !== ""
                        text: qsTr("Build Date")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailBuildDate !== ""
                        text: root.detailBuildDate
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    StyledText {
                        visible: root.detailInstallReason !== ""
                        text: qsTr("Install Reason")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailInstallReason !== ""
                        text: root.detailInstallReason
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    StyledText {
                        visible: root.detailVotes > 0
                        text: qsTr("Votes")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailVotes > 0
                        text: root.detailVotes
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        visible: root.detailPopularity > 0
                        text: qsTr("Popularity")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailPopularity > 0
                        text: root.detailPopularity.toFixed(2)
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                    }

                    StyledText {
                        visible: root.detailMaintainer !== "" || root.detailOrphaned
                        text: root.appData && root.appData.backend === "AUR" ? qsTr("Maintainer") : qsTr("Packager")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailMaintainer !== "" || root.detailOrphaned
                        text: root.detailMaintainer !== "" ? root.detailMaintainer : qsTr("Orphaned")
                        font: Tokens.font.body.medium
                        color: root.detailMaintainer !== "" ? Colours.palette.m3onSurface : Colours.palette.m3error
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    StyledText {
                        visible: root.detailOutOfDate
                        text: qsTr("Status")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailOutOfDate
                        text: qsTr("Flagged out of date")
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3error
                    }

                    StyledText {
                        visible: root.detailUrl !== ""
                        text: qsTr("URL")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailUrl !== ""
                        text: root.detailUrl
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3primary
                        elide: Text.ElideRight
                        Layout.fillWidth: true

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally(root.detailUrl)
                        }
                    }

                    StyledText {
                        visible: root.detailLicenses !== ""
                        text: qsTr("Licenses")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailLicenses !== ""
                        text: root.detailLicenses
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    StyledText {
                        visible: root.detailProvides !== ""
                        text: qsTr("Provides")
                        font: Tokens.font.label.large
                        color: Colours.palette.m3onSurfaceVariant
                    }
                    StyledText {
                        visible: root.detailProvides !== ""
                        text: root.detailProvides
                        font: Tokens.font.body.medium
                        color: Colours.palette.m3onSurface
                        elide: Text.ElideRight
                        Layout.fillWidth: true
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

        StyledRect {
            Layout.fillWidth: true
            visible: root.appData && root.appData.backend === "AUR"
            implicitHeight: buildScriptColumn.implicitHeight + Tokens.padding.medium * 2
            radius: Tokens.rounding.large
            color: Colours.tPalette.m3surfaceContainer

            ColumnLayout {
                id: buildScriptColumn

                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                spacing: Tokens.padding.small

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Tokens.padding.small

                    MaterialIcon {
                        text: "terminal"
                        fontStyle: Tokens.font.icon.medium
                        color: Colours.palette.m3primary
                    }

                    StyledText {
                        text: qsTr("PKGBUILD")
                        font: Tokens.font.title.large
                        color: Colours.palette.m3onSurface
                    }

                    Item { Layout.fillWidth: true }

                    IconButton {
                        type: ButtonBase.Text
                        icon: "expand_more"
                        rotation: root.buildScriptExpanded ? 180 : 0

                        Behavior on rotation {
                            Anim {}
                        }

                        onClicked: root.toggleBuildScript()
                    }
                }

                StyledText {
                    Layout.fillWidth: true
                    visible: !root.buildScriptExpanded
                    text: qsTr("AUR packages are built from a script that runs on your machine. Review it before installing.")
                    font: Tokens.font.body.small
                    color: Colours.palette.m3onSurfaceVariant
                    wrapMode: Text.WordWrap
                }

                Item {
                    Layout.fillWidth: true
                    implicitHeight: root.buildScriptExpanded ? 320 : 0
                    clip: true

                    Behavior on implicitHeight {
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.OutCubic
                        }
                    }

                    StyledRect {
                        anchors.fill: parent
                        radius: Tokens.rounding.medium
                        color: Colours.palette.m3surfaceContainerLow

                        LoadingIndicator {
                            anchors.centerIn: parent
                            visible: root.buildScriptLoading
                            implicitSize: 32
                            color: Colours.palette.m3primary
                        }

                        StyledText {
                            anchors.centerIn: parent
                            visible: !root.buildScriptLoading && root.buildScript.length === 0
                            text: qsTr("The build script could not be loaded.")
                            font: Tokens.font.body.small
                            color: Colours.palette.m3onSurfaceVariant
                        }

                        StyledFlickable {
                            anchors.fill: parent
                            anchors.margins: Tokens.padding.small
                            visible: root.buildScript.length > 0
                            contentWidth: buildScriptText.implicitWidth
                            contentHeight: buildScriptText.implicitHeight
                            clip: true

                            Text {
                                id: buildScriptText

                                text: root.buildScript
                                font.family: "Monospace, DejaVu Sans Mono, Courier New"
                                font.pixelSize: 12
                                color: Colours.palette.m3onSurface
                                textFormat: Text.PlainText
                                wrapMode: Text.NoWrap
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
