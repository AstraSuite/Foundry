pragma ComponentBehavior: Bound

import QtQuick.Layouts
import AstraMarket.Config
import qs.components.controls
import qs.modules.astra.common

PageBase {
    id: root

    readonly property list<MenuItem> positionItems: [
        MenuItem {
            property string value: "top"

            text: qsTr("Top")
        },
        MenuItem {
            property string value: "bottom"

            text: qsTr("Bottom")
        },
        MenuItem {
            property string value: "left"

            text: qsTr("Left")
        },
        MenuItem {
            property string value: "right"

            text: qsTr("Right")
        }
    ]

    title: qsTr("Taskbar")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Behaviour")
        }

        ToggleRow {
            first: true
            text: qsTr("Persistent")
            subtext: qsTr("Keep the bar visible at all times")
            configNode: root.targetConfig.bar
            propertyName: "persistent"
            checked: root.targetConfig.bar.persistent
            onToggled: {
                root.targetConfig.bar.persistent = checked;
                root.targetConfig.save();
            }
        }

        SelectRow {
            Layout.fillWidth: true
            label: qsTr("Position")
            subtext: qsTr("Screen edge to place the bar on")
            configNode: root.targetConfig.bar
            propertyName: "position"
            active: {
                for (let i = 0; i < positionItems.length; i++) {
                    if (positionItems[i].value === root.targetConfig.bar.position)
                        return positionItems[i];
                }
                return positionItems[0];
            }
            menuItems: positionItems
            onSelected: item => {
                root.targetConfig.bar.position = item.value;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Show on hover")
            subtext: qsTr("Reveal the bar when the cursor reaches the screen edge")
            configNode: root.targetConfig.bar
            propertyName: "showOnHover"
            checked: root.targetConfig.bar.showOnHover
            onToggled: {
                root.targetConfig.bar.showOnHover = checked;
                root.targetConfig.save();
            }
        }

        StepperRow {
            last: true
            label: qsTr("Drag threshold")
            subtext: qsTr("Pixels dragged before the bar reveals")
            configNode: root.targetConfig.bar
            propertyName: "dragThreshold"
            value: root.targetConfig.bar.dragThreshold
            from: 0
            to: 200
            stepSize: 5
            onMoved: v => {
                root.targetConfig.bar.dragThreshold = v;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Components")
        }

        NavRow {
            first: true
            icon: "view_agenda"
            text: qsTr("Taskbar components")
            subtext: qsTr("Add, remove or reorder components")
            onClicked: root.nState.openSubPage(6)
        }

        NavRow {
            icon: "workspaces"
            text: qsTr("Workspaces")
            subtext: qsTr("Indicators, window icons")
            onClicked: root.nState.openSubPage(7)
        }

        NavRow {
            icon: "web_asset"
            text: qsTr("Active window")
            subtext: qsTr("Title display, popout")
            onClicked: root.nState.openSubPage(8)
        }

        NavRow {
            icon: "dock"
            text: qsTr("Dock")
            subtext: qsTr("Positioning, recoloring")
            onClicked: root.nState.openSubPage(12)
        }

        NavRow {
            icon: "widgets"
            text: qsTr("Tray")
            subtext: qsTr("System tray icons")
            onClicked: root.nState.openSubPage(9)
        }

        NavRow {
            icon: "signal_cellular_alt"
            text: qsTr("Status icons")
            subtext: qsTr("Visible indicators")
            onClicked: root.nState.openSubPage(10)
        }

        NavRow {
            icon: "schedule"
            text: qsTr("Clock")
            subtext: qsTr("Date, icon, background")
            onClicked: root.nState.openSubPage(11)
        }

        NavRow {
            icon: "code"
            text: qsTr("GitHub")
            subtext: qsTr("Contributions, token setup")
            onClicked: root.nState.openSubPage(13)
        }

        NavRow {
            last: true
            icon: "music_note"
            text: qsTr("Spotify")
            subtext: qsTr("Visualizer, title length, background")
            onClicked: root.nState.openSubPage(14)
        }

        SectionHeader {
            text: qsTr("Scroll actions")
        }

        ToggleRow {
            first: true
            text: qsTr("Workspaces")
            subtext: qsTr("Scroll over the workspace indicator to switch workspaces")
            checked: Config.bar.scrollActions.workspaces
            onToggled: GlobalConfig.bar.scrollActions.workspaces = checked
        }

        ToggleRow {
            text: qsTr("Volume")
            subtext: qsTr("Scroll on the top half of the bar to adjust volume")
            checked: Config.bar.scrollActions.volume
            onToggled: GlobalConfig.bar.scrollActions.volume = checked
        }

        ToggleRow {
            last: true
            text: qsTr("Brightness")
            subtext: qsTr("Scroll on the bottom half of the bar to adjust brightness")
            checked: Config.bar.scrollActions.brightness
            onToggled: GlobalConfig.bar.scrollActions.brightness = checked
        }
    }
}
