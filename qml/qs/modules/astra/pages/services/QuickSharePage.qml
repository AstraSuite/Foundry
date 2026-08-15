import QtQuick
import QtQuick.Layouts
import qs.components.controls
import qs.modules.astra.common
import qs.services
import AstraMarket.Config

PageBase {
    id: root

    title: qsTr("Quick Share")

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Startup")
        }

        ToggleRow {
            first: true
            last: true
            text: qsTr("Auto-start on launch")
            subtext: qsTr("Enable Quick Share when the shell starts")
            configNode: root.targetConfig.services
            propertyName: "quickShareAutoStart"
            checked: root.targetConfig.services.quickShareAutoStart
            onToggled: {
                root.targetConfig.services.quickShareAutoStart = checked;
                root.targetConfig.save();
            }
        }

        SectionHeader {
            text: qsTr("Status")
        }

        ToggleRow {
            first: true
            last: true
            text: qsTr("Enable Quick Share")
            subtext: QuickShare.isEnabled ? qsTr("Discoverable by nearby devices") : qsTr("Disabled")
            checked: QuickShare.isEnabled
            onToggled: {
                QuickShare.setEnabled(checked);
                QuickShare.setVisible(checked);
            }
        }
    }
}
