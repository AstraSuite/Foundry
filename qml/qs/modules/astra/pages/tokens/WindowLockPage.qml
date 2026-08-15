import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Window & Lock Sizes")
    isSubPage: true

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: TokenConfig.appearance.spacing.extraSmall / 2

        SectionHeader {
            first: true
            text: qsTr("Lock Screen")
        }

        SliderRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Height Multiplier")
            value: TokenConfig.sizes.lock.heightMult
            valueLabel: value.toFixed(2)
            showReset: true
            onMoved: v => TokenConfig.sizes.lock.heightMult = v
            onReset: { TokenConfig.sizes.lock.heightMult = TokenConfig.defaults().sizes.lock.heightMult; TokenConfig.sizes.lock.resetOption("heightMult"); }
        }

        SliderRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Aspect Ratio")
            value: (TokenConfig.sizes.lock.ratio - 1.0) / 1.5
            valueLabel: (1.0 + value * 1.5).toFixed(2)
            showReset: true
            onMoved: v => TokenConfig.sizes.lock.ratio = 1.0 + v * 1.5
            onReset: { TokenConfig.sizes.lock.ratio = TokenConfig.defaults().sizes.lock.ratio; TokenConfig.sizes.lock.resetOption("ratio"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Center Card Width")
            value: TokenConfig.sizes.lock.centerWidth
            from: 200
            to: 1000
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.lock.centerWidth = v
            onReset: { TokenConfig.sizes.lock.centerWidth = TokenConfig.defaults().sizes.lock.centerWidth; TokenConfig.sizes.lock.resetOption("centerWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Forecast Item Width")
            value: TokenConfig.sizes.lock.forecastItemWidth
            from: 20
            to: 100
            stepSize: 2
            showReset: true
            onMoved: v => TokenConfig.sizes.lock.forecastItemWidth = v
            onReset: { TokenConfig.sizes.lock.forecastItemWidth = TokenConfig.defaults().sizes.lock.forecastItemWidth; TokenConfig.sizes.lock.resetOption("forecastItemWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Large Logo Width")
            value: TokenConfig.sizes.lock.largeLogoWidth
            from: 100
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.lock.largeLogoWidth = v
            onReset: { TokenConfig.sizes.lock.largeLogoWidth = TokenConfig.defaults().sizes.lock.largeLogoWidth; TokenConfig.sizes.lock.resetOption("largeLogoWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Large Font Width")
            value: TokenConfig.sizes.lock.largeFontWidth
            from: 100
            to: 600
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.lock.largeFontWidth = v
            onReset: { TokenConfig.sizes.lock.largeFontWidth = TokenConfig.defaults().sizes.lock.largeFontWidth; TokenConfig.sizes.lock.resetOption("largeFontWidth"); }
        }

        SectionHeader {
            text: qsTr("Astra Settings Window")
        }

        SliderRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Height Multiplier")
            value: TokenConfig.sizes.astra.heightMult
            valueLabel: value.toFixed(2)
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.heightMult = v
            onReset: { TokenConfig.sizes.astra.heightMult = TokenConfig.defaults().sizes.astra.heightMult; TokenConfig.sizes.astra.resetOption("heightMult"); }
        }

        SliderRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Aspect Ratio")
            value: (TokenConfig.sizes.astra.ratio - 1.0) / 1.5
            valueLabel: (1.0 + value * 1.5).toFixed(2)
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.ratio = 1.0 + v * 1.5
            onReset: { TokenConfig.sizes.astra.ratio = TokenConfig.defaults().sizes.astra.ratio; TokenConfig.sizes.astra.resetOption("ratio"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Minimum Width")
            value: TokenConfig.sizes.astra.minWidth
            from: 400
            to: 1200
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.minWidth = v
            onReset: { TokenConfig.sizes.astra.minWidth = TokenConfig.defaults().sizes.astra.minWidth; TokenConfig.sizes.astra.resetOption("minWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Minimum Height")
            value: TokenConfig.sizes.astra.minHeight
            from: 300
            to: 900
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.minHeight = v
            onReset: { TokenConfig.sizes.astra.minHeight = TokenConfig.defaults().sizes.astra.minHeight; TokenConfig.sizes.astra.resetOption("minHeight"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Max Navigation Width")
            value: TokenConfig.sizes.astra.maxNavWidth
            from: 200
            to: 800
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.maxNavWidth = v
            onReset: { TokenConfig.sizes.astra.maxNavWidth = TokenConfig.defaults().sizes.astra.maxNavWidth; TokenConfig.sizes.astra.resetOption("maxNavWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            Layout.fillWidth: true
            label: qsTr("Max Content Width")
            value: TokenConfig.sizes.astra.maxContentWidth
            from: 400
            to: 1200
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.maxContentWidth = v
            onReset: { TokenConfig.sizes.astra.maxContentWidth = TokenConfig.defaults().sizes.astra.maxContentWidth; TokenConfig.sizes.astra.resetOption("maxContentWidth"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Popup Width")
            value: TokenConfig.sizes.astra.popupWidth
            from: 150
            to: 500
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.astra.popupWidth = v
            onReset: { TokenConfig.sizes.astra.popupWidth = TokenConfig.defaults().sizes.astra.popupWidth; TokenConfig.sizes.astra.resetOption("popupWidth"); }
        }

        SectionHeader {
            text: qsTr("Window Information Overlay")
        }

        SliderRow {
            first: true
            Layout.fillWidth: true
            label: qsTr("Height Multiplier")
            value: TokenConfig.sizes.winfo.heightMult
            valueLabel: value.toFixed(2)
            showReset: true
            onMoved: v => TokenConfig.sizes.winfo.heightMult = v
            onReset: { TokenConfig.sizes.winfo.heightMult = TokenConfig.defaults().sizes.winfo.heightMult; TokenConfig.sizes.winfo.resetOption("heightMult"); }
        }

        StepperRow {
            Layout.topMargin: TokenConfig.appearance.spacing.extraSmall / 2 - parent.spacing
            last: true
            Layout.fillWidth: true
            label: qsTr("Details Pane Width")
            value: TokenConfig.sizes.winfo.detailsWidth
            from: 200
            to: 800
            stepSize: 10
            showReset: true
            onMoved: v => TokenConfig.sizes.winfo.detailsWidth = v
            onReset: { TokenConfig.sizes.winfo.detailsWidth = TokenConfig.defaults().sizes.winfo.detailsWidth; TokenConfig.sizes.winfo.resetOption("detailsWidth"); }
        }
    }
}
