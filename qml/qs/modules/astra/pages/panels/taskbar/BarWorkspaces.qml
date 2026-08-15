pragma ComponentBehavior: Bound

import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Workspaces")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        StepperRow {
            first: true
            label: qsTr("Shown")
            subtext: qsTr("Number of workspaces displayed")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "shown"
            value: root.targetConfig.bar.workspaces.shown
            from: 1
            to: 20
            stepSize: 1
            onMoved: v => {
                root.targetConfig.bar.workspaces.shown = v;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Active indicator")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "activeIndicator"
            checked: root.targetConfig.bar.workspaces.activeIndicator
            onToggled: {
                root.targetConfig.bar.workspaces.activeIndicator = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Active trail")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "activeTrail"
            checked: root.targetConfig.bar.workspaces.activeTrail
            onToggled: {
                root.targetConfig.bar.workspaces.activeTrail = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Occupied background")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "occupiedBg"
            checked: root.targetConfig.bar.workspaces.occupiedBg
            onToggled: {
                root.targetConfig.bar.workspaces.occupiedBg = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            Layout.fillWidth: true
            text: qsTr("Use material icons for indicators")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "useIcon"
            checked: root.targetConfig.bar.workspaces.useIcon
            onToggled: {
                root.targetConfig.bar.workspaces.useIcon = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            Layout.fillWidth: true
            text: qsTr("Show windows")
            subtext: qsTr("Show icons of open windows on each workspace")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "showWindows"
            checked: root.targetConfig.bar.workspaces.showWindows
            onToggled: {
                root.targetConfig.bar.workspaces.showWindows = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Windows on special workspaces")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "showWindowsOnSpecialWorkspaces"
            checked: root.targetConfig.bar.workspaces.showWindowsOnSpecialWorkspaces
            onToggled: {
                root.targetConfig.bar.workspaces.showWindowsOnSpecialWorkspaces = checked;
                root.targetConfig.save();
            }
        }

        StepperRow {
            label: qsTr("Max window icons")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "maxWindowIcons"
            value: root.targetConfig.bar.workspaces.maxWindowIcons
            from: 0
            to: 20
            stepSize: 1
            onMoved: v => {
                root.targetConfig.bar.workspaces.maxWindowIcons = v;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            last: true
            text: qsTr("Per-monitor workspaces")
            subtext: qsTr("Show each monitor's workspaces independently")
            configNode: root.targetConfig.bar.workspaces
            propertyName: "perMonitorWorkspaces"
            checked: root.targetConfig.bar.workspaces.perMonitorWorkspaces
            onToggled: {
                root.targetConfig.bar.workspaces.perMonitorWorkspaces = checked;
                root.targetConfig.save();
            }
        }
    }
}
