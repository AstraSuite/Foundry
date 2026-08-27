pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes
import "../"

Item {
    id: root

    enum PathType {
        Arc
    }

    property real lineWidth: 4
    property color color: "white"
    property int pathType: WavyLine.Arc
    property real radius: 100
    property real startAngle: -90
    property real fullAngle: 360
    property real value: 0
    property int frequency: 8
    property real amplitudeMultiplier: 0.5
    property real waveProgress: 0

    readonly property real visibleAngle: fullAngle * Math.max(0, Math.min(1, value))
    readonly property real amplitude: lineWidth * amplitudeMultiplier

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            id: shapePath

            fillColor: "transparent"
            strokeColor: root.color
            strokeWidth: root.lineWidth
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                id: arc

                centerX: root.width / 2
                centerY: root.height / 2
                radiusX: root.radius
                radiusY: root.radius
                startAngle: root.startAngle
                sweepAngle: root.visibleAngle
            }
        }
    }

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer
        visible: root.amplitudeMultiplier > 0
        opacity: 0.5

        ShapePath {
            id: wavyPath

            fillColor: "transparent"
            strokeColor: root.color
            strokeWidth: root.lineWidth * 0.5
            capStyle: ShapePath.RoundCap
        }
    }

    Component.onCompleted: rebuildWave()
    onWaveProgressChanged: rebuildWave()
    onVisibleAngleChanged: rebuildWave()
    onAmplitudeMultiplierChanged: rebuildWave()
    onFrequencyChanged: rebuildWave()
    onRadiusChanged: rebuildWave()
    onStartAngleChanged: rebuildWave()

    function rebuildWave(): void {
        if (root.amplitudeMultiplier === 0 || root.visibleAngle <= 0) {
            wavyPath.pathElements = [];
            return;
        }

        const cx = root.width / 2;
        const cy = root.height / 2;
        const r = root.radius;
        const startRad = root.startAngle * Math.PI / 180;
        const angleRad = root.visibleAngle * Math.PI / 180;
        const steps = Math.max(8, Math.floor(root.visibleAngle / 3));
        const amp = root.amplitude;
        const freq = root.frequency;
        const phase = root.waveProgress * 2 * Math.PI;

        const elements = [];
        for (let i = 0; i <= steps; i++) {
            const t = i / steps;
            const angle = startRad + angleRad * t;
            const wave = Math.sin(t * freq * 2 * Math.PI + phase) * amp;
            const currentR = r + wave;
            const x = cx + currentR * Math.cos(angle);
            const y = cy + currentR * Math.sin(angle);

            if (i === 0) {
                elements.push(moveToComponent.createObject(wavyPath, { x: x, y: y }));
            } else {
                elements.push(lineToComponent.createObject(wavyPath, { x: x, y: y }));
            }
        }
        wavyPath.pathElements = elements;
    }

    Component {
        id: moveToComponent
        PathMove {}
    }

    Component {
        id: lineToComponent
        PathLine {}
    }
}
