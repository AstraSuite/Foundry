pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Foundry.Config
import qs.components
import qs.components.containers
import qs.components.controls
import qs.services
import qs.modules.astra

ColumnLayout {
    id: root

    required property string title
    required property AstraState nState
    readonly property GlobalConfig targetConfig: GlobalConfig.forScreen(nState.targetScreen)
    property bool isSubPage
    property bool scrollable: true
    readonly property int cappedWidth: Math.min(Tokens.sizes.astra.maxContentWidth, width)
    readonly property alias flickable: flickable

    default property Item contentChild

    spacing: Tokens.spacing.extraLargeIncreased

    MouseArea {
        z: 1
        implicitWidth: header.implicitWidth
        implicitHeight: header.implicitHeight - Layout.bottomMargin
        Layout.bottomMargin: -flickable.topMargin
        onClicked: focus = true

        RowLayout {
            id: header

            spacing: Tokens.spacing.largeIncreased

            Loader {
                visible: active
                active: root.isSubPage
                asynchronous: true
                sourceComponent: IconButton {
                    icon: "arrow_back"
                    font: Tokens.font.icon.medium
                    type: IconButton.Tonal
                    isRound: true
                    inactiveColour: Colours.tPalette.m3surfaceContainerHigh
                    inactiveOnColour: Colours.palette.m3onSurfaceVariant
                    onClicked: root.nState.closeSubPage()
                }
            }

            StyledText {
                Layout.fillWidth: true
                text: root.title
                font: Tokens.font.title.large
                elide: Text.ElideRight
            }
        }
    }

    VerticalFadeFlickable {
        id: flickable

        interactive: root.scrollable
        Layout.fillWidth: true
        Layout.fillHeight: true

        Layout.topMargin: -topMargin
        topMargin: Tokens.padding.large
        bottomMargin: Tokens.padding.extraLarge

        contentHeight: root.scrollable ? (root.contentChild ? (root.contentChild.implicitHeight > 0 ? root.contentChild.implicitHeight : root.contentChild.childrenRect.height) : 0) : height

        Binding {
            target: root.contentChild
            property: "parent"
            value: flickable.contentItem
            when: root.contentChild !== null
        }

        Binding {
            target: root.contentChild
            property: "width"
            value: flickable.width
            when: root.contentChild !== null
        }

        TapHandler {
            onTapped: flickable.focus = true
        }
    }
}
