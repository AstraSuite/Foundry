import "navpane"
import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.services
import qs.modules.astra

ColumnLayout {
    id: root

    required property AstraState nState

    spacing: Tokens.spacing.large

    NavLocations {
        z: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        nState: root.nState
    }
}
