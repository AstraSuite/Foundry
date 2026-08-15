pragma Singleton

import QtQuick
import Caelestia
import AstraMarket.Config

QtObject {
    id: root

    readonly property string home: "/home/dim"
    readonly property string pictures: `${home}/Pictures`
    readonly property string videos: `${home}/Videos`

    readonly property string data: `${home}/.local/share/caelestia`
    readonly property string state: `${home}/.local/state/caelestia`
    readonly property string cache: `${home}/.cache/caelestia`
    readonly property string config: `${home}/.config/caelestia`

    readonly property string imagecache: `${cache}/imagecache`
    readonly property string notifimagecache: `${imagecache}/notifs`
    readonly property string wallsdir: `${pictures}/Wallpapers`
    readonly property string recsdir: `${videos}/Recordings`
    readonly property string libdir: "/usr/lib/caelestia"

    function toLocalFile(path: url): string {
        path = Qt.resolvedUrl(path);
        return path.toString() ? CUtils.toLocalFile(path) : "";
    }

    function absolutePath(path: string): string {
        return toLocalFile(path.replace(/~|(\$({?)HOME(}?))+/, home));
    }

    function shortenHome(path: string): string {
        return path.replace(home, "~");
    }
}
