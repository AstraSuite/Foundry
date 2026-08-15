pragma Singleton

import QtQuick
import QtCore
import Quickshell
import Quickshell.Hyprland
import Quickshell.Services.UPower
import Quickshell.Io
import AstraMarket.Config

import qs.services
import qs.utils

Singleton {
    id: root

    property bool manualPause: false
    property bool pauseOnBattery: false
    property bool pauseOnWindowOverlap: true
    property string hwDecoder: "none"

    Settings {
        id: pauserSettings
        location: `${Paths.state}/wallpaper/pauser.ini`
        category: "WallpaperPauser"
        property alias manualPause: root.manualPause
        property alias pauseOnBattery: root.pauseOnBattery
        property alias pauseOnWindowOverlap: root.pauseOnWindowOverlap
        property alias hwDecoder: root.hwDecoder
    }
    property bool paused: false
    property bool _loaded: false
    property string pauseReason: "None"

    Process {
        id: saveHwDecoderProcess
    }

    function recalculate() {
        let newPaused = false;
        let reason = "None";

        if ((typeof Config !== "undefined" && Config.background && Config.background.videoWallpaperPaused) || manualPause) {
            newPaused = true;
            reason = "Manual / Config Pause";
        } else if (pauseOnBattery && UPower.onBattery) {
            newPaused = true;
            reason = "Battery";
        } else if (pauseOnWindowOverlap) {
            const monitor = Hyprland.focusedMonitor;
            const ws = monitor && monitor.activeWorkspace ? monitor.activeWorkspace : Hyprland.focusedWorkspace;

            if (ws) {

                const toplevels = ws.toplevels.values;

                if (toplevels.length >= 2) {
                    newPaused = true;
                    reason = "2+ windows (" + toplevels.length + " total)";
                } else {

                    if (monitor) {
                        const screen = Quickshell.screens.find(s => s.name === monitor.name);
                        if (screen) {
                            const screenArea = screen.width * screen.height;
                            if (screenArea > 0) {
                                const threshold = screenArea * 0.7;
                                for (const t of toplevels) {
                                    const size = t.lastIpcObject?.size;
                                    if (size && size.length >= 2 && size[0] * size[1] >= threshold) {
                                        newPaused = true;
                                        reason = "70% area rule by: " + (t.lastIpcObject?.title ?? "Unknown") + " (" + size[0] + "x" + size[1] + ")";
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        paused = newPaused;
        root.pauseReason = reason;
    }

    Connections {
        target: Hyprland
        function onFocusedWorkspaceChanged() {
            root.recalculate();
        }
        function onFocusedMonitorChanged() {
            root.recalculate();
        }
        function onRawEvent(event) {
            const n = event.name;
            if (n.startsWith("workspace") || n.startsWith("activewindow") || n.startsWith("createworkspace") || n.startsWith("destroyworkspace") || ["fullscreen", "changefloatingmode", "minimize", "movewindow", "openwindow", "closewindow", "moveworkspace", "focusedmon"].includes(n)) {
                recalcTimer.restart();
            }
        }
    }

    Connections {
        target: UPower
        function onOnBatteryChanged() {
            recalcTimer.restart();
        }
    }

    Timer {
        id: recalcTimer
        interval: 50
        onTriggered: root.recalculate()
    }

    Timer {
        id: startupTimer
        interval: 1000
        repeat: true
        running: true
        property int attempts: 0
        onTriggered: {
            root.recalculate();
            attempts++;
            if (attempts >= 5) {
                running = false;
            }
        }
    }

    onManualPauseChanged: {
        recalculate();
    }

    onPauseOnBatteryChanged: {
        recalculate();
    }

    onPauseOnWindowOverlapChanged: {
        recalculate();
    }

    onHwDecoderChanged: {

        if (root._loaded) {
            saveHwDecoderProcess.command = ["sh", "-c", "echo '" + root.hwDecoder + "' > ~/.cache/caelestia/hwDecoder.txt && nohup sh -c 'sleep 0.5 && caelestia shell -d' >/dev/null 2>&1 & caelestia shell -k"];
            saveHwDecoderProcess.running = true;
        }
    }

    Component.onCompleted: {
        root._loaded = true;
        recalculate();
    }
}
