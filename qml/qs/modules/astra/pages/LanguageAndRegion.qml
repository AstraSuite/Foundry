import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.services
import qs.modules.astra.common

PageBase {
    id: root

    readonly property list<MenuItem> tempItems: [
        MenuItem {
            text: "°C"
        },
        MenuItem {
            text: "°F"
        }
    ]

    readonly property list<MenuItem> clockItems: [
        MenuItem {
            text: qsTr("24-hour")
        },
        MenuItem {
            text: qsTr("12-hour")
        }
    ]

    title: qsTr("Language & region")

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Language")
        }

        ConnectedRect {
            Layout.fillWidth: true
            first: true
            last: true
            implicitHeight: localeLayout.implicitHeight + localeLayout.anchors.margins * 2

            RowLayout {
                id: localeLayout

                anchors.fill: parent
                anchors.margins: Tokens.padding.medium
                anchors.leftMargin: Tokens.padding.largeIncreased
                anchors.rightMargin: Tokens.padding.largeIncreased
                spacing: Tokens.spacing.medium

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    StyledText {
                        Layout.fillWidth: true
                        text: qsTr("System language")
                        font: Tokens.font.body.small
                        elide: Text.ElideRight
                    }

                    StyledText {
                        Layout.fillWidth: true
                        text: qsTr("Follows your system locale (%1)").arg(Qt.locale().name)
                        color: Colours.palette.m3outline
                        font: Tokens.font.label.small
                        elide: Text.ElideRight
                    }
                }

                StyledText {
                    text: Qt.locale().nativeLanguageName || Qt.locale().name
                    color: Colours.palette.m3onSurfaceVariant
                    font: Tokens.font.body.small
                }
            }
        }

        SectionHeader {
            text: qsTr("Weather")
        }

        ConnectedRect {
            Layout.fillWidth: true
            first: true
            last: true
            implicitHeight: comingSoon.implicitHeight + Tokens.padding.extraLarge * 2

            ColumnLayout {
                id: comingSoon

                anchors.centerIn: parent
                width: parent.width - Tokens.padding.largeIncreased * 2
                spacing: Tokens.padding.extraSmall

                MaterialIcon {
                    Layout.alignment: Qt.AlignHCenter
                    text: "map"
                    color: Colours.palette.m3outlineVariant
                    fontStyle: Tokens.font.icon.extraLarge
                }

                StyledText {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Location picker coming soon")
                    color: Colours.palette.m3outlineVariant
                    font: Tokens.font.title.small
                }

                StyledText {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: qsTr("Choose your weather location on a map in a future update")
                    color: Colours.palette.m3outlineVariant
                    font: Tokens.font.body.small
                }
            }
        }

        SectionHeader {
            text: qsTr("Units")
        }

        SelectRow {
            first: true
            label: qsTr("Temperature")
            subtext: qsTr("Units for weather temperatures")
            configNode: root.targetConfig.services
            propertyName: "useFahrenheit"
            menuItems: root.tempItems
            active: root.tempItems[root.targetConfig.services.useFahrenheit ? 1 : 0]
            onSelected: item => {
                root.targetConfig.services.useFahrenheit = root.tempItems.indexOf(item) === 1;
                root.targetConfig.save();
            }
        }

        SelectRow {
            last: true
            label: qsTr("System temperatures")
            subtext: qsTr("Units for CPU and GPU temperatures")
            configNode: root.targetConfig.services
            propertyName: "useFahrenheitPerformance"
            menuItems: root.tempItems
            active: root.tempItems[root.targetConfig.services.useFahrenheitPerformance ? 1 : 0]
            onSelected: item => {
                root.targetConfig.services.useFahrenheitPerformance = root.tempItems.indexOf(item) === 1;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Time & date")
        }

        SelectRow {
            first: true
            last: true
            label: qsTr("Clock format")
            subtext: qsTr("How times are shown across the shell")
            configNode: root.targetConfig.services
            propertyName: "useTwelveHourClock"
            menuItems: root.clockItems
            active: root.clockItems[root.targetConfig.services.useTwelveHourClock ? 1 : 0]
            onSelected: item => {
                root.targetConfig.services.useTwelveHourClock = root.clockItems.indexOf(item) === 1;
                root.targetConfig.save();
            }
        }
    }
}
