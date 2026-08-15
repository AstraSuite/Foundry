pragma Singleton

import QtQuick
import AstraMarket.Config

QtObject {
    id: root

    property bool enabled
    property date enabledSince

    onEnabledChanged: {
        if (enabled)
            enabledSince = new Date();
    }
}
