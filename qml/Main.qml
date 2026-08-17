import QtQuick
import QtQuick.Window
import qs.services
import qs.modules.astra
import AstraMarket.Market 1.0
import AstraMarket.Tray 1.0

Window {
    id: window
    visible: (typeof startInTray !== "undefined" && startInTray) ? false : true
    width: 1080
    height: 600
    minimumWidth: 800
    minimumHeight: 500
    title: "AstraMarket"
    color: Colours.palette.m3surface

    onClosing: (close) => {
        if (TrayManager.closeToTray && TrayManager.trayEnabled) {
            close.accepted = false;
            window.hide();
        }
    }

    Connections {
        target: TrayManager

        function onRequestShowMainWindow(): void {
            window.show();
            window.raise();
            window.requestActivate();
        }

        function onRequestToggleMainWindow(): void {
            if (window.visible) {
                window.hide();
            } else {
                window.show();
                window.raise();
                window.requestActivate();
            }
        }

        function onRequestShowUpdatesPage(): void {
            window.show();
            window.raise();
            window.requestActivate();
            if (astraUi.nState) {
                astraUi.nState.currentPageIdx = 3;
            }
        }

        function onRequestShowSettingsPage(): void {
            window.show();
            window.raise();
            window.requestActivate();
            if (astraUi.nState) {
                astraUi.nState.currentPageIdx = 5;
            }
        }
    }

    Astra {
        id: astraUi
        anchors.fill: parent
        anchors.margins: 0

        onClose: {
            if (TrayManager.closeToTray && TrayManager.trayEnabled) {
                window.hide();
            } else {
                window.close();
                Qt.quit();
            }
        }
    }

    DropArea {
        anchors.fill: parent
        keys: ["text/uri-list"]

        onDropped: drop => {
            if (drop.hasUrls) {
                for (var i = 0; i < drop.urls.length; i++) {
                    var url = drop.urls[i].toString();
                    if (url.toLowerCase().endsWith(".appimage")) {
                        AppImageInstaller.installAppImage(url);
                    }
                }
            }
        }
    }
}
