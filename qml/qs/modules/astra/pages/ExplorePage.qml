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

    title: qsTr("Explore")

    property string searchQuery: ""
    property string activeFilter: "All"
    property string activeCategory: ""
    property var packagesList: []

    property int visibleCount: 15
    property bool isSearching: false

    property var collections: ({})
    property bool feedLoading: false

    readonly property var homeSections: [
        { key: "popular", label: qsTr("Popular on Flathub") },
        { key: "trending", label: qsTr("Trending") },
        { key: "recently-updated", label: qsTr("Recently updated") }
    ]

    readonly property bool hasFeedContent: {
        for (let i = 0; i < root.homeSections.length; i++) {
            const apps = root.collections[root.homeSections[i].key];
            if (apps && apps.length > 0)
                return true;
        }
        return false;
    }

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
            id: searchDebounceTimer
            interval: 220
            repeat: false
            onTriggered: {
                root.performSearch();
            }
        }
    }

    function performSearch(): void {
        root.visibleCount = 15;
        if (!root.hasActiveQuery) {
            root.packagesList = [];
            root.isSearching = false;
            return;
        }
        root.isSearching = true;
        root.packagesList = []; // Clear immediately to prevent stale list flashing
        var query = root.searchQuery.trim();
        if (root.activeCategory !== "") {
            query = root.activeCategory;
        }
        var filterParam = (root.activeFilter === "All" || root.activeCategory !== "") ? "" : root.activeFilter;
        PackageManager.searchPackagesAsync(query, filterParam);
    }

    function refreshSearch(): void {
        searchDebounceTimer.stop();
        performSearch();
    }

    function loadHomeFeed(): void {
        if (!PackageManager.isFlatpakAvailable)
            return;

        root.feedLoading = true;
        for (let i = 0; i < root.homeSections.length; i++) {
            PackageManager.fetchCollectionAsync(root.homeSections[i].key, 12);
        }
    }

    Component.onCompleted: {
        if (hasActiveQuery) {
            refreshSearch();
        } else {
            loadHomeFeed();
        }
    }

    Item {
        Connections {
            target: root.flickable
            function onContentYChanged(): void {
                if (root.flickable && root.flickable.contentY + root.flickable.height >= root.flickable.contentHeight - 150) {
                    if (!root.isSearching && root.visibleCount < root.filteredPackages.length) {
                        root.visibleCount = Math.min(root.visibleCount + 10, root.filteredPackages.length);
                    }
                }
            }
        }
    }

    Column {
        width: root ? root.width : 0
        spacing: Tokens.spacing.extraSmall / 2

        Connections {
            target: root.nState

            function onFocusSearchRequested(): void {
                searchBar.forceActiveFocus();
                searchBar.selectAll();
            }

            function onRefreshRequested(): void {
                if (root.hasActiveQuery)
                    root.refreshSearch();
                else
                    root.loadHomeFeed();
            }
        }

        Connections {
            target: PackageManager
            function onSearchCompleted(results): void {
                root.packagesList = results ? results : [];
                root.isSearching = false;
            }

            function onCollectionReady(collection, apps): void {
                const updated = Object.assign({}, root.collections);
                updated[collection] = apps ? apps : [];
                root.collections = updated;
                root.feedLoading = false;
            }
        }

        RowLayout {
            width: root ? root.width : 0
            spacing: Tokens.padding.medium

            SearchBar {
                id: searchBar

                Layout.fillWidth: true
                placeholderText: qsTr("Search Flatpak, Pacman, AUR...")
                onTextChanged: {
                    root.searchQuery = text;
                    if (root.activeCategory !== "" && text.trim().length > 0) {
                        root.activeCategory = "";
                    }
                    if (text.trim().length === 0 && root.activeCategory === "") {
                        searchDebounceTimer.stop();
                        root.packagesList = [];
                        root.isSearching = false;
                    } else {
                        searchDebounceTimer.restart();
                    }
                }
            }

            IconButton {
                icon: "refresh"
                onClicked: root.refreshSearch()
            }
        }

        Item { width: 1; height: Tokens.padding.extraSmall }

        RowLayout {
            width: root ? root.width : 0
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
                                text: chip.modelData === "All" ? qsTr("All") : chip.modelData
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
                    visible: Boolean(fabContainer && fabContainer.isOpen) || (fabMenuOverlay.opacity > 0.01)
                    opacity: Boolean(fabContainer && fabContainer.isOpen) ? 1 : 0
                    z: 999

                    Behavior on opacity {
                        id: fabMenuOpacity
                        Anim { type: Anim.DefaultEffects }
                    }

                    Repeater {
                        model: [
                            { name: "Photo & Video", label: qsTr("Photo & Video"), icon: "photo_camera" },
                            { name: "Music & Audio", label: qsTr("Music & Audio"), icon: "headphones" },
                            { name: "Productivity", label: qsTr("Productivity"), icon: "work" },
                            { name: "Communication & News", label: qsTr("Communication & News"), icon: "forum" },
                            { name: "Education & Science", label: qsTr("Education & Science"), icon: "school" },
                            { name: "Games", label: qsTr("Games"), icon: "sports_esports" },
                            { name: "Utilities", label: qsTr("Utilities"), icon: "build" },
                            { name: "Development", label: qsTr("Development"), icon: "code" }
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
                                    text: modelData.label ?? modelData.name
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

        Column {
            id: homeFeed

            width: root ? root.width : 0
            spacing: Tokens.padding.medium
            visible: !root.hasActiveQuery && !root.isSearching && root.hasFeedContent

            Repeater {
                model: root.homeSections

                ColumnLayout {
                    id: feedSection

                    required property var modelData
                    readonly property var apps: root.collections[modelData.key] ?? []

                    width: homeFeed.width
                    spacing: Tokens.spacing.extraSmall
                    visible: feedSection.apps.length > 0

                    StyledText {
                        Layout.fillWidth: true
                        Layout.leftMargin: Tokens.padding.small
                        text: feedSection.modelData.label
                        font: Tokens.font.label.medium
                        color: Colours.palette.m3onSurfaceVariant
                        elide: Text.ElideRight
                    }

                    StyledFlickable {
                        Layout.fillWidth: true
                        implicitHeight: 184
                        contentWidth: feedRow.implicitWidth
                        contentHeight: height
                        flickableDirection: Flickable.HorizontalFlick
                        clip: true

                        Row {
                            id: feedRow

                            height: parent.height
                            spacing: Tokens.padding.small

                            Repeater {
                                model: feedSection.apps

                                StyledRect {
                                    id: feedCard

                                    required property var modelData

                                    implicitWidth: 158
                                    implicitHeight: 176
                                    radius: Tokens.rounding.large
                                    color: Colours.tPalette.m3surfaceContainer

                                    StateLayer {
                                        anchors.fill: parent
                                        radius: feedCard.radius
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            root.nState.selectedApp = feedCard.modelData;
                                            root.nState.openSubPage(1);
                                        }
                                    }

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: Tokens.padding.medium
                                        spacing: Tokens.padding.extraSmall

                                        StyledRect {
                                            Layout.alignment: Qt.AlignHCenter
                                            implicitWidth: 56
                                            implicitHeight: 56
                                            radius: Tokens.rounding.medium
                                            color: Colours.palette.m3surfaceContainerHigh
                                            clip: true

                                            Image {
                                                id: feedIcon

                                                anchors.fill: parent
                                                anchors.margins: 6
                                                asynchronous: true
                                                fillMode: Image.PreserveAspectFit
                                                smooth: true
                                                mipmap: true
                                                visible: feedIcon.status === Image.Ready
                                                source: PackageManager.getIconPath(feedCard.modelData.icon || feedCard.modelData.id || "", "Flatpak")
                                            }

                                            MaterialIcon {
                                                anchors.centerIn: parent
                                                visible: !feedIcon.visible
                                                text: "deployed_code"
                                                fontStyle: Tokens.font.icon.medium
                                                color: Colours.palette.m3onSurfaceVariant
                                            }
                                        }

                                        StyledText {
                                            Layout.fillWidth: true
                                            horizontalAlignment: Text.AlignHCenter
                                            text: feedCard.modelData.name || feedCard.modelData.id
                                            font: Tokens.font.title.small
                                            color: Colours.palette.m3onSurface
                                            elide: Text.ElideRight
                                        }

                                        StyledText {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            horizontalAlignment: Text.AlignHCenter
                                            text: feedCard.modelData.summary || ""
                                            font: Tokens.font.body.small
                                            color: Colours.palette.m3onSurfaceVariant
                                            wrapMode: Text.WordWrap
                                            maximumLineCount: 2
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Item {
            width: root ? root.width : 0
            implicitHeight: Math.max(350, root.height - 180)
            visible: !root.hasActiveQuery && !root.isSearching && root.feedLoading && !root.hasFeedContent

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.medium

                LoadingIndicator {
                    implicitSize: 52
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    text: qsTr("Loading recommendations...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        Item {
            width: root ? root.width : 0
            implicitHeight: Math.max(350, root.height - 180)
            visible: !root.hasActiveQuery && !root.isSearching && !root.hasFeedContent && !root.feedLoading

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
            width: root ? root.width : 0
            implicitHeight: Math.max(350, root.height - 180)
            visible: root.hasActiveQuery && !root.isSearching && !PackageManager.isBusy && root.filteredPackages.length === 0

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
            width: root ? root.width : 0
            implicitHeight: Math.max(350, root.height - 180)
            visible: root.hasActiveQuery && (root.isSearching || (PackageManager.isBusy && root.packagesList.length === 0))

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Tokens.padding.medium

                LoadingIndicator {
                    implicitSize: 52
                    color: Colours.palette.m3primary
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    text: root.activeCategory !== "" ? qsTr("Loading %1 apps...").arg(root.activeCategory) : qsTr("Searching packages catalog...")
                    font: Tokens.font.title.medium
                    color: Colours.palette.m3onSurfaceVariant
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        Repeater {
            id: pkgRepeater
            model: (!root.hasActiveQuery || root.isSearching) ? [] : root.filteredPackages.slice(0, root.visibleCount)

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
            width: root ? root.width : 0
            visible: root.hasActiveQuery && !root.isSearching && root.visibleCount < root.filteredPackages.length
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
