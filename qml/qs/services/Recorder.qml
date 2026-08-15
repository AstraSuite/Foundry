pragma Singleton

import QtQuick
import Quickshell
import Quickshell.Io
import qs.services

Singleton {
    id: root

    readonly property alias running: props.running
    readonly property alias paused: props.paused
    readonly property alias elapsed: props.elapsed
    property bool needsStart
    property list<string> startArgs
    property bool needsStop
    property bool needsPause

    function start(extraArgs = []): void {
        needsStart = true;
        startArgs = extraArgs;
        checkProc.running = true;
    }

    function stop(): void {
        needsStop = true;
        checkProc.running = true;
    }

    function togglePause(): void {
        needsPause = true;
        checkProc.running = true;
    }

    PersistentProperties {
        id: props

        property bool running: false
        property bool paused: false
        property real elapsed: 0

        reloadableId: "recorder"
    }

    Process {
        id: checkProc

        running: true
        command: ["pidof", "gpu-screen-recorder"]
        onExited: code => {
            props.running = code === 0;

            if (code === 0) {
                if (root.needsStop) {
                    Quickshell.execDetached(["caelestia", "record"]);
                    props.running = false;
                    props.paused = false;
                    Audio.playVideoStop();
                } else if (root.needsPause) {
                    Quickshell.execDetached(["caelestia", "record", "-p"]);
                    props.paused = !props.paused;
                }
            } else if (root.needsStart) {
                Quickshell.execDetached(["caelestia", "record", ...root.startArgs]);
                props.running = true;
                props.paused = false;
                props.elapsed = 0;
                Audio.playVideoRecord();
            }

            root.needsStart = false;
            root.needsStop = false;
            root.needsPause = false;
        }
    }

    Connections {

        function onSecondsChanged(): void {
            props.elapsed++;
        }

        target: Time
    }
}
