import QtQuick
import QtMultimedia
import Quickshell
import Quickshell.Widgets
import AstraMarket.Config
import qs.services

Item {
    id: root

    property string path
    property var screen
    property bool isFirstInstance: false

    property alias playing: mediaPlayer.playing
    property alias playbackState: mediaPlayer.playbackState

    readonly property bool shouldPause: {
        if (GlobalConfig.background.videoWallpaperPaused)
            return true;

        const pauseOnFullscreen = GlobalConfig.background.videoWallpaperPauseOnFullscreen;
        const pauseOnTiled = GlobalConfig.background.videoWallpaperPauseOnTiled;

        if (!pauseOnFullscreen && !pauseOnTiled)
            return false;

        const checkToplevels = toplevels => {
            if (pauseOnFullscreen && toplevels.some(t => t?.lastIpcObject?.fullscreen > 1))
                return true;
            if (pauseOnTiled && toplevels.some(t => !t?.lastIpcObject?.floating && !t?.lastIpcObject?.fullscreen))
                return true;
            return false;
        };

        if (GlobalConfig.background.videoWallpaperPauseOnAllDisplays) {
            return Hypr.monitors.values.some(monitor => checkToplevels(monitor?.activeWorkspace?.toplevels?.values || []));
        }

        if (!root.screen)
            return false;

        const monitor = Hypr.monitorFor(root.screen);
        if (!monitor)
            return false;

        return checkToplevels(monitor.activeWorkspace?.toplevels?.values || []);
    }

    onShouldPauseChanged: applyPauseState()

    function applyPauseState(): void {
        if (root.shouldPause) {
            if (mediaPlayer.playing)
                mediaPlayer.pause();
        } else if (root.path && !mediaPlayer.playing) {
            mediaPlayer.play();
        }
    }

    readonly property bool shouldMute: !root.isFirstInstance || !GlobalConfig.background.videoWallpaperSoundEnabled || (GlobalConfig.background.videoWallpaperMuteOnMedia && (Players.active?.isPlaying ?? false))

    Component.onCompleted: {
        isFirstInstance = (VideoWallpaperPlayer.firstInstance === null);
        VideoWallpaperPlayer.firstInstance = root;
        Qt.callLater(applyPauseState);
    }

    Component.onDestruction: {
        if (VideoWallpaperPlayer.firstInstance === root) {
            VideoWallpaperPlayer.firstInstance = null;
        }
    }

    onPathChanged: {
        mediaPlayer.source = path || "";
        if (path && !root.shouldPause)
            mediaPlayer.play();
    }

    AudioOutput {
        id: audioOutput
        muted: root.shouldMute
    }

    MediaPlayer {
        id: mediaPlayer

        source: path || ""
        videoOutput: videoOutput
        loops: MediaPlayer.Infinite
        autoPlay: true
        audioOutput: audioOutput
    }

    VideoOutput {
        id: videoOutput

        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop

        Component.onDestruction: {
            mediaPlayer.stop();
        }
    }
}
