import QtQuick
import QtQuick.Shapes
import qs.components
import qs.services

Item {
    id: root

    readonly property real designWidth: 1074
    readonly property real designHeight: 1053
    property bool skipIntroAnimation: false

    property real bucketProgress: 1.0
    property real tiltProgress: 1.0
    property real pourProgress: 1.0
    property real clickTilt: 0.0
    property real clickSurge: 0.0

    implicitWidth: 128
    implicitHeight: 128

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (!introAnim.running && !clickPourAnim.running) {
                clickPourAnim.restart();
            }
        }
    }

    // Interactive pour surge animation on click
    SequentialAnimation {
        id: clickPourAnim
        running: false

        ParallelAnimation {
            // Bucket tips forward into a deeper pour then recoils
            SequentialAnimation {
                NumberAnimation {
                    target: root
                    property: "clickTilt"
                    from: 0
                    to: 15
                    duration: 220
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: root
                    property: "clickTilt"
                    from: 15
                    to: -5
                    duration: 260
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: root
                    property: "clickTilt"
                    from: -5
                    to: 0
                    duration: 320
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.4
                }
            }

            // Molten stream surges and expands
            SequentialAnimation {
                NumberAnimation {
                    target: root
                    property: "clickSurge"
                    from: 0.0
                    to: 1.0
                    duration: 220
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: root
                    property: "clickSurge"
                    from: 1.0
                    to: 0.0
                    duration: 500
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.2
                }
            }

            // Overall subtle bounce
            SequentialAnimation {
                NumberAnimation {
                    target: logo
                    property: "scale"
                    from: Math.min(root.width / root.designWidth, root.height / root.designHeight)
                    to: Math.min(root.width / root.designWidth, root.height / root.designHeight) * 1.08
                    duration: 220
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: logo
                    property: "scale"
                    from: Math.min(root.width / root.designWidth, root.height / root.designHeight) * 1.08
                    to: Math.min(root.width / root.designWidth, root.height / root.designHeight)
                    duration: 480
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.3
                }
            }
        }

        ScriptAction {
            script: {
                root.clickTilt = 0;
                root.clickSurge = 0;
            }
        }
    }

    Item {
        id: logo

        implicitWidth: root.designWidth
        implicitHeight: root.designHeight

        anchors.centerIn: parent
        scale: Math.min(root.width / root.designWidth, root.height / root.designHeight)
        transformOrigin: Item.Center

        rotation: 0.0
        opacity: 1.0

        SequentialAnimation {
            id: introAnim
            running: !root.skipIntroAnimation

            ScriptAction {
                script: {
                    root.bucketProgress = 0.0;
                    root.tiltProgress = 0.0;
                    root.pourProgress = 0.0;
                    root.clickTilt = 0.0;
                    root.clickSurge = 0.0;
                    logo.opacity = 0.0;
                }
            }

            ParallelAnimation {
                // Fade in
                NumberAnimation {
                    target: logo
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 500
                    easing.type: Easing.InOutQuad
                }

                // Bucket scale entrance
                NumberAnimation {
                    target: root
                    property: "bucketProgress"
                    from: 0.0
                    to: 1.0
                    duration: 700
                    easing.type: Easing.OutCubic
                }

                // Bucket tilts forward into pouring position
                SequentialAnimation {
                    PauseAnimation { duration: 200 }
                    NumberAnimation {
                        target: root
                        property: "tiltProgress"
                        from: 0.0
                        to: 1.0
                        duration: 800
                        easing.type: Easing.OutBack
                        easing.overshoot: 1.3
                    }
                }

                // Molten stream pours out as ladle tilts
                SequentialAnimation {
                    PauseAnimation { duration: 450 }
                    NumberAnimation {
                        target: root
                        property: "pourProgress"
                        from: 0.0
                        to: 1.0
                        duration: 650
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }

        // 1. Molten Steel Stream pouring out from behind the ladle
        Item {
            id: moltenStream
            width: root.designWidth
            height: root.designHeight
            z: 0

            opacity: Math.min(1.0, root.pourProgress * 1.6)

            transform: [
                Scale {
                    origin.x: 1023.67
                    origin.y: 401.717
                    xScale: 0.8 + 0.2 * root.pourProgress + root.clickSurge * 0.15
                    yScale: root.pourProgress
                },
                Translate {
                    y: (1.0 - root.pourProgress) * -30
                }
            ]

            Shape {
                width: root.designWidth
                height: root.designHeight
                preferredRendererType: Shape.CurveRenderer

                // Horizontal stream leaving crucible lip
                ShapePath {
                    fillColor: Colours.palette.m3tertiary
                    strokeColor: "transparent"

                    PathSvg {
                        path: "M839.674 351.717H1023.67V451.717H839.674V351.717Z"
                    }
                }

                // Curved pour elbow
                ShapePath {
                    fillColor: Colours.palette.m3tertiary
                    strokeColor: "transparent"

                    PathSvg {
                        path: "M1073.67 401.717C1073.67 429.331 1051.29 451.717 1023.67 451.717C996.06 451.717 973.674 429.331 973.674 401.717C973.674 374.103 996.06 351.717 1023.67 351.717C1051.29 351.717 1073.67 374.103 1073.67 401.717Z"
                    }
                }

                // Vertical cascading waterfall of molten steel
                ShapePath {
                    fillColor: Colours.palette.m3tertiary
                    strokeColor: "transparent"

                    PathSvg {
                        path: "M973.674 401.717H1073.67V1037.72C1073.67 1046 1066.96 1052.72 1058.67 1052.72H988.674C980.39 1052.72 973.674 1046 973.674 1037.72V401.717Z"
                    }
                }
            }
        }

        // 2. Ladle / Bucket Group (Tilted crucible + inner vessel + emblem, on top of the pour)
        Item {
            id: bucketGroup
            width: root.designWidth
            height: root.designHeight
            z: 1

            opacity: Math.min(1.0, root.bucketProgress * 1.8)

            transform: [
                Rotation {
                    origin.x: 650
                    origin.y: 550
                    angle: (1.0 - root.tiltProgress) * -30 + root.clickTilt
                },
                Scale {
                    origin.x: 486
                    origin.y: 488
                    xScale: 0.7 + 0.3 * root.bucketProgress
                    yScale: 0.7 + 0.3 * root.bucketProgress
                }
            ]

            // Outer dark crucible rim
            Shape {
                id: outerBadge
                width: root.designWidth
                height: root.designHeight
                z: 1
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    fillColor: Colours.palette.m3onSurface
                    strokeColor: "transparent"

                    PathSvg {
                        path: "M502.637 6.16955C514.581 -2.96412 531.442 -1.84503 542.074 8.78708L964.548 431.261C975.18 441.894 976.299 458.754 967.166 470.698L665.406 865.306C662.759 868.769 659.388 871.613 655.53 873.64L472.341 969.892C460.713 976.001 446.462 973.836 437.174 964.548L8.78757 536.161C-0.50068 526.873 -2.66622 512.622 3.4435 500.994L99.695 317.806C101.722 313.947 104.566 310.577 108.029 307.929L502.637 6.16955Z"
                    }
                }
            }

            // Inner primary crucible body
            Shape {
                id: innerBadge
                width: root.designWidth
                height: root.designHeight
                z: 2
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    fillColor: Colours.palette.m3primary
                    strokeColor: "transparent"

                    PathSvg {
                        path: "M502.44 41.4278C513.484 32.973 529.074 34.0089 538.905 43.8508L929.549 434.928C939.38 444.77 940.415 460.378 931.969 471.434L652.946 836.716C650.497 839.921 647.381 842.554 643.813 844.431L474.427 933.529C463.675 939.185 450.498 937.18 441.909 928.582L45.7993 532.032C37.2109 523.434 35.2085 510.243 40.8579 499.479L129.857 329.904C131.732 326.332 134.362 323.212 137.563 320.761L502.44 41.4278Z"
                    }
                }
            }

            // Emblem on the bucket
            Shape {
                id: anvilGlyph
                width: root.designWidth
                height: root.designHeight
                z: 3
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    fillColor: Colours.palette.m3tertiary
                    strokeColor: "transparent"

                    PathSvg {
                        path: "M287.674 330.217V278.717H687.674V330.217H287.674ZM287.674 690.717V536.217H262.674V484.717L287.674 355.967H687.674L712.674 484.717V536.217H687.674V690.717H637.674V536.217H537.674V690.717H287.674ZM337.674 639.217H487.674V536.217H337.674V639.217ZM313.924 484.717H661.424L646.424 407.467H328.924L313.924 484.717Z"
                    }
                }
            }
        }
    }
}

