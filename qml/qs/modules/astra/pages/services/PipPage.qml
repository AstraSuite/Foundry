import QtQuick
import QtQuick.Layouts
import qs.components.controls
import qs.modules.astra.common
import qs.services
import AstraMarket.Config

PageBase {
    id: root

    title: qsTr("Picture in Picture")

    readonly property list<MenuItem> positionItems: [
        MenuItem { text: qsTr("Top left") },
        MenuItem { text: qsTr("Top center") },
        MenuItem { text: qsTr("Top right") },
        MenuItem { text: qsTr("Middle left") },
        MenuItem { text: qsTr("Middle center") },
        MenuItem { text: qsTr("Middle right") },
        MenuItem { text: qsTr("Bottom left") },
        MenuItem { text: qsTr("Bottom center") },
        MenuItem { text: qsTr("Bottom right") }
    ]

    function getPositionIndex(): int {
        const p = GlobalConfig.services.pipPosition.toLowerCase();
        if (p.includes("top")) {
            if (p.includes("left")) return 0;
            if (p.includes("center")) return 1;
            return 2;
        } else if (p.includes("middle")) {
            if (p.includes("left")) return 3;
            if (p.includes("center")) return 4;
            return 5;
        } else {
            if (p.includes("left")) return 6;
            if (p.includes("center")) return 7;
            return 8;
        }
    }

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Behavior")
        }

        SelectRow {
            first: true
            label: qsTr("Position")
            subtext: qsTr("Anchor quadrant for Picture in Picture windows")
            configNode: root.targetConfig.services
            propertyName: "pipPosition"
            menuItems: root.positionItems
            active: root.positionItems[root.getPositionIndex()]
            onSelected: item => {
                root.targetConfig.services.pipPosition = item.text.toLowerCase();
                root.targetConfig.save();
            }
        }

        ToggleRow {
            Layout.fillWidth: true
            last: true
            text: qsTr("Follow active focus")
            subtext: qsTr("Automatically warp PiP to the active monitor and workspace")
            configNode: root.targetConfig.services
            propertyName: "pipFollowFocus"
            checked: root.targetConfig.services.pipFollowFocus
            onToggled: {
                root.targetConfig.services.pipFollowFocus = checked;
                root.targetConfig.save();
            }
        }
    }
}
