import QtQuick

Item {
    property var stream
    signal collected(string text)
    signal streamFinished(string text)
}
