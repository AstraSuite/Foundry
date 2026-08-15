import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root
    title: qsTr("Background visualiser")
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
            Layout.fillWidth: true
            text: qsTr("Enable background visualiser")
            configNode: root.targetConfig.background.visualiser
            propertyName: "enabled"
            checked: root.targetConfig.background.visualiser.enabled
            onToggled: {
                root.targetConfig.background.visualiser.enabled = checked;
                root.targetConfig.save();
            }
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            text: qsTr("Auto-hide visualiser")
            subtext: qsTr("Hide visualiser when a window is open")
            configNode: root.targetConfig.background.visualiser
            propertyName: "autoHide"
            checked: root.targetConfig.background.visualiser.autoHide
            onToggled: {
                root.targetConfig.background.visualiser.autoHide = checked;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.visualiser.enabled
        }

        ToggleRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            text: qsTr("Blur background")
            subtext: qsTr("Blur the wallpaper behind the visualiser")
            configNode: root.targetConfig.background.visualiser
            propertyName: "blur"
            checked: root.targetConfig.background.visualiser.blur
            onToggled: {
                root.targetConfig.background.visualiser.blur = checked;
                root.targetConfig.save();
            }
            enabled: root.targetConfig.background.visualiser.enabled
        }

        SectionHeader {
            text: qsTr("Appearance")
        }

        SliderRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Rounding")
            configNode: root.targetConfig.background.visualiser
            propertyName: "rounding"
            value: root.targetConfig.background.visualiser.rounding / 10.0
            valueLabel: (value * 10.0).toFixed(1)
            onMoved: v => {
                root.targetConfig.background.visualiser.rounding = v * 10.0;
                root.targetConfig.save();
            }
        }

        SliderRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Spacing")
            configNode: root.targetConfig.background.visualiser
            propertyName: "spacing"
            value: root.targetConfig.background.visualiser.spacing / 10.0
            valueLabel: (value * 10.0).toFixed(1)
            onMoved: v => {
                root.targetConfig.background.visualiser.spacing = v * 10.0;
                root.targetConfig.save();
            }
        }

        StepperRow {
            Layout.topMargin: Tokens.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Visualiser bars")
            subtext: qsTr("Number of bars in the audio visualisers")
            configNode: root.targetConfig.services
            propertyName: "visualiserBars"
            value: root.targetConfig.services.visualiserBars
            from: 10
            to: 120
            stepSize: 2
            onMoved: v => {
                root.targetConfig.services.visualiserBars = v;
                root.targetConfig.save();
            }
        }
    }
}
