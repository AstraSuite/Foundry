pragma Singleton

import QtQuick
import QtQuick.Layouts
import Foundry.Config
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
                Component {
                    LogsPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    AppImagePage {}
                }
                Component {
                    LogsPage {}
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
                Component {
                    LogsPage {}
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
                Component {
                    LogsPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    AboutPage {}
                }
                Component {
                    LogsPage {}
                }
            }
        },

        Component {
            StackPage {
                Component {
                    MarketSettingsPage {}
                }
                Component {
                    LogsPage {}
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
