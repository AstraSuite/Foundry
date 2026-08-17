pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.services
import qs.modules.astra.common

ConnectedRect {
    id: root

    property string text
    property string subtext
    property string currentValue
    property var options: []
    property int currentIndex: 0
    signal optionSelected(var value, string label)

    Layout.fillWidth: true
    implicitHeight: rowLayout.implicitHeight + rowLayout.anchors.margins * 2

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (root.options && root.options.length > 0) {
                const nextIdx = (root.currentIndex + 1) % root.options.length;
                root.currentIndex = nextIdx;
                root.optionSelected(root.options[nextIdx].value, root.options[nextIdx].label);
            }
        }
    }

    RowLayout {
        id: rowLayout
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
                text: root.text
                font: Tokens.font.body.small
                elide: Text.ElideRight
            }

            StyledText {
                Layout.fillWidth: true
                visible: root.subtext !== ""
                text: root.subtext
                color: Colours.palette.m3outline
                font: Tokens.font.label.small
                elide: Text.ElideRight
            }
        }

        RowLayout {
            spacing: Tokens.spacing.small

            StyledText {
                text: root.currentValue
                color: Colours.palette.m3primary
                font: Tokens.font.label.medium
            }

            MaterialIcon {
                text: "swap_horiz"
                color: Colours.palette.m3primary
                fontStyle: Tokens.font.icon.small
            }
        }
    }
}
