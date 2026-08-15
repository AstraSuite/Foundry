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

    title: qsTr("Explore")

    property string searchQuery: ""
    property string activeFilter: "All"
    property string activeCategory: ""
    property var packagesList: []

    property int visibleCount: 15
    property bool isFiltering: false

    readonly property bool hasActiveQuery: root.searchQuery.trim().length > 0 || root.activeCategory !== ""

    readonly property var filteredPackages: root.packagesList.filter(pkg => {
        if (root.activeCategory !== "") {
            return pkg.backend === "Flatpak";
        }
        if (root.activeFilter === "All") return true;
        if (root.activeFilter === "AUR") {
            return pkg.backend === "AUR" || pkg.repository === "local" || pkg.repository === "aur";
        }
        if (root.activeFilter === "Pacman") {
            return pkg.backend === "Pacman" || pkg.repository === "extra" || pkg.repository === "core" || pkg.repository === "multilib";
        }
        return pkg.backend === root.activeFilter;
    })

    Item {
        Timer {
            id: filterTimer
            interval: 150
            repeat: false
            onTriggered: root.isFiltering = false
        }
    }

    function triggerFilterAnimation(): void {
        root.visibleCount = 15;
        root.isFiltering = true;
        filterTimer.restart();
    }

    function refreshSearch(): void {
        triggerFilterAnimation();
        if (!hasActiveQuery) {
            root.packagesList = [];
            return;
        }
        var query = searchQuery.trim();
        if (activeCategory !== "") {
            query = activeCategory;
        }
        PackageManager.searchPackagesAsync(query);
    }

    Component.onCompleted: {
        if (hasActiveQuery) {
            refreshSearch();
        }
    }

    Item {
        Connections {
            target: root.flickable
            function onContentYChanged(): void {
                if (root.flickable && root.flickable.contentY + root.flickable.height >= root.flickable.contentHeight - 150) {
                    if (!root.isFiltering && root.visibleCount < root.filteredPackages.length) {
                        root.visibleCount = Math.min(root.visibleCount + 10, root.filteredPackages.length);
                    }
                }
            }
        }
    }

    Column {
        width: root.width
        spacing: Tokens.spacing.extraSmall / 2

        Connections {
            target: PackageManager
            function onSearchCompleted(results): void {
                root.packagesList = results ? results : [];
            }
        }

        RowLayout {
            width: parent.width
            spacing: Tokens.padding.medium

            SearchBar {
                Layout.fillWidth: true
                placeholderText: qsTr("Search Flatpak, Pacman, AUR...")
                onTextChanged: {
                    root.searchQuery = text;
                    root.refreshSearch();
                }
            }

            IconButton {
                icon: "refresh"
                onClicked: root.refreshSearch()
            }
        }

        Item { width: 1; height: Tokens.padding.extraSmall }

        RowLayout {
            width: parent.width
            spacing: 4
            z: 500

            RowLayout {
                spacing: 4

                Repeater {
                    model: ["All", "Flatpak", "Pacman", "AUR"]

                    Rectangle {
                        id: chip
                        required property string modelData
                        readonly property bool isSelected: root.activeFilter === modelData && root.activeCategory === ""

                        implicitWidth: chipRow.implicitWidth + Tokens.padding.medium * 2
                        implicitHeight: 32
                        radius: Tokens.rounding.large

                        color: isSelected ? Colours.palette.m3secondaryContainer : Colours.tPalette.m3surfaceContainerLow
                        border.color: isSelected ? Qt.rgba(0,0,0,0) : Colours.palette.m3outline
                        border.width: isSelected ? 0 : 1

                        Behavior on color { CAnim {} }
                        Behavior on border.color { CAnim {} }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.activeCategory = "";
                                root.activeFilter = chip.modelData;
                                root.refreshSearch();
                            }
                        }

                        RowLayout {
                            id: chipRow
                            anchors.centerIn: parent
                            spacing: Tokens.padding.extraSmall

                            MaterialIcon {
                                visible: chip.isSelected
                                text: "check"
                                fontStyle: Tokens.font.icon.small
                                color: Colours.palette.m3onSecondaryContainer
                            }

                            StyledText {
                                text: chip.modelData
                                font: Tokens.font.label.large
                                color: chip.isSelected ? Colours.palette.m3onSecondaryContainer : Colours.palette.m3onSurface
                            }
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Item {
                id: fabContainer
                implicitWidth: fabButton.implicitWidth
                implicitHeight: 36
                z: 500

                property bool isOpen: false

                Rectangle {
                    id: fabButton
                    implicitWidth: fabContainer.isOpen ? 36 : (fabRow.implicitWidth + Tokens.padding.medium * 2)
                    implicitHeight: 36
                    radius: 18
                    color: Colours.palette.m3primary
                    border.width: 0
                    border.color: Qt.rgba(0, 0, 0, 0)

                    Behavior on implicitWidth { Anim { type: Anim.DefaultEffects } }
                    Behavior on color { CAnim {} }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: fabContainer.isOpen = !fabContainer.isOpen
                    }

                    RowLayout {
                        id: fabRow
                        anchors.centerIn: parent
                        spacing: Tokens.padding.extraSmall

                        MaterialIcon {
                            text: fabContainer.isOpen ? "close" : "category"
                            fontStyle: Tokens.font.icon.small
                            color: Colours.palette.m3onPrimary
                        }

                        StyledText {
                            visible: !fabContainer.isOpen
                            text: root.activeCategory ? root.activeCategory : qsTr("Categories")
                            font: Tokens.font.label.large
                            color: Colours.palette.m3onPrimary
                        }

                        MaterialIcon {
                            visible: !fabContainer.isOpen && root.activeCategory !== ""
                            text: "cancel"
                            fontStyle: Tokens.font.icon.small
                            color: Colours.palette.m3onPrimary
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.activeCategory = "";
                                    root.activeFilter = "All";
                                    root.refreshSearch();
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    id: fabMenuOverlay
                    anchors.top: fabButton.bottom
                    anchors.topMargin: Tokens.padding.small
                    anchors.right: fabButton.right
                    spacing: 8
                    visible: Boolean(fabContainer && fabContainer.isOpen) || fabMenuOpacity.running
                    opacity: Boolean(fabContainer && fabContainer.isOpen) ? 1 : 0
                    z: 999

                    Behavior on opacity {
                        id: fabMenuOpacity
                        Anim { type: Anim.DefaultEffects }
                    }

                    Repeater {
                        model: [
                            { name: "Photo & Video", icon: "photo_camera" },
                            { name: "Music & Audio", icon: "headphones" },
                            { name: "Productivity", icon: "work" },
                            { name: "Communication & News", icon: "forum" },
                            { name: "Education & Science", icon: "school" },
                            { name: "Games", icon: "sports_esports" },
                            { name: "Utilities", icon: "build" },
                            { name: "Development", icon: "code" }
                        ]

                        Rectangle {
                            id: pillItem
                            required property var modelData
                            required property int index
                            readonly property bool isSelected: root.activeCategory === modelData.name

                            z: 100 - index

                            Layout.alignment: Qt.AlignRight
                            implicitWidth: pillRow.implicitWidth + Tokens.padding.medium * 2
                            implicitHeight: 36
                            radius: 18

                            color: isSelected ? Colours.palette.m3primary : Colours.palette.m3primaryContainer
                            border.color: Qt.rgba(0, 0, 0, 0)
                            border.width: 0

                            property real animProgress: 0

                            Connections {
                                target: fabContainer
                                function onIsOpenChanged(): void {
                                    if (fabContainer.isOpen) {
                                        pillItem.animProgress = 0;
                                    }
                                    animTimer.restart();
                                }
                            }

                            Timer {
                                id: animTimer
                                interval: fabContainer.isOpen ? index * 45 : (7 - index) * 30
                                onTriggered: {
                                    pillItem.animProgress = fabContainer.isOpen ? 1 : 0;
                                }
                            }

                            Behavior on animProgress {
                                NumberAnimation {
                                    duration: 260
                                    easing.type: fabContainer.isOpen ? Easing.OutBack : Easing.InQuad
                                    easing.overshoot: 1.15
                                }
                            }

                            opacity: animProgress
                            scale: 0.8 + 0.2 * animProgress

                            transform: Translate {
                                y: (1 - pillItem.animProgress) * -28
                            }

                            Behavior on color { CAnim {} }

                            MouseArea {
                                anchors.fill: parent
                                enabled: fabContainer.isOpen && pillItem.animProgress > 0.5
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.activeFilter = "Flatpak";
                                    root.activeCategory = modelData.name;
                                    fabContainer.isOpen = false;
                                    root.refreshSearch();
                                }
                            }

                            RowLayout {
                                id: pillRow
                                anchors.centerIn: parent
                                spacing: Tokens.padding.small

                                MaterialIcon {
                                    text: modelData.icon
                                    fontStyle: Tokens.font.icon.small
                                    color: pillItem.isSelected ? Colours.palette.m3onPrimary : Colours.palette.m3onPrimaryContainer
                                    Layout.alignment: Qt.AlignVCenter
                                }

                                StyledText {
                                    text: modelData.name
                                    font: Tokens.font.label.large
                                    color: pillItem.isSelected ? Colours.palette.m3onPrimary : Colours.palette.m3onPrimaryContainer
                                    Layout.alignment: Qt.AlignVCenter
                                }
                            }
                        }
                    }
                }
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        Item {
            width: parent.width
            implicitHeight: Math.max(350, root.height - 180)
            visible: !root.hasActiveQuery && !root.isFiltering

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.extraSmall

                MaterialIcon {
                    Layout.alignment: Qt.AlignHCenter
                    text: "manage_search"
                    color: Colours.palette.m3outlineVariant
                    fontStyle: Tokens.font.icon.extraLarge
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Explore packages")
                    color: Colours.palette.m3outlineVariant
                    font: Tokens.font.title.large
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Search for an app or select a category to get started.")
                    color: Colours.palette.m3outlineVariant
                    font: Tokens.font.body.large
                }
            }
        }

        Item {
            width: parent.width
            implicitHeight: Math.max(350, root.height - 180)
            visible: root.hasActiveQuery && !root.isFiltering && !PackageManager.isBusy && root.filteredPackages.length === 0

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.extraSmall

                MaterialIcon {
                    Layout.alignment: Qt.AlignHCenter
                    text: "search_off"
                    color: Colours.palette.m3outlineVariant
                    fontStyle: Tokens.font.icon.extraLarge
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No packages found")
                    color: Colours.palette.m3outlineVariant
                    font: Tokens.font.title.large
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Try searching with different keywords or switching sources.")
                    color: Colours.palette.m3outlineVariant
                    font: Tokens.font.body.large
                }
            }
        }

        Item {
            width: parent.width
            implicitHeight: Math.max(350, root.height - 180)
            visible: root.hasActiveQuery && (root.isFiltering || (PackageManager.isBusy && root.packagesList.length === 0))

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.medium

                LoadingIndicator {
                    implicitSize: 52
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    text: root.isFiltering ? qsTr("Filtering packages...") : qsTr("Searching packages catalog...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        Repeater {
            id: pkgRepeater
            model: (!root.hasActiveQuery || root.isFiltering) ? [] : root.filteredPackages.slice(0, root.visibleCount)

            ConnectedRect {
                id: cardItem
                required property var modelData
                required property int index
                width: root.width
                first: index === 0
                last: index === pkgRepeater.count - 1
                implicitHeight: cardLayout.implicitHeight + Tokens.padding.medium * 2

                property real animProgress: 0

                opacity: animProgress
                scale: 0.90 + 0.10 * animProgress

                transform: Translate {
                    y: (1 - cardItem.animProgress) * 20
                }

                Behavior on animProgress {
                    NumberAnimation {
                        duration: 260
                        easing.type: Easing.OutBack
                        easing.overshoot: 1.15
                    }
                }

                Timer {
                    id: rowAnimTimer
                    interval: Math.max(0, Math.min(index % 15, 8) * 35)
                    running: true
                    repeat: false
                    onTriggered: {
                        cardItem.animProgress = 1.0;
                    }
                }

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
                            text: (modelData.backend === "Pacman" || modelData.backend === "AUR") ? ((modelData.repository ? modelData.repository : (modelData.backend === "AUR" ? "local" : "extra")) + "/" + (modelData.id || modelData.name)) : (modelData.summary || modelData.id)
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
            visible: root.hasActiveQuery && !root.isFiltering && root.visibleCount < root.filteredPackages.length
            spacing: Tokens.padding.small

            Item { implicitHeight: Tokens.padding.small }

            LoadingIndicator {
                implicitSize: 36
                color: Colours.palette.m3primary
                Layout.alignment: Qt.AlignHCenter
            }

            StyledText {
                text: qsTr("Loading more packages...")
                font: Tokens.font.label.medium
                color: Colours.palette.m3onSurfaceVariant
                Layout.alignment: Qt.AlignHCenter
            }

            Item { implicitHeight: Tokens.padding.medium }
        }
    }
}
