pragma ComponentBehavior: Bound

import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    function isToggleOn(id: string): bool {
        const item = Config.utilities.quickToggles.values.find(t => t.id === id);
        return item?.enabled ?? false;
    }

    function setToggleOn(id: string, on: bool): void {
        const list = GlobalConfig.utilities.quickToggles;
        for (let i = 0; i < list.count; i++) {
            const item = list.at(i);
            if (item.id === id) {
                item.enabled = on;
                return;
            }
        }
        list.insert({
            id,
            enabled: on
        });
    }

    title: qsTr("Utilities")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("General")
        }

        ToggleRow {
            first: true
            last: true
            text: qsTr("Enabled")
            subtext: qsTr("Show the utilities panel")
            configNode: root.targetConfig.utilities
            propertyName: "enabled"
            checked: root.targetConfig.utilities.enabled
            onToggled: {
                root.targetConfig.utilities.enabled = checked;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Cards")
        }

        ToggleRow {
            first: true
            text: qsTr("Keep awake")
            subtext: qsTr("Show the idle inhibitor card")
            configNode: root.targetConfig.utilities.cards
            propertyName: "keepAwake"
            checked: root.targetConfig.utilities.cards.keepAwake
            onToggled: {
                root.targetConfig.utilities.cards.keepAwake = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Screen recorder")
            subtext: qsTr("Show the screen recorder card")
            configNode: root.targetConfig.utilities.cards
            propertyName: "recorder"
            checked: root.targetConfig.utilities.cards.recorder
            onToggled: {
                root.targetConfig.utilities.cards.recorder = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            last: true
            text: qsTr("Quick toggles")
            subtext: qsTr("Show the quick toggles card")
            configNode: root.targetConfig.utilities.cards
            propertyName: "quickToggles"
            checked: root.targetConfig.utilities.cards.quickToggles
            onToggled: {
                root.targetConfig.utilities.cards.quickToggles = checked;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Quick toggles")
        }

        ToggleRow {
            first: true
            text: qsTr("Wi-Fi")
            subtext: qsTr("Toggle wireless networking")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("wifi")
            onToggled: root.setToggleOn("wifi", checked)
        }

        ToggleRow {
            text: qsTr("Bluetooth")
            subtext: qsTr("Toggle the Bluetooth adapter")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("bluetooth")
            onToggled: root.setToggleOn("bluetooth", checked)
        }

        ToggleRow {
            text: qsTr("Microphone")
            subtext: qsTr("Mute or unmute the default source")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("mic")
            onToggled: root.setToggleOn("mic", checked)
        }

        ToggleRow {
            text: qsTr("Settings")
            subtext: qsTr("Open the settings window")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("settings")
            onToggled: root.setToggleOn("settings", checked)
        }

        ToggleRow {
            text: qsTr("Game mode")
            subtext: qsTr("Toggle game mode")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("gameMode")
            onToggled: root.setToggleOn("gameMode", checked)
        }

        ToggleRow {
            text: qsTr("Do not disturb")
            subtext: qsTr("Silence notifications")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("dnd")
            onToggled: root.setToggleOn("dnd", checked)
        }

        ToggleRow {
            text: qsTr("VPN")
            subtext: qsTr("Connect or disconnect the VPN")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("vpn")
            onToggled: root.setToggleOn("vpn", checked)
        }

        ToggleRow {
            text: qsTr("Quick Share")
            subtext: qsTr("Send and receive files nearby")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("quickshare")
            onToggled: root.setToggleOn("quickshare", checked)
        }

        ToggleRow {
            text: qsTr("Wallpaper selector")
            subtext: qsTr("Show the wallpaper picker menu")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("wallpaper")
            onToggled: root.setToggleOn("wallpaper", checked)
        }

        ToggleRow {
            text: qsTr("Bad Apple")
            subtext: qsTr("Play the Bad Apple animation")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("badapple")
            onToggled: root.setToggleOn("badapple", checked)
        }

        ToggleRow {
            text: qsTr("Pause video wallpaper")
            subtext: qsTr("Pause or resume the video wallpaper")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("pauseWallpaper")
            onToggled: root.setToggleOn("pauseWallpaper", checked)
        }

        ToggleRow {
            last: true
            text: qsTr("Pause PiP")
            subtext: qsTr("Pause or resume picture-in-picture")
            disabled: !Config.utilities.cards.quickToggles
            checked: root.isToggleOn("pipPause")
            onToggled: root.setToggleOn("pipPause", checked)
        }
    }
}
