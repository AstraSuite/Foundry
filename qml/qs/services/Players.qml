pragma Singleton

import QtQml
import Quickshell
import Quickshell.Io
import Quickshell.Services.Mpris
import Caelestia
import AstraMarket.Config
import qs.components.misc

Singleton {
    id: root

    readonly property list<MprisPlayer> list: Mpris.players.values
    readonly property MprisPlayer active: props.manualActive ?? list.find(p => getIdentity(p) === GlobalConfig.services.defaultPlayer) ?? list[0] ?? null
    property alias manualActive: props.manualActive

    readonly property list<string> appVolumePlayers: [ "Spotify" ]

    property string lastNowPlayingKey: ""

    function getIdentity(player: MprisPlayer): string {
        if (!player)
            return "";
        const alias = GlobalConfig.services.playerAliases.find(a => a.from === player.identity);
        return alias?.to ?? player.identity;
    }

    function supportsAppVolume(player: MprisPlayer): bool {
        if (!player)
            return false;
        const identity = root.getIdentity(player).toLowerCase();
        return root.appVolumePlayers.some(id => identity.includes(id.toLowerCase()));
    }

    function getArtUrl(player: MprisPlayer): string {
        if (!player)
            return "";
        if (player.trackArtUrl)
            return player.trackArtUrl;

        const url = player.metadata["xesam:url"] ?? "";
        if (url.startsWith("https://www.youtube.com/watch")) {

            const id = url.match(/[?&]v=([\w-]{11})/)?.[1];
            return id ? `https:
        }
        return "";
    }

    function maybeToastNowPlaying(): void {
        if (!GlobalConfig.utilities.toasts.nowPlaying)
            return;

        const player = root.active;
        if (!player)
            return;

        const title = player.trackTitle ?? "";
        const artist = player.trackArtist ?? "";
        if (!title || !artist)
            return;

        const key = `${getIdentity(player)}\0${player.uniqueId}\0${title}\0${artist}`;
        if (key === lastNowPlayingKey)
            return;

        lastNowPlayingKey = key;
        Toaster.toast(qsTr("Now Playing"), qsTr("%1 - %2").arg(artist).arg(title), "music_note");
    }

    onActiveChanged: lastNowPlayingKey = ""

    Connections {
        function onPostTrackChanged(): void {
            root.maybeToastNowPlaying();
        }

        function onTrackTitleChanged(): void {
            root.maybeToastNowPlaying();
        }

        function onTrackArtistChanged(): void {
            root.maybeToastNowPlaying();
        }

        target: root.active
    }

    PersistentProperties {
        id: props

        property MprisPlayer manualActive

        reloadableId: "players"
    }

    CustomShortcut {

        name: "mediaToggle"
        description: "Toggle media playback"
        onPressed: {
            const active = root.active;
            if (active && active.canTogglePlaying)
                active.togglePlaying();
        }
    }

    CustomShortcut {

        name: "mediaPrev"
        description: "Previous track"
        onPressed: {
            const active = root.active;
            if (active && active.canGoPrevious)
                active.previous();
        }
    }

    CustomShortcut {

        name: "mediaNext"
        description: "Next track"
        onPressed: {
            const active = root.active;
            if (active && active.canGoNext)
                active.next();
        }
    }

    CustomShortcut {

        name: "mediaStop"
        description: "Stop media playback"
        onPressed: root.active?.stop()
    }

    IpcHandler {
        function getActive(prop: string): string {
            const active = root.active;
            return active ? active[prop] ?? "Invalid property" : "No active player";
        }

        function list(): string {
            return root.list.map(p => root.getIdentity(p)).join("\n");
        }

        function play(): void {
            const active = root.active;
            if (active?.canPlay)
                active.play();
        }

        function pause(): void {
            const active = root.active;
            if (active?.canPause)
                active.pause();
        }

        function playPause(): void {
            const active = root.active;
            if (active?.canTogglePlaying)
                active.togglePlaying();
        }

        function previous(): void {
            const active = root.active;
            if (active?.canGoPrevious)
                active.previous();
        }

        function next(): void {
            const active = root.active;
            if (active?.canGoNext)
                active.next();
        }

        function stop(): void {
            root.active?.stop();
        }

        target: "mpris"
    }
}
