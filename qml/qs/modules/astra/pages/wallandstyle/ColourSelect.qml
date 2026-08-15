import QtQuick
import QtQuick.Layouts
import Quickshell
import AstraMarket.Config
import qs.components
import qs.components.controls
import qs.services
import qs.modules.astra.common

PageBase {
    id: root

    title: qsTr("Colours")
    isSubPage: true

    readonly property list<var> variantData: [
        { name: "vibrant", label: qsTr("Vibrant") },
        { name: "tonalspot", label: qsTr("Tonal Spot") },
        { name: "expressive", label: qsTr("Expressive") },
        { name: "fidelity", label: qsTr("Fidelity") },
        { name: "content", label: qsTr("Content") },
        { name: "fruitsalad", label: qsTr("Fruit Salad") },
        { name: "rainbow", label: qsTr("Rainbow") },
        { name: "neutral", label: qsTr("Neutral") },
        { name: "monochrome", label: qsTr("Monochrome") }
    ]

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: root.cappedWidth
        spacing: Tokens.spacing.extraSmall / 2

        Variants {
            id: schemeItems
            model: Schemes.list
            MenuItem {
                required property var modelData
                text: `${modelData.name} ${modelData.flavour}`
                onClicked: {
                    Quickshell.execDetached(["caelestia", "scheme", "set", "-n", modelData.name, "-f", modelData.flavour]);
                }
            }
        }

        Variants {
            id: variantItems
            model: root.variantData
            MenuItem {
                required property var modelData
                text: modelData.label
                onClicked: {
                    Quickshell.execDetached(["caelestia", "scheme", "set", "-v", modelData.name]);
                }
            }
        }

        SectionHeader {
            first: true
            text: qsTr("General")
        }

        ToggleRow {
            first: true
            last: true
            Layout.fillWidth: true
            text: qsTr("Smart colour scheme")
            subtext: qsTr("Derive theme mode and variant from the wallpaper")
            checked: GlobalConfig.services.smartScheme
            onToggled: GlobalConfig.services.smartScheme = checked
        }

        SectionHeader {
            text: qsTr("Scheme Settings")
        }

        SelectRow {
            first: true
            label: qsTr("Colour Scheme")
            subtext: qsTr("Select your base colour scheme style")
            menuItems: schemeItems.instances
            active: {
                const current = Colours.scheme + " " + Colours.flavour;
                const list = menuItems;
                for (let i = 0; i < list.length; i++) {
                    if (list[i].text === current)
                        return list[i];
                }
                return null;
            }
            fallbackText: Colours.scheme + " " + Colours.flavour
        }

        SelectRow {
            last: true
            label: qsTr("Scheme Variant")
            subtext: qsTr("Select the color distribution algorithm")
            menuItems: variantItems.instances
            active: {
                const current = Colours.variant;
                let match = null;
                for (let i = 0; i < root.variantData.length; i++) {
                    if (root.variantData[i].name === current) {
                        match = root.variantData[i];
                        break;
                    }
                }
                if (!match) return null;
                const list = menuItems;
                for (let i = 0; i < list.length; i++) {
                    if (list[i].text === match.label)
                        return list[i];
                }
                return null;
            }
            fallbackText: Colours.variant
        }
    }
}
