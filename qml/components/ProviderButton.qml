import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A provider card used in the setup wizard.
Rectangle {
    id: root

    property var provider: ({})
    property bool selected: false
    signal clicked()

    width: parent.width
    height: 64
    radius: 10
    color: selected ? "#E8F1FB" : (hoverArea.containsMouse ? "#F5F6F8" : "#FFFFFF")
    border.width: selected ? 2 : 1
    border.color: selected ? "#0F6CBD" : "#E0E0E0"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 14

        Rectangle {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            radius: 20
            color: provider.color ? provider.color : "#999999"

            Text {
                anchors.centerIn: parent
                text: provider.letter ? provider.letter : ""
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                text: provider.brand ? provider.brand : provider.name
                font.pixelSize: 15
                font.weight: Font.DemiBold
                color: "#1F1F1F"
            }

            Text {
                text: provider.description ? provider.description : ""
                font.pixelSize: 12
                color: "#666666"
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        Rectangle {
            Layout.preferredWidth: 18
            Layout.preferredHeight: 18
            radius: 9
            color: selected ? "#0F6CBD" : "transparent"
            border.width: selected ? 0 : 1
            border.color: "#C0C0C0"

            Text {
                anchors.centerIn: parent
                text: "\u2713"
                color: "white"
                font.pixelSize: 11
                visible: selected
            }
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
