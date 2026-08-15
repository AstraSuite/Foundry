import Quickshell
import Quickshell.Wayland
import AstraMarket.Config

PanelWindow {

    required property string name

    WlrLayershell.namespace: `caelestia-${name}`
    color: "transparent"

    contentItem.Config.screen: screen.name
    contentItem.Tokens.screen: screen.name
}
