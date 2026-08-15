import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components.controls
import qs.modules.astra.common

PageBase {
    id: root

    readonly property list<MenuItem> notifFullscreenItems: [
        MenuItem {
            text: qsTr("Off")
            icon: "notifications_off"
        },
        MenuItem {
            text: qsTr("On")
            icon: "notifications"
        }
    ]
    readonly property list<string> notifFullscreenValues: ["off", "on"]

    readonly property list<MenuItem> toastFullscreenItems: [
        MenuItem {
            text: qsTr("Off")
            icon: "notifications_off"
        },
        MenuItem {
            text: qsTr("Important")
            icon: "priority_high"
        },
        MenuItem {
            text: qsTr("On")
            icon: "notifications"
        }
    ]
    readonly property list<string> toastFullscreenValues: ["off", "important", "all"]

    title: qsTr("Notifications")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Notifications")
        }

        SelectRow {
            first: true
            label: qsTr("Show in fullscreen")
            subtext: qsTr("Whether notifications appear over fullscreen apps")
            configNode: root.targetConfig.notifs
            propertyName: "fullscreen"
            menuItems: root.notifFullscreenItems
            active: root.notifFullscreenItems[Math.max(0, root.notifFullscreenValues.indexOf(root.targetConfig.notifs.fullscreen))]
            onSelected: item => {
                root.targetConfig.notifs.fullscreen = root.notifFullscreenValues[root.notifFullscreenItems.indexOf(item)];
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Expire automatically")
            subtext: qsTr("Dismiss notifications after their timeout")
            configNode: root.targetConfig.notifs
            propertyName: "expire"
            checked: root.targetConfig.notifs.expire
            onToggled: {
                root.targetConfig.notifs.expire = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Open expanded")
            subtext: qsTr("Show notifications expanded by default")
            configNode: root.targetConfig.notifs
            propertyName: "openExpanded"
            checked: root.targetConfig.notifs.openExpanded
            onToggled: {
                root.targetConfig.notifs.openExpanded = checked;
                root.targetConfig.save();
            }
        }

        StepperRow {
            label: qsTr("Default timeout")
            subtext: qsTr("Time before a notification dismisses (ms)")
            configNode: root.targetConfig.notifs
            propertyName: "defaultExpireTimeout"
            value: root.targetConfig.notifs.defaultExpireTimeout
            from: 1000
            to: 60000
            stepSize: 500
            onMoved: v => {
                root.targetConfig.notifs.defaultExpireTimeout = Math.round(v);
                root.targetConfig.save();
            }
        }

        StepperRow {
            last: true
            label: qsTr("Group preview count")
            subtext: qsTr("Notifications shown per group before collapsing")
            configNode: root.targetConfig.notifs
            propertyName: "groupPreviewNum"
            value: root.targetConfig.notifs.groupPreviewNum
            from: 1
            to: 10
            stepSize: 1
            onMoved: v => {
                root.targetConfig.notifs.groupPreviewNum = Math.round(v);
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Toasts")
        }

        SelectRow {
            first: true
            label: qsTr("Show in fullscreen")
            subtext: qsTr("Whether toasts appear over fullscreen apps")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "fullscreen"
            menuItems: root.toastFullscreenItems
            active: root.toastFullscreenItems[Math.max(0, root.toastFullscreenValues.indexOf(root.targetConfig.utilities.toasts.fullscreen))]
            onSelected: item => {
                root.targetConfig.utilities.toasts.fullscreen = root.toastFullscreenValues[root.toastFullscreenItems.indexOf(item)];
                root.targetConfig.save();
            }
        }

        StepperRow {
            label: qsTr("Visible toasts")
            subtext: qsTr("Maximum number of toasts shown at once")
            configNode: root.targetConfig.utilities
            propertyName: "maxToasts"
            value: root.targetConfig.utilities.maxToasts
            from: 1
            to: 10
            stepSize: 1
            onMoved: v => {
                root.targetConfig.utilities.maxToasts = Math.round(v);
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Toast transparency")
            subtext: qsTr("Apply transparency and blur to toast notifications")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "transparency"
            checked: root.targetConfig.utilities.toasts.transparency
            onToggled: {
                root.targetConfig.utilities.toasts.transparency = checked;
                root.targetConfig.save();
            }
        }

        SliderRow {
            last: true
            label: qsTr("Base transparency")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "transparencyBase"
            valueLabel: Math.round(value * 100) + "%"
            value: root.targetConfig.utilities.toasts.transparencyBase
            enabled: root.targetConfig.utilities.toasts.transparency
            onMoved: v => {
                root.targetConfig.utilities.toasts.transparencyBase = v;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Toast events")
        }

        ToggleRow {
            first: true
            text: qsTr("Charging changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "chargingChanged"
            checked: root.targetConfig.utilities.toasts.chargingChanged
            onToggled: {
                root.targetConfig.utilities.toasts.chargingChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Game mode changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "gameModeChanged"
            checked: root.targetConfig.utilities.toasts.gameModeChanged
            onToggled: {
                root.targetConfig.utilities.toasts.gameModeChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Do not disturb changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "dndChanged"
            checked: root.targetConfig.utilities.toasts.dndChanged
            onToggled: {
                root.targetConfig.utilities.toasts.dndChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Audio output changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "audioOutputChanged"
            checked: root.targetConfig.utilities.toasts.audioOutputChanged
            onToggled: {
                root.targetConfig.utilities.toasts.audioOutputChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Audio input changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "audioInputChanged"
            checked: root.targetConfig.utilities.toasts.audioInputChanged
            onToggled: {
                root.targetConfig.utilities.toasts.audioInputChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Caps lock changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "capsLockChanged"
            checked: root.targetConfig.utilities.toasts.capsLockChanged
            onToggled: {
                root.targetConfig.utilities.toasts.capsLockChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Num lock changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "numLockChanged"
            checked: root.targetConfig.utilities.toasts.numLockChanged
            onToggled: {
                root.targetConfig.utilities.toasts.numLockChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Keyboard layout changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "kbLayoutChanged"
            checked: root.targetConfig.utilities.toasts.kbLayoutChanged
            onToggled: {
                root.targetConfig.utilities.toasts.kbLayoutChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("VPN changes")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "vpnChanged"
            checked: root.targetConfig.utilities.toasts.vpnChanged
            onToggled: {
                root.targetConfig.utilities.toasts.vpnChanged = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            last: true
            text: qsTr("Now playing")
            configNode: root.targetConfig.utilities.toasts
            propertyName: "nowPlaying"
            checked: root.targetConfig.utilities.toasts.nowPlaying
            onToggled: {
                root.targetConfig.utilities.toasts.nowPlaying = checked;
                root.targetConfig.save();
            }
        }
    }
}
