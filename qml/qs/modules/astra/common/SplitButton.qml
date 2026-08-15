import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.services

Item {
    id: root

    property string mainText: qsTr("Install")
    property string mainIcon: "download"
    property string currentScope: "user"
    property string currentSourceLabel: qsTr("Flathub (User)")
    property var sourcesList: [
        { id: "user", label: qsTr("Flathub (User)"), desc: qsTr("Install in user directory (~/.local)") },
        { id: "system", label: qsTr("Flathub (System)"), desc: qsTr("Install system-wide (/var/lib)") }
    ]
    property bool isSplit: true

    signal mainClicked()
    signal sourceSelected(string scopeId, string scopeLabel)

    implicitWidth: mainRow.implicitWidth
    implicitHeight: 38

    RowLayout {
        id: mainRow
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: mainBtnRect
            Layout.fillHeight: true
            implicitWidth: mainContentRow.implicitWidth + Tokens.padding.medium * 2
            color: mainMouse.pressed ? Colours.palette.m3primaryContainer : (mainMouse.containsMouse ? Qt.lighter(Colours.palette.m3primary, 1.1) : Colours.palette.m3primary)
            radius: Tokens.rounding.large
            topLeftRadius: Tokens.rounding.large
            bottomLeftRadius: Tokens.rounding.large
            topRightRadius: root.isSplit ? 0 : Tokens.rounding.large
            bottomRightRadius: root.isSplit ? 0 : Tokens.rounding.large

            Behavior on color {
                CAnim {}
            }

            RowLayout {
                id: mainContentRow
                anchors.centerIn: parent
                spacing: Tokens.padding.extraSmall

                MaterialIcon {
                    text: root.mainIcon
                    fontStyle: Tokens.font.icon.small
                    color: Colours.palette.m3onPrimary
                }

                StyledText {
                    text: root.mainText
                    font: Tokens.font.label.large
                    color: Colours.palette.m3onPrimary
                }
            }

            MouseArea {
                id: mainMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.mainClicked()
            }
        }

        Rectangle {
            visible: root.isSplit
            Layout.fillHeight: true
            implicitWidth: 1
            color: Qt.rgba(1, 1, 1, 0.25)
        }

        Rectangle {
            id: dropBtnRect
            visible: root.isSplit
            Layout.fillHeight: true
            implicitWidth: dropContentRow.implicitWidth + Tokens.padding.small * 2
            color: dropMouse.pressed || popupMenu.visible ? Qt.darker(Colours.palette.m3primary, 1.15) : (dropMouse.containsMouse ? Qt.lighter(Colours.palette.m3primary, 1.1) : Colours.palette.m3primary)
            radius: Tokens.rounding.large
            topLeftRadius: 0
            bottomLeftRadius: 0
            topRightRadius: Tokens.rounding.large
            bottomRightRadius: Tokens.rounding.large

            Behavior on color {
                CAnim {}
            }

            RowLayout {
                id: dropContentRow
                anchors.centerIn: parent
                spacing: Tokens.padding.extraSmall / 2

                StyledText {
                    text: root.currentSourceLabel
                    font: Tokens.font.label.medium
                    color: Colours.palette.m3onPrimary
                }

                MaterialIcon {
                    text: popupMenu.visible ? "arrow_drop_up" : "arrow_drop_down"
                    fontStyle: Tokens.font.icon.medium
                    color: Colours.palette.m3onPrimary
                }
            }

            MouseArea {
                id: dropMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: popupMenu.visible = !popupMenu.visible
            }
        }
    }

    Rectangle {
        id: popupMenu
        visible: false
        z: 9999
        anchors.top: parent.bottom
        anchors.topMargin: Tokens.spacing.extraSmall
        anchors.right: parent.right
        width: Math.max(220, mainRow.implicitWidth)
        implicitHeight: popupCol.implicitHeight + Tokens.padding.small * 2
        radius: Tokens.rounding.medium
        color: Colours.palette.m3surfaceContainerHigh
        border.color: Colours.palette.m3outlineVariant
        border.width: 1

        layer.enabled: true
        layer.smooth: true

        ColumnLayout {
            id: popupCol
            anchors.fill: parent
            anchors.margins: Tokens.padding.small
            spacing: 2

            Repeater {
                model: root.sourcesList

                Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: itemCol.implicitHeight + Tokens.padding.small
                    radius: Tokens.rounding.small
                    color: itemMouse.containsMouse ? Colours.palette.m3surfaceContainerHighest : (root.currentScope === modelData.id ? Colours.palette.m3secondaryContainer : "transparent")

                    Behavior on color {
                        CAnim {}
                    }

                    RowLayout {
                        id: itemRow
                        anchors.fill: parent
                        anchors.leftMargin: Tokens.padding.small
                        anchors.rightMargin: Tokens.padding.small
                        spacing: Tokens.padding.small

                        MaterialIcon {
                            text: root.currentScope === modelData.id ? "check" : "radio_button_unchecked"
                            fontStyle: Tokens.font.icon.small
                            color: root.currentScope === modelData.id ? Colours.palette.m3primary : Colours.palette.m3onSurfaceVariant
                        }

                        ColumnLayout {
                            id: itemCol
                            Layout.fillWidth: true
                            spacing: 1

                            StyledText {
                                text: modelData.label || modelData.id
                                font: Tokens.font.label.large
                                color: Colours.palette.m3onSurface
                            }

                            StyledText {
                                text: modelData.desc || ""
                                font: Tokens.font.body.small
                                color: Colours.palette.m3onSurfaceVariant
                                visible: (modelData.desc || "") !== ""
                            }
                        }
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentScope = modelData.id;
                            root.currentSourceLabel = modelData.label;
                            popupMenu.visible = false;
                            root.sourceSelected(modelData.id, modelData.label);
                        }
                    }
                }
            }
        }
    }
}
