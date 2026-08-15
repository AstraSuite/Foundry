pragma Singleton

import QtQuick
import Quickshell
import qs.utils
import Caelestia

Singleton {
    id: root

    property var events: []
    property bool loaded: false
    property bool loading: false

    function reload() {
        if (loading) return;
        loading = true;

        Requests.get("https://pastafariancalendar.com/holidays4.json", text => {
            loading = false;
            try {
                const data = JSON.parse(text);
                if (Array.isArray(data)) {
                    events = data;
                    loaded = true;
                    retryTimer.stop();
                } else {
                    console.error("Pastafarian calendar response is not an array");
                    retryTimer.restart();
                }
            } catch (e) {
                console.error("Failed to parse Pastafarian calendar events:", e);
                retryTimer.restart();
            }
        }, err => {
            loading = false;
            console.warn("Failed to fetch Pastafarian calendar events:", err);
            retryTimer.restart();
        });
    }

    Connections {
        target: Nmcli
        function onIsConnectedChanged() {
            if (Nmcli.isConnected && !root.loaded) {
                root.reload();
            }
        }
    }

    Timer {
        id: retryTimer

        interval: 30000
        repeat: true

        onTriggered: root.reload()
    }
}
