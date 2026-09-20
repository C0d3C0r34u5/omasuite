import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0

Rectangle {
    id: contactsView
    color: "#F3F4F6"

    property int selectedIndex: -1
    property var selectedContact: null

    function currentAccountId() {
        return Accounts.count > 0 ? Accounts.accounts[0].id : 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "#FFFFFF"
            border.width: 1
            border.color: "#E0E0E0"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 12

                Text {
                    text: "Contacts"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    color: "#1F1F1F"
                }

                Item { Layout.fillWidth: true }

                TextField {
                    id: searchField
                    placeholderText: "Search contacts..."
                    Layout.preferredWidth: 220
                    onTextChanged: AppCore.contacts.setFilter(text)
                }

                Button {
                    text: "Sync"
                    flat: true
                    onClicked: Sync.syncContacts(contactsView.currentAccountId())
                }

                Button {
                    text: "New contact"
                    padding: 12
                    onClicked: { contactDialog.reset(); contactDialog.open() }

                    background: Rectangle {
                        color: parent.hovered ? "#0A5CAD" : "#0F6CBD"
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

            // Contact list
            Rectangle {
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                color: "#FFFFFF"
                border.width: 1
                border.color: "#E0E0E0"

                ListView {
                    id: contactList
                    anchors.fill: parent
                    clip: true
                    model: AppCore.contacts
                    currentIndex: contactsView.selectedIndex

                    Text {
                        anchors.centerIn: parent
                        text: "No contacts yet"
                        color: "#999999"
                        visible: contactList.count === 0
                    }

                    delegate: Rectangle {
                        width: contactList.width
                        height: 64
                        color: contactList.currentIndex === index ? "#E8F1FB" : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            Rectangle {
                                width: 40
                                height: 40
                                radius: 20
                                color: "#0F6CBD"

                                Text {
                                    anchors.centerIn: parent
                                    text: model.initials
                                    color: "white"
                                    font.pixelSize: 15
                                    font.weight: Font.DemiBold
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1

                                Text {
                                    text: model.fullName
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                    color: "#1F1F1F"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: model.email
                                    font.pixelSize: 12
                                    color: "#888888"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                contactsView.selectedIndex = index
                                contactsView.selectedContact = AppCore.contacts.get(index)
                            }
                        }
                    }
                }
            }

            // Detail pane
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#FFFFFF"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 10

                    Rectangle {
                        width: 72
                        height: 72
                        radius: 36
                        color: "#0F6CBD"

                        Text {
                            anchors.centerIn: parent
                            text: contactsView.selectedContact ? AppCore.contacts.get(contactsView.selectedIndex).fullName.charAt(0).toUpperCase() : "?"
                            color: "white"
                            font.pixelSize: 28
                            font.weight: Font.DemiBold
                        }
                    }

                    Text {
                        text: contactsView.selectedContact ? contactsView.selectedContact.fullName : "Select a contact"
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                        color: "#1F1F1F"
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: "#E0E0E0"
                    }

                    GridLayout {
                        columns: 2
                        Layout.fillWidth: true
                        columnSpacing: 12
                        rowSpacing: 12

                        Text { text: "Email"; font.pixelSize: 12; color: "#888888" }
                        Text { text: contactsView.selectedContact ? contactsView.selectedContact.email : ""; font.pixelSize: 14; color: "#1F1F1F" }

                        Text { text: "Phone"; font.pixelSize: 12; color: "#888888" }
                        Text { text: contactsView.selectedContact ? contactsView.selectedContact.phone : ""; font.pixelSize: 14; color: "#1F1F1F" }

                        Text { text: "Organization"; font.pixelSize: 12; color: "#888888" }
                        Text { text: contactsView.selectedContact ? contactsView.selectedContact.organization : ""; font.pixelSize: 14; color: "#1F1F1F" }
                    }

                    Button {
                        text: "Delete contact"
                        flat: true
                        visible: contactsView.selectedContact !== null
                        onClicked: {
                            var c = contactsView.selectedContact
                            Sync.deleteContact(contactsView.currentAccountId(), c.id, c.uid)
                            contactsView.selectedContact = null
                            contactsView.selectedIndex = -1
                        }
                    }
                }
            }
        }
    }

    // ---- New contact dialog ----
    Popup {
        id: contactDialog
        width: 420
        modal: true
        anchors.centerIn: parent
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        function reset() {
            firstField.text = ""
            lastField.text = ""
            emailField.text = ""
            phoneField.text = ""
            orgField.text = ""
        }

        background: Rectangle { radius: 12; color: "#FFFFFF" }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 10

            Text {
                text: "New contact"
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: "#1F1F1F"
            }

            RowLayout { Layout.fillWidth: true; spacing: 8
                TextField { id: firstField; Layout.fillWidth: true; placeholderText: "First name" }
                TextField { id: lastField; Layout.fillWidth: true; placeholderText: "Last name" }
            }

            TextField { id: emailField; Layout.fillWidth: true; placeholderText: "Email"; inputMethodHints: Qt.ImhEmailCharactersOnly }
            TextField { id: phoneField; Layout.fillWidth: true; placeholderText: "Phone" }
            TextField { id: orgField; Layout.fillWidth: true; placeholderText: "Organization" }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 8

                Button { text: "Cancel"; flat: true; onClicked: contactDialog.close() }
                Button {
                    text: "Save"
                    padding: 12
                    enabled: emailField.text.length > 0 || firstField.text.length > 0
                    onClicked: {
                        Sync.createContact(contactsView.currentAccountId(), firstField.text, lastField.text,
                                           emailField.text, phoneField.text, orgField.text)
                        contactDialog.close()
                    }

                    background: Rectangle {
                        color: parent.enabled ? (parent.hovered ? "#0A5CAD" : "#0F6CBD") : "#B0B0B0"
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
