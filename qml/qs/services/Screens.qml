pragma Singleton

import QtQuick
import AstraMarket.Config

QtObject {
    id: root

    readonly property list<var> screens: []

    function isExcluded(screen: var): bool {
        return false;
    }
}
