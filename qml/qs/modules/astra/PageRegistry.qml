pragma Singleton

import QtQuick

QtObject {
    id: root

    readonly property var pages: [

        {
            label: qsTr("Explore"),
            icon: "storefront",
            description: qsTr("Browse packages"),
            category: "marketplace"
        },
        {
            label: qsTr("AppImage installer"),
            icon: "extension",
            description: qsTr("Drag and drop AppImages to install"),
            category: "marketplace"
        },

        {
            label: qsTr("Installed apps"),
            icon: "download_done",
            description: qsTr("Manage installed packages"),
            category: "packages"
        },
        {
            label: qsTr("Updates"),
            icon: "update",
            description: qsTr("System and package updates"),
            category: "packages"
        },

        {
            label: qsTr("About"),
            icon: "info",
            description: qsTr("Versioning and credits"),
            category: "system"
        },
        {
            label: qsTr("Settings"),
            icon: "settings",
            description: qsTr("Toggle backends, control theming"),
            category: "system"
        }
    ]
}
