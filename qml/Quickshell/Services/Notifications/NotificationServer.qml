import QtQuick

Item {
    property bool keepOnReload: true
    property bool bodyMarkupSupported: true
    property bool bodyImagesSupported: true
    property bool bodyHyperlinksSupported: true
    property bool actionsSupported: true
    property bool soundSupported: true
    property bool imageSupported: true
    property bool persistenceSupported: true
    signal notification(var notif)
}
