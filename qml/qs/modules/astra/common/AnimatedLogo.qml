import QtQuick
import QtQuick.Shapes
import qs.components
import qs.services

// The Foundry logo (assets/icons/astra-foundry.svg) drawn in the theme's colours: an
// anvil on a rounded-square badge. On load the badge pops in and the anvil drops onto its
// base, throwing sparks; clicking strikes the anvil like a hammer blow.
Item {
    id: root

    readonly property real designSize: 512
    property bool skipIntroAnimation: false

    property real badgeProgress: 1.0
    property real dropProgress: 1.0
    property real squash: 0.0
    property real sparkProgress: 1.0

    // Where the hammer lands: the middle of the anvil's top face
    readonly property real strikeX: 292
    readonly property real strikeY: 184
    readonly property real baseScale: Math.min(width / designSize, height / designSize)

    implicitWidth: 128
    implicitHeight: 128

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (!introAnim.running && !strikeAnim.running)
                strikeAnim.restart();
        }
    }

    // Click: the anvil squashes under the blow, sparks fly, and it springs back
    SequentialAnimation {
        id: strikeAnim

        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "squash"
                from: 0.0
                to: 1.0
                duration: 90
                easing.type: Easing.OutQuad
            }

            NumberAnimation {
                target: root
                property: "sparkProgress"
                from: 0.0
                to: 1.0
                duration: 650
                easing.type: Easing.OutCubic
            }

            SequentialAnimation {
                PauseAnimation { duration: 90 }
                NumberAnimation {
                    target: root
                    property: "squash"
                    to: 0.0
                    duration: 500
                    easing.type: Easing.OutElastic
                    easing.amplitude: 1.0
                    easing.period: 0.35
                }
            }

            SequentialAnimation {
                NumberAnimation {
                    target: logo
                    property: "scale"
                    to: root.baseScale * 0.96
                    duration: 90
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: logo
                    property: "scale"
                    to: root.baseScale
                    duration: 450
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.4
                }
            }
        }
    }

    Item {
        id: logo

        width: root.designSize
        height: root.designSize
        anchors.centerIn: parent
        scale: root.baseScale
        transformOrigin: Item.Center

        SequentialAnimation {
            id: introAnim
            running: !root.skipIntroAnimation

            ScriptAction {
                script: {
                    root.badgeProgress = 0.0;
                    root.dropProgress = 0.0;
                    root.squash = 0.0;
                    root.sparkProgress = 1.0;
                    logo.opacity = 0.0;
                }
            }

            ParallelAnimation {
                NumberAnimation {
                    target: logo
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 400
                    easing.type: Easing.InOutQuad
                }

                NumberAnimation {
                    target: root
                    property: "badgeProgress"
                    from: 0.0
                    to: 1.0
                    duration: 700
                    easing.type: Easing.OutBack
                    easing.overshoot: 1.2
                }

                SequentialAnimation {
                    PauseAnimation { duration: 300 }
                    NumberAnimation {
                        target: root
                        property: "dropProgress"
                        from: 0.0
                        to: 1.0
                        duration: 550
                        easing.type: Easing.InQuad
                    }

                    // The landing: a squash and a shower of sparks
                    ParallelAnimation {
                        SequentialAnimation {
                            NumberAnimation {
                                target: root
                                property: "squash"
                                from: 0.0
                                to: 1.0
                                duration: 80
                                easing.type: Easing.OutQuad
                            }
                            NumberAnimation {
                                target: root
                                property: "squash"
                                to: 0.0
                                duration: 500
                                easing.type: Easing.OutElastic
                                easing.amplitude: 1.0
                                easing.period: 0.35
                            }
                        }

                        NumberAnimation {
                            target: root
                            property: "sparkProgress"
                            from: 0.0
                            to: 1.0
                            duration: 650
                            easing.type: Easing.OutCubic
                        }
                    }
                }
            }
        }

        // Badge: outer rim and inner face
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            opacity: Math.min(1.0, root.badgeProgress * 1.8)

            transform: Scale {
                origin.x: 256
                origin.y: 256
                xScale: 0.7 + 0.3 * root.badgeProgress
                yScale: 0.7 + 0.3 * root.badgeProgress
            }

            ShapePath {
                fillColor: Colours.palette.m3primary
                strokeColor: "transparent"

                PathSvg {
                    path: "M172 40H340A132 132 0 0 1 472 172V340A132 132 0 0 1 340 472H172A132 132 0 0 1 40 340V172A132 132 0 0 1 172 40Z"
                }
            }

            ShapePath {
                fillColor: Colours.palette.m3primaryContainer
                strokeColor: "transparent"

                PathSvg {
                    path: "M184 88H328A96 96 0 0 1 424 184V328A96 96 0 0 1 328 424H184A96 96 0 0 1 88 328V184A96 96 0 0 1 184 88Z"
                }
            }
        }

        // Base the anvil stands on
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            opacity: Math.min(1.0, root.badgeProgress * 1.5)

            ShapePath {
                fillColor: Colours.palette.m3onPrimaryContainer
                strokeColor: "transparent"

                PathSvg {
                    path: "M176 336v-14a26 26 0 0 1 26-26h108a26 26 0 0 1 26 26v14Z"
                }
            }
        }

        // Anvil: drops in from above and squashes on impact
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            opacity: Math.min(1.0, root.dropProgress * 3.0)

            transform: [
                Scale {
                    // Squash towards the anvil's foot so it stays planted on the base
                    origin.x: 260
                    origin.y: 292
                    xScale: 1.0 + 0.08 * root.squash
                    yScale: 1.0 - 0.14 * root.squash
                },
                Translate {
                    y: -170 * (1.0 - root.dropProgress)
                }
            ]

            ShapePath {
                fillColor: Colours.palette.m3tertiary
                strokeColor: Colours.palette.m3tertiary
                strokeWidth: 8
                joinStyle: ShapePath.RoundJoin

                PathSvg {
                    path: "M124 196Q176 190 200 184H384V230H352Q314 234 306 266V288H214V266Q206 240 168 232Q138 224 124 196Z"
                }
            }
        }

        // Sparks thrown up from the strike point
        Repeater {
            model: [-160, -125, -95, -65, -25]

            Rectangle {
                required property var modelData
                readonly property real angle: modelData * Math.PI / 180
                readonly property real distance: 40 + 90 * root.sparkProgress

                width: 16 * (1.0 - 0.5 * root.sparkProgress)
                height: width
                radius: width / 2
                color: Colours.palette.m3tertiary
                visible: root.sparkProgress > 0 && root.sparkProgress < 1
                opacity: 1.0 - root.sparkProgress
                x: root.strikeX + distance * Math.cos(angle) - width / 2
                y: root.strikeY + distance * Math.sin(angle) - height / 2
            }
        }
    }
}
