import QtQuick
import QtQuick.Controls
import OmaSuite 1.0

ApplicationWindow {
    id: root
    width: 1280
    height: 820
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: "OmaSuite"

    // Palette
    readonly property color accent: "#0F6CBD"
    readonly property color accentLight: "#E8F1FB"
    readonly property color bg: "#F3F4F6"
    readonly property color surface: "#FFFFFF"
    readonly property color textPrimary: "#1F1F1F"
    readonly property color textMuted: "#666666"
    readonly property color border: "#E0E0E0"
    readonly property color sidebar: "#ECEEF1"
    readonly property color danger: "#D13438"

    Loader {
        anchors.fill: parent
        sourceComponent: Accounts.hasAccounts ? mainWindowComponent : wizardComponent
    }

    Component {
        id: wizardComponent
        SetupWizard {}
    }

    Component {
        id: mainWindowComponent
        MainWindow {}
    }
}
