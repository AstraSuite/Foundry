import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.services
import qs.modules.astra.common
import AstraMarket.Market 1.0
import AstraMarket.Theme 1.0
import AstraMarket.Tray 1.0

PageBase {
    id: root

    title: qsTr("Marketplace Settings")

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
            subtext: qsTr("Keep Astra Market running in the background when the main window is closed")
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
            subtext: qsTr("Launch Astra Market silently in the system tray on login (~/.config/autostart)")
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
