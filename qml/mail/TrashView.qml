import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0

Rectangle {
    id: trashView
    color: Theme.bg

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 12

                Text {
                    text: "Trash"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }

                Text {
                    text: AppCore.mail.trashCount + (AppCore.mail.trashCount === 1 ? " message" : " messages")
                    font.pixelSize: 13
                    color: Theme.textFaint
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Empty Trash"
                    flat: true
                    enabled: AppCore.mail.trashCount > 0
                    onClicked: AppCore.mail.emptyTrash()
                }
            }
        }

        // Trashed message list
        ListView {
            id: trashList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: AppCore.mailTrash

            Text {
                anchors.centerIn: parent
                text: "Trash is empty"
                color: Theme.textFaint
                font.pixelSize: 14
                visible: trashList.count === 0
            }

            delegate: Rectangle {
                width: trashList.width
                height: 64
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    spacing: 14

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: model.sender
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            color: Theme.textPrimary
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            text: model.subject
                            font.pixelSize: 12
                            color: Theme.textMuted
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    Text {
                        text: model.date
                        font.pixelSize: 11
                        color: Theme.textFaint
                    }

                    Button {
                        text: "Restore"
                        flat: true
                        onClicked: AppCore.mail.restore(model.id)
                    }

                    Button {
                        text: "Delete forever"
                        flat: true
                        onClicked: AppCore.mail.purge(model.id)
                    }
                }
            }
        }
    }
}
