import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Font Sizes")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: TokenConfig.appearance.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Standard Fonts")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Small")
            value: TokenConfig.appearance.fontSize.small
            from: 6
            to: 36
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.small = v
            onReset: { TokenConfig.appearance.fontSize.small = TokenConfig.defaults().appearance.fontSize.small; TokenConfig.appearance.fontSize.resetOption("small"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Smaller")
            value: TokenConfig.appearance.fontSize.smaller
            from: 6
            to: 36
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.smaller = v
            onReset: { TokenConfig.appearance.fontSize.smaller = TokenConfig.defaults().appearance.fontSize.smaller; TokenConfig.appearance.fontSize.resetOption("smaller"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Normal")
            value: TokenConfig.appearance.fontSize.normal
            from: 8
            to: 40
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.normal = v
            onReset: { TokenConfig.appearance.fontSize.normal = TokenConfig.defaults().appearance.fontSize.normal; TokenConfig.appearance.fontSize.resetOption("normal"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Larger")
            value: TokenConfig.appearance.fontSize.larger
            from: 8
            to: 40
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.larger = v
            onReset: { TokenConfig.appearance.fontSize.larger = TokenConfig.defaults().appearance.fontSize.larger; TokenConfig.appearance.fontSize.resetOption("larger"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large")
            value: TokenConfig.appearance.fontSize.large
            from: 10
            to: 48
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.large = v
            onReset: { TokenConfig.appearance.fontSize.large = TokenConfig.defaults().appearance.fontSize.large; TokenConfig.appearance.fontSize.resetOption("large"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Extra Large")
            value: TokenConfig.appearance.fontSize.extraLarge
            from: 12
            to: 72
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.extraLarge = v
            onReset: { TokenConfig.appearance.fontSize.extraLarge = TokenConfig.defaults().appearance.fontSize.extraLarge; TokenConfig.appearance.fontSize.resetOption("extraLarge"); }
        }

        SectionHeader {
            text: qsTr("Monospace Fonts")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Mono Small")
            value: TokenConfig.appearance.fontSize.monoSmall
            from: 6
            to: 36
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.monoSmall = v
            onReset: { TokenConfig.appearance.fontSize.monoSmall = TokenConfig.defaults().appearance.fontSize.monoSmall; TokenConfig.appearance.fontSize.resetOption("monoSmall"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Mono Medium")
            value: TokenConfig.appearance.fontSize.monoMedium
            from: 8
            to: 40
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.monoMedium = v
            onReset: { TokenConfig.appearance.fontSize.monoMedium = TokenConfig.defaults().appearance.fontSize.monoMedium; TokenConfig.appearance.fontSize.resetOption("monoMedium"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Mono Large")
            value: TokenConfig.appearance.fontSize.monoLarge
            from: 10
            to: 48
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.fontSize.monoLarge = v
            onReset: { TokenConfig.appearance.fontSize.monoLarge = TokenConfig.defaults().appearance.fontSize.monoLarge; TokenConfig.appearance.fontSize.resetOption("monoLarge"); }
        }
    }
}
