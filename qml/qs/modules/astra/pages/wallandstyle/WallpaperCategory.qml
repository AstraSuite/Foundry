pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import Caelestia.Models
import qs.services
import qs.utils
import qs.modules.astra.common

PageBase {
    id: root

    title: {
        const c = nState.selectedWallpaperCategory;
        return c.slice(0, 1).toUpperCase() + c.slice(1);
    }
    isSubPage: true

    GridLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth

        columns: Config.astra.wallpapersPerRow
        rowSpacing: Tokens.spacing.medium
        columnSpacing: Tokens.spacing.large

        Repeater {
            model: {
                const category = root.nState ? root.nState.selectedWallpaperCategory : "";
                let walls = Wallpapers.list.filter(w => Wallpapers.getCategoryFor(w) === category);
                const filter = root.nState ? root.nState.wallpaperFilterType : "all";

                walls = walls.filter(w => {
                    const isVid = Images.isVideo(w.name);
                    const isGif = w.name.toLowerCase().endsWith(".gif");
                    const isImg = Images.isValidImageByName(w.name) && !isGif;

                    if (filter === "all") return true;
                    if (filter === "video") return isVid;
                    if (filter === "gif") return isGif;
                    if (filter === "image") return isImg;
                    return false;
                });

                walls.sort((a, b) => a.name.localeCompare(b.name));
                while (walls.length < Config.astra.wallpapersPerRow)
                    walls.push(null);
                return walls;
            }

            WallItem {
                required property FileSystemEntry modelData

                opacity: modelData ? 1 : 0
                enabled: modelData

                source: modelData ? Wallpapers.getThumbnailPath(modelData.path) : ""
                realPath: modelData ? modelData.path : ""
                text: modelData?.name ?? ""
                onClicked: {
                    Wallpapers.setWallpaper(modelData.path);
                }
            }
        }
    }
}
