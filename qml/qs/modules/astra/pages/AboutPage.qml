import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.components.containers
import qs.services
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("About")

    ColumnLayout {
        Layout.fillWidth: true
        spacing: Tokens.spacing.extraSmall / 2

        ConnectedRect {
            Layout.fillWidth: true
            first: true
            last: true
            implicitHeight: heroCol.implicitHeight + Tokens.padding.extraLarge * 2

            ColumnLayout {
                id: heroCol
                anchors.centerIn: parent
                width: parent.width - Tokens.padding.large * 2
                spacing: Tokens.padding.small

                AnimatedLogo {
                    Layout.alignment: Qt.AlignHCenter
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: "AstraMarket"
                    font: Tokens.font.headline.large
                    color: Colours.palette.m3onSurface
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("v%1").arg(typeof appVersion !== "undefined" ? appVersion : "1.1.0")
                    font: Tokens.font.body.medium
                    color: Colours.palette.m3onSurfaceVariant
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Next-generation Linux Marketplace with unified multi-backend package support.")
                    font: Tokens.font.body.small
                    color: Colours.palette.m3onSurfaceVariant
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
            }
        }

        SectionHeader {
            text: qsTr("System")
        }

        InfoRow {
            first: true
            label: qsTr("Framework")
            value: "Qt 6.8 (C++20 & QML)"
            icon: "code"
        }

        InfoRow {
            label: qsTr("Theme Engine")
            value: "Caelestia Material 3"
            icon: "palette"
        }

        InfoRow {
            last: true
            label: qsTr("Executable")
            value: "astra (--gui / -g)"
            icon: "terminal"
        }

        SectionHeader {
            text: qsTr("Package Backends")
        }

        InfoRow {
            first: true
            label: qsTr("Arch Linux")
            value: qsTr("Pacman & AUR (yay / paru)")
            icon: "box"
        }

        InfoRow {
            last: true
            label: qsTr("Flatpak")
            value: qsTr("Flathub & System Repositories")
            icon: "deployed_code"
        }

        SectionHeader {
            text: qsTr("Credits & License")
        }

        InfoRow {
            first: true
            label: qsTr("License")
            value: "GNU GPL v3.0"
            icon: "gavel"
        }

        InfoRow {
            last: true
            label: qsTr("Design & Components")
            value: "Caelestia Shell (GPL-3.0)"
            icon: "favorite"
        }
    }
}
