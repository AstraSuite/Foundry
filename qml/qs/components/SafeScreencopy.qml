import QtQuick
import Quickshell
import Quickshell.Wayland

Item {
    id: root

    property var captureSource: null
    property bool live: false
    property bool smooth: false
    property size constraintSize: Qt.size(-1, -1)
    property bool _isStable: false

    implicitWidth: view.implicitWidth
    implicitHeight: view.implicitHeight

    readonly property bool _isValidSource: {
        const src = root.captureSource;
        if (!src) return false;

        if (typeof src.width === "number" && src.width <= 0) return false;
        if (typeof src.height === "number" && src.height <= 0) return false;

        if (src.size !== undefined) {
            if (typeof src.size.width === "number" && src.size.width <= 0) return false;
            if (typeof src.size.height === "number" && src.size.height <= 0) return false;
        }

        if (src.hasBuffer !== undefined && !src.hasBuffer) return false;
        if (src.bufferSize !== undefined) {
            if (typeof src.bufferSize.width === "number" && src.bufferSize.width <= 0) return false;
            if (typeof src.bufferSize.height === "number" && src.bufferSize.height <= 0) return false;
        }

        return true;
    }

    readonly property var _effectiveSource: (root._isStable && root._isValidSource) ? root.captureSource : null

    Timer {
        id: stableTimer
        interval: 150
        repeat: false
        onTriggered: root._isStable = true
    }

    onCaptureSourceChanged: {
        root._isStable = false;
        if (root._isValidSource) {
            stableTimer.restart();
        } else {
            stableTimer.stop();
        }
    }

    on_IsValidSourceChanged: {
        if (!root._isValidSource) {
            root._isStable = false;
            stableTimer.stop();
        } else if (root.captureSource && !root._isStable) {
            stableTimer.restart();
        }
    }

    ScreencopyView {
        id: view
        anchors.fill: parent
        captureSource: root._effectiveSource
        live: root.live && root._effectiveSource !== null
        smooth: root.smooth
        constraintSize: root.constraintSize
    }
}
