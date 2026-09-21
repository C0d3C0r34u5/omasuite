import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0

Rectangle {
    id: mailView
    color: Theme.bg

    property int selectedIndex: -1
    property var selectedMail: null
    property string selectedBody: ""
    property string selectedUid: ""

    function currentAccountId() {
        return Accounts.count > 0 ? Accounts.accounts[0].id : 0
    }

    function openCompose() {
        composePopup.open()
    }

    Connections {
        target: Sync
        function onMailBodyFetched(uid, body) {
            if (String(uid) === mailView.selectedUid) {
                mailView.selectedBody = body
            }
        }
    }

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
                    text: "Mail"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Refresh"
                    flat: true
                    onClicked: Sync.syncMail(mailView.currentAccountId())
                }

                Button {
                    text: "New message"
                    padding: 12
                    onClicked: composePopup.open()

                    background: Rectangle {
                        color: parent.hovered ? Theme.accentHover : Theme.accent
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Message list
            Rectangle {
                Layout.preferredWidth: 360
                Layout.fillHeight: true
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                ListView {
                    id: mailList
                    anchors.fill: parent
                    clip: true
                    model: AppCore.mailInbox
                    currentIndex: mailView.selectedIndex

                    Text {
                        anchors.centerIn: parent
                        text: "No messages yet"
                        color: Theme.textFaint
                        font.pixelSize: 14
                        visible: mailList.count === 0
                    }

                    delegate: Rectangle {
                        width: mailList.width
                        height: 78
                        color: mailList.currentIndex === index ? Theme.accentLight : "transparent"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            anchors.topMargin: 10
                            anchors.bottomMargin: 10
                            spacing: 2

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Text {
                                    text: model.sender
                                    font.pixelSize: 14
                                    font.weight: model.read ? Font.Normal : Font.DemiBold
                                    color: Theme.textPrimary
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: model.starred ? "\u2605" : "\u2606"
                                    color: model.starred ? Theme.star : Theme.textFaint
                                    font.pixelSize: 14
                                }

                                Text {
                                    text: model.date
                                    font.pixelSize: 11
                                    color: model.read ? Theme.textFaint : Theme.accent
                                }
                            }

                            Text {
                                text: model.subject
                                font.pixelSize: 13
                                font.weight: model.read ? Font.Normal : Font.DemiBold
                                color: Theme.textBody
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Text {
                                text: model.preview
                                font.pixelSize: 12
                                color: Theme.textFaint
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                mailView.selectedIndex = index
                                mailView.selectedMail = AppCore.mail.getById(model.id)
                                mailView.selectedBody = mailView.selectedMail.body
                                mailView.selectedUid = mailView.selectedMail.uid
                                if (!model.read)
                                    AppCore.mail.setRead(model.id, true)
                                if (model.uid && model.uid !== "" && mailView.selectedBody === "")
                                    Sync.fetchMailBody(mailView.currentAccountId(), model.uid)
                            }
                        }
                    }
                }
            }

            // Reading pane
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.surface

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            Layout.fillWidth: true
                            text: mailView.selectedMail ? mailView.selectedMail.subject : "Select a message"
                            font.pixelSize: 20
                            font.weight: Font.DemiBold
                            color: Theme.textPrimary
                            elide: Text.ElideRight
                        }

                        Button {
                            text: "Move to Trash"
                            flat: true
                            visible: mailView.selectedMail !== null
                            onClicked: {
                                AppCore.mail.trash(mailView.selectedMail.id)
                                mailView.selectedMail = null
                                mailView.selectedIndex = -1
                            }
                        }
                    }

                    Text {
                        text: mailView.selectedMail ? ("From: " + mailView.selectedMail.sender) : ""
                        font.pixelSize: 13
                        color: Theme.textMuted
                    }

                    Text {
                        text: mailView.selectedMail ? ("To: " + mailView.selectedMail.recipient) : ""
                        font.pixelSize: 13
                        color: Theme.textMuted
                    }

                    Text {
                        text: mailView.selectedMail ? mailView.selectedMail.date : ""
                        font.pixelSize: 12
                        color: Theme.textFaint
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4
                        color: Theme.border
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            width: parent.width
                            text: mailView.selectedBody !== "" ? mailView.selectedBody : (mailView.selectedMail ? mailView.selectedMail.body : "")
                            font.pixelSize: 14
                            color: Theme.textBody
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }

    // ---- Compose popup ----
    Popup {
        id: composePopup
        width: 620
        height: 560
        modal: true
        anchors.centerIn: parent
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle { radius: 12; color: Theme.surface }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 10

            Text {
                text: "New message"
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: Theme.textPrimary
            }

            TextField {
                id: toField
                Layout.fillWidth: true
                placeholderText: "To"
                inputMethodHints: Qt.ImhEmailCharactersOnly
            }
            TextField {
                id: subjectField
                Layout.fillWidth: true
                placeholderText: "Subject"
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                TextArea {
                    id: bodyField
                    placeholderText: "Write your message..."
                    wrapMode: TextArea.Wrap
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 8

                Button {
                    text: "Cancel"
                    flat: true
                    onClicked: composePopup.close()
                }

                Button {
                    text: "Send"
                    padding: 12
                    enabled: toField.text.length > 0
                    onClicked: {
                        Sync.sendMail(mailView.currentAccountId(), toField.text, subjectField.text, bodyField.text)
                        toField.text = ""
                        subjectField.text = ""
                        bodyField.text = ""
                        composePopup.close()
                    }

                    background: Rectangle {
                        color: parent.enabled ? (parent.hovered ? Theme.accentHover : Theme.accent) : Theme.accentDisabled
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
