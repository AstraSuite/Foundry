pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.services
import qs.modules.astra.common

PageBase {
    id: root

    readonly property var builtinComponents: ({
            logo: qsTr("Logo"),
            workspaces: qsTr("Workspaces"),
            github: qsTr("GitHub"),
            spotify: qsTr("Spotify"),
            spacer: qsTr("Spacer"),
            activeWindow: qsTr("Active window"),
            tray: qsTr("System tray"),
            clock: qsTr("Clock"),
            statusIcons: qsTr("Status icons"),
            dock: qsTr("Dock"),
            power: qsTr("Power menu")
        })

    readonly property bool isHorizontal: root.targetConfig.bar.position === "top" || root.targetConfig.bar.position === "bottom"
    readonly property string startLabel: isHorizontal ? qsTr("Left") : qsTr("Top")
    readonly property string endLabel: isHorizontal ? qsTr("Right") : qsTr("Bottom")

    title: qsTr("Taskbar components")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        EntrySectionEditor {
            sectionLabel: root.startLabel
            targetList: root.targetConfig.bar.entries.start
            first: true
        }

        EntrySectionEditor {
            sectionLabel: qsTr("Center")
            targetList: root.targetConfig.bar.entries.center
        }

        EntrySectionEditor {
            sectionLabel: root.endLabel
            targetList: root.targetConfig.bar.entries.end
        }
    }

    component EntrySectionEditor: ColumnLayout {
        id: sectionEditor

        required property string sectionLabel
        required property var targetList
        property bool first

        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: sectionEditor.first
            text: sectionEditor.sectionLabel
        }

        ListEditor {
            function labelFor(item: var): string {
                const prettyName = root.builtinComponents[item.id];
                if (prettyName)
                    return prettyName;
                const label = item.id.replace(/([A-Z])/g, " $1");
                return label.charAt(0).toUpperCase() + label.slice(1).toLowerCase();
            }

            function toggledFor(item: var): bool {
                return item.enabled;
            }

            z: 1
            first: true
            values: sectionEditor.targetList.values
            onItemMoved: (from, to) => {
                sectionEditor.targetList.move(from, to);
                root.targetConfig.save();
            }
            onItemRemoved: index => {
                sectionEditor.targetList.remove(index);
                root.targetConfig.save();
            }
            onItemToggled: (index, checked) => {
                sectionEditor.targetList.at(index).enabled = checked;
                root.targetConfig.save();
            }
        }

        DialogSelectButton {
            rootParent: root.flickable
            icon: "add"
            label: qsTr("Add entry")
            header: qsTr("Add new entry")
            acceptLabel: qsTr("Add")

            model: {
                const builtins = Object.keys(root.builtinComponents).map(k => ({
                            id: k,
                            label: root.builtinComponents[k]
                        }));
                return builtins;
            }

            onAccepted: {
                if (!selectedItem)
                    return;

                sectionEditor.targetList.insert({
                    id: selectedItem,
                    enabled: true
                });
                root.targetConfig.save();
            }
        }
    }
}
