import QtQuick
import QtQuick.Window
import qs.services
import qs.modules.astra
import AstraMarket.Market 1.0

Window {
    id: window
    visible: true
    width: 1080
    height: 600
    minimumWidth: 800
    minimumHeight: 500
    title: "AstraMarket"
    color: Colours.palette.m3surface

    Astra {
        id: astraUi
        anchors.fill: parent
        anchors.margins: 0

        onClose: {
            window.close();
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
