import QtQuick
import QtQuick.Layouts
import Foundry.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.services
import qs.modules.astra.common
import Foundry.Market 1.0
import Foundry.Theme 1.0
import Foundry.Tray 1.0

PageBase {
    id: root

    title: qsTr("Marketplace Settings")

    property var remotes: []
    property string remoteStatus: ""

    function reloadRemotes(): void {
        root.remotes = PackageManager.isFlatpakAvailable ? PackageManager.flatpakRemotes() : [];
    }

    Component.onCompleted: {
        reloadRemotes();
    }

    data: [
        Connections {
            target: PackageManager

            function onRemotesChanged(): void {
                root.reloadRemotes();
            }

            function onRemoteOperationFinished(success, message): void {
                root.remoteStatus = message;
            }
        }
    ]

    Column {
        width: root ? root.width : 0
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            text: qsTr("App Sources")
        }

        ToggleRow {
            width: parent.width
            first: true
            text: qsTr("Flatpak (Flathub & System)")
            subtext: PackageManager.isFlatpakAvailable ? qsTr("Enable Flathub and Flatpak package management") : qsTr("flatpak dependency is not installed on this system")
            checked: PackageManager.isFlatpakAvailable && PackageManager.enableFlatpak
            enabled: PackageManager.isFlatpakAvailable
            disabled: !PackageManager.isFlatpakAvailable
            onCheckedChanged: {
                if (PackageManager.isFlatpakAvailable) {
                    PackageManager.enableFlatpak = checked;
                }
            }
        }

        ToggleRow {
            width: parent.width
            text: qsTr("Pacman (Arch Linux)")
            subtext: PackageManager.isPacmanAvailable ? qsTr("Enable native Arch Linux package management") : qsTr("pacman dependency is not installed on this system")
            checked: PackageManager.isPacmanAvailable && PackageManager.enablePacman
            enabled: PackageManager.isPacmanAvailable
            disabled: !PackageManager.isPacmanAvailable
            onCheckedChanged: {
                if (PackageManager.isPacmanAvailable) {
                    PackageManager.enablePacman = checked;
                }
            }
        }

        ToggleRow {
            width: parent.width
            text: qsTr("Arch User Repository (AUR)")
            subtext: PackageManager.isAurAvailable ? qsTr("Enable AUR packages via yay / paru helper") : qsTr("paru or yay helper dependency is not installed on this system")
            checked: PackageManager.isAurAvailable && PackageManager.enableAur
            enabled: PackageManager.isAurAvailable
            disabled: !PackageManager.isAurAvailable
            onCheckedChanged: {
                if (PackageManager.isAurAvailable) {
                    PackageManager.enableAur = checked;
                }
            }
        }

        ToggleRow {
            width: parent.width
            last: true
            text: qsTr("AppImage Installer")
            subtext: qsTr("Enable AppImage drag & drop installer & desktop integration")
            checked: PackageManager.enableAppImage
            enabled: true
            disabled: false
            onCheckedChanged: {
                PackageManager.enableAppImage = checked;
            }
        }

        Item {
            width: 1
            height: Tokens.padding.small
            visible: PackageManager.isFlatpakAvailable
        }

        SectionHeader {
            text: qsTr("Flatpak Remotes")
            visible: PackageManager.isFlatpakAvailable
        }

        Repeater {
            id: remotesRepeater

            model: PackageManager.isFlatpakAvailable ? root.remotes : []

            ConnectedRect {
                id: remoteRow

                required property var modelData
                required property int index

                width: root.width
                first: index === 0
                implicitHeight: remoteLayout.implicitHeight + Tokens.padding.medium * 2

                RowLayout {
                    id: remoteLayout

                    anchors.fill: parent
                    anchors.margins: Tokens.padding.medium
                    anchors.leftMargin: Tokens.padding.largeIncreased
                    anchors.rightMargin: Tokens.padding.medium
                    spacing: Tokens.padding.small

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: Tokens.padding.small

                            StyledText {
                                text: remoteRow.modelData.title
                                font: Tokens.font.body.small
                                color: Colours.palette.m3onSurface
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                implicitWidth: scopeText.implicitWidth + Tokens.padding.small * 2
                                implicitHeight: scopeText.implicitHeight + Tokens.padding.extraSmall
                                radius: Tokens.rounding.small
                                color: Colours.palette.m3secondaryContainer

                                StyledText {
                                    id: scopeText

                                    anchors.centerIn: parent
                                    text: remoteRow.modelData.scope === "system" ? qsTr("System") : qsTr("User")
                                    font: Tokens.font.label.small
                                    color: Colours.palette.m3onSecondaryContainer
                                }
                            }

                            Item { Layout.fillWidth: true }
                        }

                        StyledText {
                            Layout.fillWidth: true
                            text: remoteRow.modelData.url
                            font: Tokens.font.label.small
                            color: Colours.palette.m3outline
                            elide: Text.ElideRight
                        }
                    }

                    IconButton {
                        icon: "delete"
                        type: IconButton.Text
                        inactiveOnColour: Colours.palette.m3error
                        activeOnColour: Colours.palette.m3error
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: PackageManager.removeFlatpakRemote(remoteRow.modelData.name, remoteRow.modelData.scope)
                    }
                }
            }
        }

        ConnectedRect {
            id: addRemoteRow

            width: root.width
            visible: PackageManager.isFlatpakAvailable
            first: root.remotes.length === 0
            last: true
            implicitHeight: addRemoteLayout.implicitHeight + Tokens.padding.medium * 2

            RowLayout {
                id: addRemoteLayout

                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                anchors.leftMargin: Tokens.padding.largeIncreased
                anchors.rightMargin: Tokens.padding.medium
                spacing: Tokens.padding.small

                StyledRect {
                    Layout.preferredWidth: 140
                    implicitHeight: 36
                    radius: Tokens.rounding.full
                    color: Colours.tPalette.m3surfaceContainerHigh

                    TextFieldBase {
                        id: remoteNameField

                        anchors.fill: parent
                        leftPadding: Tokens.padding.medium
                        rightPadding: Tokens.padding.medium
                    }

                    StyledText {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: Tokens.padding.medium
                        visible: remoteNameField.text.length === 0
                        text: qsTr("Name")
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                    }
                }

                StyledRect {
                    Layout.fillWidth: true
                    implicitHeight: 36
                    radius: Tokens.rounding.full
                    color: Colours.tPalette.m3surfaceContainerHigh

                    TextFieldBase {
                        id: remoteUrlField

                        anchors.fill: parent
                        leftPadding: Tokens.padding.medium
                        rightPadding: Tokens.padding.medium
                    }

                    StyledText {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: Tokens.padding.medium
                        anchors.rightMargin: Tokens.padding.medium
                        visible: remoteUrlField.text.length === 0
                        text: qsTr("Repository URL or .flatpakrepo file")
                        font: Tokens.font.body.small
                        color: Colours.palette.m3onSurfaceVariant
                        elide: Text.ElideRight
                    }
                }

                IconTextButton {
                    icon: "add"
                    text: qsTr("Add")
                    type: ButtonBase.Tonal
                    enabled: remoteNameField.text.trim().length > 0 && remoteUrlField.text.trim().length > 0
                    Layout.alignment: Qt.AlignVCenter
                    onClicked: {
                        PackageManager.addFlatpakRemote(remoteNameField.text, remoteUrlField.text, "user");
                        remoteNameField.clear();
                        remoteUrlField.clear();
                    }
                }
            }
        }

        StyledText {
            width: root.width
            visible: PackageManager.isFlatpakAvailable && root.remoteStatus !== ""
            text: root.remoteStatus
            font: Tokens.font.label.small
            color: Colours.palette.m3onSurfaceVariant
            leftPadding: Tokens.padding.largeIncreased
            topPadding: Tokens.spacing.extraSmall
            wrapMode: Text.WordWrap
        }

        Item { width: 1; height: Tokens.padding.small }

        SectionHeader {
            text: qsTr("System Tray & Background Daemon")
        }

        ToggleRow {
            width: parent.width
            first: true
            text: qsTr("Enable System Tray Icon")
            subtext: qsTr("Show status and quick actions in the system tray / notification area")
            checked: TrayManager.trayEnabled
            onCheckedChanged: {
                TrayManager.trayEnabled = checked;
            }
        }

        ToggleRow {
            width: parent.width
            text: qsTr("Close Window to Tray")
            subtext: qsTr("Keep Foundry running in the background when the main window is closed")
            checked: TrayManager.closeToTray
            enabled: TrayManager.trayEnabled
            disabled: !TrayManager.trayEnabled
            onCheckedChanged: {
                TrayManager.closeToTray = checked;
            }
        }

        ToggleRow {
            width: parent.width
            last: true
            text: qsTr("Start Minimized on Boot")
            subtext: qsTr("Launch Foundry silently in the system tray on login (~/.config/autostart)")
            checked: TrayManager.autostart
            onCheckedChanged: {
                TrayManager.autostart = checked;
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        SectionHeader {
            text: qsTr("Automated Update Checks & Notifications")
        }

        StepperRow {
            width: parent.width
            first: true
            label: qsTr("Check Frequency")
            subtext: TrayManager.checkIntervalHours === 0 ? qsTr("Manual updates only (scheduled checks disabled)") : qsTr("Check for updates every %1 %2").arg(TrayManager.checkIntervalHours).arg(TrayManager.checkIntervalHours === 1 ? qsTr("hour") : qsTr("hours"))
            from: 0
            stepSize: 1
            suffix: "h"
            value: TrayManager.checkIntervalHours
            onMoved: (val) => {
                TrayManager.checkIntervalHours = Math.round(val);
            }
        }

        StepperRow {
            width: parent.width
            label: qsTr("Notification Threshold")
            subtext: qsTr("Notify when %1 %2 available").arg(TrayManager.notifyThreshold).arg(TrayManager.notifyThreshold === 1 ? qsTr("or more update is") : qsTr("or more updates are"))
            from: 1
            stepSize: 1
            suffix: ""
            value: TrayManager.notifyThreshold
            onMoved: (val) => {
                TrayManager.notifyThreshold = Math.round(val);
            }
        }

        ToggleRow {
            visible: PackageManager.isCaelestiaAvailable
            width: parent.width
            text: qsTr("Use 'caelestia update' for System Updates")
            subtext: qsTr("Delegate Pacman and AUR updates to Caelestia CLI (bypasses individual Pacman/AUR checks)")
            checked: TrayManager.useCaelestiaUpdate
            onCheckedChanged: {
                TrayManager.useCaelestiaUpdate = checked;
            }
        }

        ToggleRow {
            width: parent.width
            last: !TrayManager.useCaelestiaUpdate && !PackageManager.isPacmanAvailable && !PackageManager.isAurAvailable
            text: qsTr("Auto-Update Flatpak Packages")
            subtext: qsTr("Automatically download and apply Flathub updates in the background")
            checked: TrayManager.autoUpdateFlatpak
            enabled: PackageManager.isFlatpakAvailable && PackageManager.enableFlatpak
            disabled: !PackageManager.isFlatpakAvailable || !PackageManager.enableFlatpak
            onCheckedChanged: {
                TrayManager.autoUpdateFlatpak = checked;
            }
        }

        ToggleRow {
            visible: TrayManager.useCaelestiaUpdate
            width: parent.width
            last: true
            text: qsTr("Auto-Update via Caelestia CLI")
            subtext: qsTr("Automatically run 'caelestia update --noconfirm' on scheduled update checks")
            checked: TrayManager.autoUpdateCaelestia
            onCheckedChanged: {
                TrayManager.autoUpdateCaelestia = checked;
            }
        }

        ToggleRow {
            visible: !TrayManager.useCaelestiaUpdate
            width: parent.width
            text: qsTr("Auto-Update Pacman Packages")
            subtext: qsTr("Automatically download and apply native system updates in the background")
            checked: TrayManager.autoUpdatePacman
            enabled: PackageManager.isPacmanAvailable && PackageManager.enablePacman
            disabled: !PackageManager.isPacmanAvailable || !PackageManager.enablePacman
            onCheckedChanged: {
                TrayManager.autoUpdatePacman = checked;
            }
        }

        ToggleRow {
            visible: !TrayManager.useCaelestiaUpdate
            width: parent.width
            last: true
            text: qsTr("Auto-Update AUR Packages")
            subtext: qsTr("Automatically rebuild and update AUR packages in the background")
            checked: TrayManager.autoUpdateAur
            enabled: PackageManager.isAurAvailable && PackageManager.enableAur
            disabled: !PackageManager.isAurAvailable || !PackageManager.enableAur
            onCheckedChanged: {
                TrayManager.autoUpdateAur = checked;
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        SectionHeader {
            text: qsTr("Caelestia Appearance Sync")
        }

        ToggleRow {
            width: parent.width
            first: true
            text: qsTr("Sync Color Scheme")
            subtext: qsTr("Sync colors and dark/light mode with Caelestia theme daemon")
            checked: ThemeWatcher.syncScheme
            onCheckedChanged: {
                ThemeWatcher.syncScheme = checked;
            }
        }

        ToggleRow {
            width: parent.width
            last: true
            text: qsTr("Sync Design Tokens")
            subtext: qsTr("Sync sizing, padding, and rounding metrics with Caelestia")
            checked: ThemeWatcher.syncTokens
            onCheckedChanged: {
                ThemeWatcher.syncTokens = checked;
            }
        }

        Item { width: 1; height: Tokens.padding.small }

        SectionHeader {
            text: qsTr("Status")
        }

        InfoRow {
            width: parent.width
            first: true
            label: qsTr("Last Update Check")
            value: TrayManager.lastCheckString
            icon: "schedule"
        }

        InfoRow {
            width: parent.width
            last: true
            label: qsTr("Pending Updates")
            value: qsTr("%1 available").arg(TrayManager.pendingUpdateCount)
            icon: "update"
        }
    }
}
