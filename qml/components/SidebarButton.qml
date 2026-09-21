import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0

Rectangle {
    id: root

    property string label: ""
    property string symbol: ""
    property bool active: false
    property color accent: Theme.accent
    signal clicked()

    width: parent.width
    height: 42
    radius: 8
    color: active ? Qt.rgba(accent.r, accent.g, accent.b, 0.12) : (hoverArea.containsMouse ? Theme.hoverOverlay : "transparent")

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        Text {
            text: root.symbol
            font.pixelSize: 16
            color: active ? root.accent : Theme.textSecondary
            Layout.preferredWidth: 22
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            text: root.label
            font.pixelSize: 14
            font.weight: active ? Font.DemiBold : Font.Normal
            color: active ? Theme.textPrimary : Theme.textSecondary
            Layout.fillWidth: true
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
