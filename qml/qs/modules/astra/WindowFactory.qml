pragma Singleton

import QtQuick
import QtQuick.Window
import AstraMarket.Config
import qs.components
import qs.services
import qs.modules.astra

Item {
    id: root

    function create(parent: Item, props: var): void {
        astraComp.createObject(parent ?? dummy, props);
    }

    QtObject {
        id: dummy
    }

    Component {
        id: astraComp

        Window {
            id: win
            visible: true

            color: Colours.tPalette.m3surface

            onVisibleChanged: {
                if (!visible)
                    destroy();
            }

            width: astra.implicitWidth
            height: astra.implicitHeight

            minimumWidth: 800
            minimumHeight: 600

            title: qsTr("Astra — %1").arg(PageRegistry.pages[astra.nState.currentPageIdx]?.label || "")

            Astra {
                id: astra

                anchors.fill: parent
                nState.isWindow: true
                onClose: win.destroy()
            }
        }
    }
}
