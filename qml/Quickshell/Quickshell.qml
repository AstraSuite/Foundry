pragma Singleton
import QtQuick

QtObject {
    function execDetached(cmd) {}
    function iconPath(name, fallback) { return ""; }
    function env(name) { return ""; }
    function shellPath(path) { return path; }
    readonly property string shellDir: ""
}
