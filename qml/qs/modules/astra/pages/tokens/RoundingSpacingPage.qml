import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Rounding & Spacing")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: TokenConfig.appearance.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Corner Rounding")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Extra Small")
            value: TokenConfig.appearance.rounding.extraSmall
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.extraSmall = v
            onReset: { TokenConfig.appearance.rounding.extraSmall = TokenConfig.defaults().appearance.rounding.extraSmall; TokenConfig.appearance.rounding.resetOption("extraSmall"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Small")
            value: TokenConfig.appearance.rounding.small
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.small = v
            onReset: { TokenConfig.appearance.rounding.small = TokenConfig.defaults().appearance.rounding.small; TokenConfig.appearance.rounding.resetOption("small"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Medium")
            value: TokenConfig.appearance.rounding.medium
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.medium = v
            onReset: { TokenConfig.appearance.rounding.medium = TokenConfig.defaults().appearance.rounding.medium; TokenConfig.appearance.rounding.resetOption("medium"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large")
            value: TokenConfig.appearance.rounding.large
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.large = v
            onReset: { TokenConfig.appearance.rounding.large = TokenConfig.defaults().appearance.rounding.large; TokenConfig.appearance.rounding.resetOption("large"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large Increased")
            value: TokenConfig.appearance.rounding.largeIncreased
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.largeIncreased = v
            onReset: { TokenConfig.appearance.rounding.largeIncreased = TokenConfig.defaults().appearance.rounding.largeIncreased; TokenConfig.appearance.rounding.resetOption("largeIncreased"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Extra Large")
            value: TokenConfig.appearance.rounding.extraLarge
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.extraLarge = v
            onReset: { TokenConfig.appearance.rounding.extraLarge = TokenConfig.defaults().appearance.rounding.extraLarge; TokenConfig.appearance.rounding.resetOption("extraLarge"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Extra Large Increased")
            value: TokenConfig.appearance.rounding.extraLargeIncreased
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.extraLargeIncreased = v
            onReset: { TokenConfig.appearance.rounding.extraLargeIncreased = TokenConfig.defaults().appearance.rounding.extraLargeIncreased; TokenConfig.appearance.rounding.resetOption("extraLargeIncreased"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Extra Extra Large")
            value: TokenConfig.appearance.rounding.extraExtraLarge
            from: 0
            to: 150
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.appearance.rounding.extraExtraLarge = v
            onReset: { TokenConfig.appearance.rounding.extraExtraLarge = TokenConfig.defaults().appearance.rounding.extraExtraLarge; TokenConfig.appearance.rounding.resetOption("extraExtraLarge"); }
        }

        SectionHeader {
            text: qsTr("Layout Spacing")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Extra Small")
            value: TokenConfig.appearance.spacing.extraSmall
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.extraSmall = v
            onReset: { TokenConfig.appearance.spacing.extraSmall = TokenConfig.defaults().appearance.spacing.extraSmall; TokenConfig.appearance.spacing.resetOption("extraSmall"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Small")
            value: TokenConfig.appearance.spacing.small
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.small = v
            onReset: { TokenConfig.appearance.spacing.small = TokenConfig.defaults().appearance.spacing.small; TokenConfig.appearance.spacing.resetOption("small"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Medium")
            value: TokenConfig.appearance.spacing.medium
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.medium = v
            onReset: { TokenConfig.appearance.spacing.medium = TokenConfig.defaults().appearance.spacing.medium; TokenConfig.appearance.spacing.resetOption("medium"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large")
            value: TokenConfig.appearance.spacing.large
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.large = v
            onReset: { TokenConfig.appearance.spacing.large = TokenConfig.defaults().appearance.spacing.large; TokenConfig.appearance.spacing.resetOption("large"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large Increased")
            value: TokenConfig.appearance.spacing.largeIncreased
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.largeIncreased = v
            onReset: { TokenConfig.appearance.spacing.largeIncreased = TokenConfig.defaults().appearance.spacing.largeIncreased; TokenConfig.appearance.spacing.resetOption("largeIncreased"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Extra Large")
            value: TokenConfig.appearance.spacing.extraLarge
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.extraLarge = v
            onReset: { TokenConfig.appearance.spacing.extraLarge = TokenConfig.defaults().appearance.spacing.extraLarge; TokenConfig.appearance.spacing.resetOption("extraLarge"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Extra Large Increased")
            value: TokenConfig.appearance.spacing.extraLargeIncreased
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.extraLargeIncreased = v
            onReset: { TokenConfig.appearance.spacing.extraLargeIncreased = TokenConfig.defaults().appearance.spacing.extraLargeIncreased; TokenConfig.appearance.spacing.resetOption("extraLargeIncreased"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Extra Extra Large")
            value: TokenConfig.appearance.spacing.extraExtraLarge
            from: 0
            to: 150
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.appearance.spacing.extraExtraLarge = v
            onReset: { TokenConfig.appearance.spacing.extraExtraLarge = TokenConfig.defaults().appearance.spacing.extraExtraLarge; TokenConfig.appearance.spacing.resetOption("extraExtraLarge"); }
        }

        SectionHeader {
            text: qsTr("Inner Padding")
        }

        StepperRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Extra Small")
            value: TokenConfig.appearance.padding.extraSmall
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.extraSmall = v
            onReset: { TokenConfig.appearance.padding.extraSmall = TokenConfig.defaults().appearance.padding.extraSmall; TokenConfig.appearance.padding.resetOption("extraSmall"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Small")
            value: TokenConfig.appearance.padding.small
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.small = v
            onReset: { TokenConfig.appearance.padding.small = TokenConfig.defaults().appearance.padding.small; TokenConfig.appearance.padding.resetOption("small"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Medium")
            value: TokenConfig.appearance.padding.medium
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.medium = v
            onReset: { TokenConfig.appearance.padding.medium = TokenConfig.defaults().appearance.padding.medium; TokenConfig.appearance.padding.resetOption("medium"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large")
            value: TokenConfig.appearance.padding.large
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.large = v
            onReset: { TokenConfig.appearance.padding.large = TokenConfig.defaults().appearance.padding.large; TokenConfig.appearance.padding.resetOption("large"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large Increased")
            value: TokenConfig.appearance.padding.largeIncreased
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.largeIncreased = v
            onReset: { TokenConfig.appearance.padding.largeIncreased = TokenConfig.defaults().appearance.padding.largeIncreased; TokenConfig.appearance.padding.resetOption("largeIncreased"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Extra Large")
            value: TokenConfig.appearance.padding.extraLarge
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.extraLarge = v
            onReset: { TokenConfig.appearance.padding.extraLarge = TokenConfig.defaults().appearance.padding.extraLarge; TokenConfig.appearance.padding.resetOption("extraLarge"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Extra Large Increased")
            value: TokenConfig.appearance.padding.extraLargeIncreased
            from: 0
            to: 100
            stepSize: 1
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.extraLargeIncreased = v
            onReset: { TokenConfig.appearance.padding.extraLargeIncreased = TokenConfig.defaults().appearance.padding.extraLargeIncreased; TokenConfig.appearance.padding.resetOption("extraLargeIncreased"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Extra Extra Large")
            value: TokenConfig.appearance.padding.extraExtraLarge
            from: 0
            to: 150
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.appearance.padding.extraExtraLarge = v
            onReset: { TokenConfig.appearance.padding.extraExtraLarge = TokenConfig.defaults().appearance.padding.extraExtraLarge; TokenConfig.appearance.padding.resetOption("extraExtraLarge"); }
        }
    }
}
