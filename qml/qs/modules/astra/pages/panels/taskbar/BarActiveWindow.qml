pragma ComponentBehavior: Bound

import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Active window")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        ToggleRow {
            first: true
            text: qsTr("Enable component")
            checked: {
                const entries = Config.bar.entries;
                for (const section of [entries.start, entries.center, entries.end]) {
                    for (let i = 0; i < section.count; i++) {
                        if (section.at(i).id === "activeWindow")
                            return section.at(i).enabled;
                    }
                }
                return false;
            }
            onToggled: {
                const entries = GlobalConfig.bar.entries;
                for (const section of [entries.start, entries.center, entries.end]) {
                    for (let i = 0; i < section.count; i++) {
                        if (section.at(i).id === "activeWindow") {
                            section.at(i).enabled = checked;
                            GlobalConfig.save();
                            return;
                        }
                    }
                }
                entries.center.insert({
                    id: "activeWindow",
                    enabled: checked
                });
                GlobalConfig.save();
            }
        }

        ToggleRow {
            Layout.fillWidth: true
            text: qsTr("Compact")
            configNode: root.targetConfig.bar.activeWindow
            propertyName: "compact"
            checked: root.targetConfig.bar.activeWindow.compact
            onToggled: {
                root.targetConfig.bar.activeWindow.compact = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Inverted")
            configNode: root.targetConfig.bar.activeWindow
            propertyName: "inverted"
            checked: root.targetConfig.bar.activeWindow.inverted
            onToggled: {
                root.targetConfig.bar.activeWindow.inverted = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Show on hover")
            subtext: qsTr("Only show the active window title while hovering")
            configNode: root.targetConfig.bar.activeWindow
            propertyName: "showOnHover"
            checked: root.targetConfig.bar.activeWindow.showOnHover
            onToggled: {
                root.targetConfig.bar.activeWindow.showOnHover = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            last: true
            text: qsTr("Popout on hover")
            subtext: qsTr("Show a window details popout when hovering")
            configNode: root.targetConfig.bar.popouts
            propertyName: "activeWindow"
            checked: root.targetConfig.bar.popouts.activeWindow
            onToggled: {
                root.targetConfig.bar.popouts.activeWindow = checked;
                root.targetConfig.save();
            }
        }
    }
}
