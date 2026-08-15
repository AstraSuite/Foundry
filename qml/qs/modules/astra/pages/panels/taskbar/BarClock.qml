pragma ComponentBehavior: Bound

import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Clock")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        ToggleRow {
            first: true
            text: qsTr("Background")
            configNode: root.targetConfig.bar.clock
            propertyName: "background"
            checked: root.targetConfig.bar.clock.background
            onToggled: {
                root.targetConfig.bar.clock.background = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Show date")
            configNode: root.targetConfig.bar.clock
            propertyName: "showDate"
            checked: root.targetConfig.bar.clock.showDate
            onToggled: {
                root.targetConfig.bar.clock.showDate = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            last: true
            text: qsTr("Show icon")
            configNode: root.targetConfig.bar.clock
            propertyName: "showIcon"
            checked: root.targetConfig.bar.clock.showIcon
            onToggled: {
                root.targetConfig.bar.clock.showIcon = checked;
                root.targetConfig.save();
            }
        }
    }
}
