pragma ComponentBehavior: Bound

import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Spotify")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Configuration")
        }

        ToggleRow {
            first: true
            text: qsTr("Background")
            subtext: qsTr("Render a solid background behind the Spotify widget")
            configNode: root.targetConfig.bar.spotify
            propertyName: "background"
            checked: root.targetConfig.bar.spotify.background
            onToggled: {
                root.targetConfig.bar.spotify.background = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Show visualizer")
            subtext: qsTr("Display animated frequency visualizer bars")
            configNode: root.targetConfig.bar.spotify
            propertyName: "showVisualiser"
            checked: root.targetConfig.bar.spotify.showVisualiser
            onToggled: {
                root.targetConfig.bar.spotify.showVisualiser = checked;
                root.targetConfig.save();
            }
        }

        StepperRow {
            label: qsTr("Max title length")
            subtext: qsTr("Cut off character count for track title")
            configNode: root.targetConfig.bar.spotify
            propertyName: "maxTitleLength"
            value: root.targetConfig.bar.spotify.maxTitleLength
            from: 5
            to: 100
            stepSize: 1
            onMoved: v => {
                root.targetConfig.bar.spotify.maxTitleLength = v;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Inverted text direction")
            subtext: qsTr("Rotate text in the opposite direction when the bar is vertical")
            configNode: root.targetConfig.bar.spotify
            propertyName: "inverted"
            checked: root.targetConfig.bar.spotify.inverted
            onToggled: {
                root.targetConfig.bar.spotify.inverted = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            text: qsTr("Auto-hide")
            subtext: qsTr("Hide the widget when there is no media source available")
            configNode: root.targetConfig.bar.spotify
            propertyName: "autoHide"
            checked: root.targetConfig.bar.spotify.autoHide
            onToggled: {
                root.targetConfig.bar.spotify.autoHide = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            last: true
            text: qsTr("Horizontal volume slider")
            subtext: qsTr("Place a horizontal volume slider below the playback controls in the popout")
            configNode: root.targetConfig.bar.spotify
            propertyName: "horizontalVolume"
            checked: root.targetConfig.bar.spotify.horizontalVolume
            onToggled: {
                root.targetConfig.bar.spotify.horizontalVolume = checked;
                root.targetConfig.save();
            }
        }
    }
}
