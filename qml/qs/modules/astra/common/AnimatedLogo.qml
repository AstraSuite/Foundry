import QtQuick
import QtQuick.Shapes
import qs.components
import qs.services

Item {
    id: root

    readonly property real designSize: 1000
    property bool skipIntroAnimation: false

    property real outerProgress: 1.0
    property real ringProgress: 1.0
    property real glyphProgress: 1.0

    implicitWidth: 128
    implicitHeight: 128

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (!introAnim.running && !clickSpinAnim.running) {
                clickSpinAnim.restart();
            }
        }
    }

    SequentialAnimation {
        id: clickSpinAnim
        running: false

        ParallelAnimation {
            NumberAnimation {
                target: logo
                property: "rotation"
                from: 0
                to: 360
                duration: 700
                easing.type: Easing.InOutCubic
            }

            SequentialAnimation {
                NumberAnimation {
                    target: logo
                    property: "scale"
                    from: root.implicitWidth / root.designSize
                    to: (root.implicitWidth / root.designSize) * 1.12
                    duration: 300
                    easing.type: Easing.OutCubic
                }

                NumberAnimation {
                    target: logo
                    property: "scale"
                    from: (root.implicitWidth / root.designSize) * 1.12
                    to: root.implicitWidth / root.designSize
                    duration: 400
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.2
                }
            }

            SequentialAnimation {
                NumberAnimation {
                    target: root
                    property: "glyphProgress"
                    from: 1.0
                    to: 0.55
                    duration: 250
                    easing.type: Easing.InOutQuad
                }

                NumberAnimation {
                    target: root
                    property: "glyphProgress"
                    from: 0.55
                    to: 1.0
                    duration: 450
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.6
                }
            }
        }

        ScriptAction {
            script: logo.rotation = 0
        }
    }

    Item {
        id: logo

        implicitWidth: root.designSize
        implicitHeight: root.designSize

        anchors.centerIn: parent
        scale: root.implicitWidth / root.designSize
        transformOrigin: Item.Center

        rotation: 0.0
        opacity: 1.0

        SequentialAnimation {
            id: introAnim
            running: !root.skipIntroAnimation

            ScriptAction {
                script: {
                    root.outerProgress = 0.0;
                    root.ringProgress = 0.0;
                    root.glyphProgress = 0.0;
                    logo.rotation = -135.0;
                    logo.opacity = 0.0;
                }
            }

            ParallelAnimation {
                NumberAnimation {
                    target: logo
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 600
                    easing.type: Easing.InOutQuad
                }

                NumberAnimation {
                    target: logo
                    property: "rotation"
                    from: -135
                    to: 0
                    duration: 950
                    easing.type: Easing.OutCubic
                }

                SequentialAnimation {
                    PauseAnimation { duration: 120 }
                    NumberAnimation {
                        target: logo
                        property: "scale"
                        from: (root.implicitWidth / root.designSize) * 0.7
                        to: root.implicitWidth / root.designSize
                        duration: 830
                        easing.type: Easing.OutBack
                        easing.overshoot: 1.15
                    }
                }

                NumberAnimation {
                    target: root
                    property: "outerProgress"
                    from: 0.0
                    to: 1.0
                    duration: 950
                    easing.type: Easing.OutCubic
                }

                SequentialAnimation {
                    PauseAnimation { duration: 180 }
                    NumberAnimation {
                        target: root
                        property: "ringProgress"
                        from: 0.0
                        to: 1.0
                        duration: 800
                        easing.type: Easing.OutBack
                        easing.overshoot: 1.3
                    }
                }

                SequentialAnimation {
                    PauseAnimation { duration: 450 }
                    NumberAnimation {
                        target: root
                        property: "glyphProgress"
                        from: 0.0
                        to: 1.0
                        duration: 600
                        easing.type: Easing.OutBack
                        easing.overshoot: 1.5
                    }
                }
            }
        }

        Shape {
            id: outerSwoosh
            width: 1000
            height: 1000
            z: 1
            preferredRendererType: Shape.CurveRenderer

            opacity: Math.min(1.0, root.outerProgress * 1.8)

            transform: [
                Rotation {
                    origin.x: 500
                    origin.y: 500
                    angle: (1.0 - root.outerProgress) * -260
                }
            ]

            ShapePath {
                fillColor: Colours.palette.m3onSurface
                strokeColor: "transparent"

                PathSvg {
                    path: "M999.991 497.412C998.599 277.689 820.051 100 600 100C379.086 100 200 279.086 200 500C200 720.914 379.086 900 600 900C820.051 900 998.601 722.31 999.992 502.587L999.99 503.233C998.251 777.888 775.064 1000 500 1000C223.858 1000 0 776.142 0 500C0 223.858 223.858 0 500 0C775.279 0 998.598 222.46 999.991 497.412Z"
                }
            }
        }

        Shape {
            id: innerRing
            width: 1000
            height: 1000
            z: 2
            preferredRendererType: Shape.CurveRenderer

            opacity: Math.min(1.0, root.ringProgress * 1.8)

            transform: [
                Scale {
                    origin.x: 600
                    origin.y: 500
                    xScale: 0.5 + 0.5 * root.ringProgress
                    yScale: 0.5 + 0.5 * root.ringProgress
                },
                Rotation {
                    origin.x: 600
                    origin.y: 500
                    angle: (1.0 - root.ringProgress) * 180
                }
            ]

            ShapePath {
                fillColor: Colours.palette.m3primary
                strokeColor: "transparent"

                PathSvg {
                    path: "M600 125C807.107 125 975 292.893 975 500C975 348.122 851.878 225 700 225C548.122 225 425 348.122 425 500C425 651.878 548.122 775 700 775C851.285 775 974.038 652.838 974.994 501.778L974.992 502.425C973.688 708.416 806.298 875 600 875C392.893 875 225 707.107 225 500C225 292.893 392.893 125 600 125Z"
                }
            }
        }

        Shape {
            id: glyphA
            width: 1000
            height: 1000
            z: 3
            preferredRendererType: Shape.CurveRenderer

            opacity: Math.min(1.0, root.glyphProgress * 1.8)

            transform: [
                Translate {
                    y: (1.0 - root.glyphProgress) * 90
                },
                Scale {
                    origin.x: 700
                    origin.y: 500
                    xScale: 0.55 + 0.45 * root.glyphProgress
                    yScale: 0.55 + 0.45 * root.glyphProgress
                }
            ]

            ShapePath {
                fillColor: Colours.palette.m3tertiary
                strokeColor: "transparent"

                PathSvg {
                    path: "M566.667 397.25V363H833.333V397.25H566.667ZM566.667 637V534.25H550V500L566.667 414.375H833.333L850 500V534.25H833.333V637H800V534.25H733.333V637H566.667ZM600 602.75H700V534.25H600V602.75ZM584.167 500H815.833L805.833 448.625H594.167L584.167 500Z"
                }
            }
        }
    }
}
