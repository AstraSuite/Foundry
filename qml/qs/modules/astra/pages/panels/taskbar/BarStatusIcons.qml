pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.services
import qs.modules.astra.common

PageBase {
    id: root

    readonly property var builtinIcons: ({
            lockStatus: qsTr("Lock keys"),
            kbLayout: qsTr("Keyboard layout"),
            audio: qsTr("Speakers"),
            microphone: qsTr("Microphone"),
            network: qsTr("Network"),
            bluetooth: qsTr("Bluetooth"),
            battery: qsTr("Battery"),
            peripheralBattery: qsTr("Peripheral battery"),
            notifications: qsTr("Notifications")
        })

    title: qsTr("Status icons")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Tokens.padding.small

            StyledText {
                text: qsTr("Visible icons")
                color: Colours.palette.m3onSurfaceVariant
                font: Tokens.font.label.medium
                elide: Text.ElideRight
            }

            PerMonitorStatusChip {
                configNode: root.targetConfig.bar
                propertyName: "statusIcons"
            }

            Item {
                Layout.fillWidth: true
            }
        }

        ListEditor {
            function labelFor(item: var): string {
                const prettyName = root.builtinIcons[item.id];
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
            values: root.targetConfig.bar.statusIcons.values
            onItemMoved: (from, to) => {
                root.targetConfig.bar.statusIcons.move(from, to);
                root.targetConfig.save();
            }
            onItemRemoved: index => {
                root.targetConfig.bar.statusIcons.remove(index);
                root.targetConfig.save();
            }
            onItemToggled: (index, checked) => {
                root.targetConfig.bar.statusIcons.at(index).enabled = checked;
                root.targetConfig.save();
            }
        }

        DialogSelectButton {
            id: addItemContainer

            rootParent: root.flickable
            icon: "add"
            label: qsTr("Add entry")
            header: qsTr("Add new entry")
            acceptLabel: qsTr("Add")

            model: {
                const builtins = Object.keys(root.builtinIcons).map(k => ({
                            id: k,
                            label: root.builtinIcons[k]
                        }));
                return builtins;
            }

            onAccepted: {
                if (!selectedItem)
                    return;

                root.targetConfig.bar.statusIcons.insert({
                    id: selectedItem,
                    enabled: true
                });
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Behaviour")
        }

        ToggleRow {
            first: true
            last: true
            text: qsTr("Popout on hover")
            subtext: qsTr("Show a details popout when hovering the status icons")
            configNode: root.targetConfig.bar.popouts
            propertyName: "statusIcons"
            checked: root.targetConfig.bar.popouts.statusIcons
            onToggled: {
                root.targetConfig.bar.popouts.statusIcons = checked;
                root.targetConfig.save();
            }
        }
    }
}
