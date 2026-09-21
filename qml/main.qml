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
    readonly property color accent: Theme.accent
    readonly property color accentLight: Theme.accentLight
    readonly property color bg: Theme.bg
    readonly property color surface: Theme.surface
    readonly property color textPrimary: Theme.textPrimary
    readonly property color textMuted: Theme.textMuted
    readonly property color border: Theme.border
    readonly property color sidebar: Theme.sidebar
    readonly property color danger: Theme.danger

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
