import QtQuick
import QtQuick.Layouts
import qs.modules.astra.common

PageBase {
    title: qsTr("Wallhaven")
    isSubPage: true
    scrollable: false

    WallhavenTab {
        anchors.fill: parent
    }
}
