import QtQuick

Item {
    id: root
    property string path
    property bool watchChanges: true
    property bool printErrors: true

    signal fileChanged
    signal loaded
    signal loadFailed(var err)

    property string _content: ""

    function text() {
        return _content;
    }

    function reload() {
        if (!path) return;
        let req = new XMLHttpRequest();
        let fileUrl = path.startsWith("/") ? "file://" + path : path;
        req.open("GET", fileUrl, false);
        try {
            req.send();
            if (req.status === 200 || req.status === 0) {
                const newText = req.responseText;
                if (newText !== _content) {
                    _content = newText;
                    root.loaded();
                }
            } else {
                root.loadFailed(req.statusText);
            }
        } catch (e) {
            root.loadFailed(e.toString());
        }
    }

    onPathChanged: {
        reload();
    }

    Component.onCompleted: {
        reload();
    }
}
