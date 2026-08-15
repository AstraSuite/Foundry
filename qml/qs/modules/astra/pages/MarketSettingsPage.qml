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

PageBase {
    id: root

    title: qsTr("Marketplace Settings")

    Column {
        width: root.width
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

    }
}
