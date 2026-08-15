pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Quickshell
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.services

Item {
    id: root

    property string source: ""
    property alias text: label.text
    property alias radius: imgWrapper.radius
    property alias imgHeight: imgWrapper.implicitHeight
    property bool fillLabel: true
    property string realPath: ""

    signal clicked

    Layout.fillWidth: true
    implicitHeight: layout.implicitHeight

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: Tokens.spacing.small

        StyledClippingRect {
            id: imgWrapper

            Layout.fillWidth: true
            implicitHeight: width
            radius: Tokens.rounding.largeIncreased
            color: Colours.tPalette.m3surfaceContainer

            Loader {
                anchors.centerIn: parent

                opacity: img.status === Image.Ready ? 0 : 1
                active: opacity > 0

                sourceComponent: StyledRect {
                    implicitWidth: loadingIndicator.implicitSize + Tokens.padding.large * 2
                    implicitHeight: loadingIndicator.implicitSize + Tokens.padding.large * 2

                    color: Colours.palette.m3primaryContainer
                    radius: Tokens.rounding.full

                    LoadingIndicator {
                        id: loadingIndicator

                        anchors.centerIn: parent
                        containsIcon: true
                        implicitSize: Math.min(imgWrapper.width, imgWrapper.height) * 0.3
                    }
                }

                Behavior on opacity {
                    Anim {
                        type: Anim.DefaultEffects
                    }
                }
            }

            Image {
                id: img

                anchors.fill: parent
                asynchronous: true
                fillMode: Image.PreserveAspectCrop

                source: {
                    const target = root.realPath || root.source;
                    if (!target) return "";

                    if (Wallpapers.isVideo(target)) {
                        const thumb = Wallpapers.getWallpaperThumb(target, Wallpapers.cacheBuster);
                        return (typeof thumb === "string" && thumb !== "undefined") ? thumb : "";
                    }
                    return root.source || target;
                }

                sourceSize: {
                    const dpr = (QsWindow.window as QsWindow)?.devicePixelRatio ?? 1;
                    return Qt.size(width * dpr, height * dpr);
                }
                retainWhileLoading: true
                opacity: status === Image.Ready ? 1 : 0

                Behavior on opacity {
                    Anim {
                        type: Anim.SlowEffects
                    }
                }
            }
        }

        StyledText {
            id: label

            Layout.bottomMargin: Tokens.padding.small
            Layout.fillWidth: true
            color: Colours.palette.m3onSurfaceVariant
            font: Tokens.font.label.builders.small.weight(Font.Medium).build()
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    StateLayer {
        anchors.bottomMargin: root.fillLabel ? 0 : layout.implicitHeight - imgWrapper.implicitHeight
        onClicked: root.clicked()
    }
}
