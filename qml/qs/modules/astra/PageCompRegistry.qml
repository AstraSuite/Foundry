pragma Singleton

import QtQuick
import QtQuick.Layouts
import AstraMarket.Config
import qs.components
import qs.services
import qs.modules.astra.common
import qs.modules.astra.pages

QtObject {
    id: root

    readonly property list<Component> pageComps: [

        Component {
            StackPage {
                Component {
                    ExplorePage {}
                }
                Component {
                    AppDetailPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    AppImagePage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    InstalledPage {}
                }
                Component {
                    AppDetailPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    UpdatesPage {}
                }
                Component {
                    AppDetailPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    AboutPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    MarketSettingsPage {}
                }
            }
        }
    ]

    readonly property Component placeholderComp: Component {
        PlaceholderComp {}
    }

    component PlaceholderComp: Item {
        property AstraState nState

        ColumnLayout {
            anchors.centerIn: parent
            spacing: Tokens.padding.extraSmall

            MaterialIcon {
                Layout.alignment: Qt.AlignHCenter
                text: "handyman"
                color: Colours.palette.m3outlineVariant
                fontStyle: Tokens.font.icon.extraLarge
            }

            StyledText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Page under construction")
                color: Colours.palette.m3outlineVariant
                font: Tokens.font.title.large
            }

            StyledText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("This page will be available in a future update.")
                color: Colours.palette.m3outlineVariant
                font: Tokens.font.body.large
            }
        }
    }
}
