import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0
import "components"
import "mail"
import "calendar"
import "contacts"
import "tasks"

Rectangle {
    id: main
    color: "#F3F4F6"

    property int navIndex: 0

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---------- Sidebar ----------
        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: "#ECEEF1"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                // App brand
                RowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 8
                    spacing: 10

                    Rectangle {
                        width: 34
                        height: 34
                        radius: 8
                        color: "#0F6CBD"

                        Text {
                            anchors.centerIn: parent
                            text: "Oma"
                            color: "white"
                            font.pixelSize: 13
                            font.bold: true
                        }
                    }

                    Text {
                        text: "OmaSuite"
                        font.pixelSize: 17
                        font.weight: Font.DemiBold
                        color: "#1F1F1F"
                    }
                }

                // Navigation
                SidebarButton {
                    label: "Mail"
                    symbol: "\u2709"
                    active: main.navIndex === 0
                    onClicked: main.navIndex = 0
                }

                SidebarButton {
                    label: "Calendar"
                    symbol: "\u25A8"
                    active: main.navIndex === 1
                    onClicked: main.navIndex = 1
                }

                SidebarButton {
                    label: "Contacts"
                    symbol: "\u263A"
                    active: main.navIndex === 2
                    onClicked: main.navIndex = 2
                }

                SidebarButton {
                    label: "Tasks"
                    symbol: "\u2713"
                    active: main.navIndex === 3
                    onClicked: main.navIndex = 3
                }

                Item { Layout.fillHeight: true }

                // Accounts section
                Text {
                    text: "Accounts"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    color: "#888888"
                    Layout.leftMargin: 12
                    Layout.bottomMargin: 2
                }

                Repeater {
                    model: Accounts.accounts

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 8
                        Layout.topMargin: 2
                        Layout.bottomMargin: 2
                        spacing: 8

                        Rectangle {
                            width: 22
                            height: 22
                            radius: 11
                            color: modelData.color

                            Text {
                                anchors.centerIn: parent
                                text: modelData.initials
                                color: "white"
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }

                        Text {
                            text: modelData.email
                            font.pixelSize: 12
                            color: "#444444"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }

                // Settings
                SidebarButton {
                    label: "Add account"
                    symbol: "+"
                    onClicked: addAccountDialog.open()
                }

                // Sync status
                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: 8
                    Layout.leftMargin: 8
                    Layout.rightMargin: 8
                    radius: 8
                    color: "#E0E2E6"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 4

                        Button {
                            Layout.fillWidth: true
                            text: "Sync all"
                            onClicked: Sync.syncAll()
                        }

                        Text {
                            Layout.fillWidth: true
                            text: Sync.status
                            font.pixelSize: 11
                            color: "#666666"
                            elide: Text.ElideRight
                            visible: Sync.status !== ""
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }

        // ---------- Content ----------
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: main.navIndex

            MailView {}
            CalendarView {}
            ContactsView {}
            TasksView {}
        }
    }

    // Add account dialog (reuses the wizard)
    Popup {
        id: addAccountDialog
        width: 720
        height: 600
        modal: true
        anchors.centerIn: parent
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 12
            color: "#F3F4F6"
        }

        SetupWizard {
            anchors.fill: parent
            onVisibleChanged: {}
        }
    }
}
