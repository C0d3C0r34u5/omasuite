import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0

Rectangle {
    id: calView
    color: "#F3F4F6"

    property date viewDate: new Date(new Date().getFullYear(), new Date().getMonth(), 1)
    property var gridDays: []
    property date selectedDay: new Date()

    function currentAccountId() {
        return Accounts.count > 0 ? Accounts.accounts[0].id : 0
    }

    function startOfDay(d) {
        return new Date(d.getFullYear(), d.getMonth(), d.getDate()).getTime()
    }

    function rebuildGrid() {
        var y = viewDate.getFullYear()
        var m = viewDate.getMonth()
        var first = new Date(y, m, 1)
        var offset = first.getDay()
        gridDays = []
        for (var i = 0; i < 42; i++)
            gridDays.push(new Date(y, m, 1 - offset + i))
    }

    function monthLabel() {
        return viewDate.toLocaleDateString(Qt.locale(), "MMMM yyyy")
    }

    Component.onCompleted: rebuildGrid()

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
                    text: "Calendar"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    color: "#1F1F1F"
                }

                Item { Layout.fillWidth: true }

                Button { text: "\u2039"; flat: true; onClicked: { viewDate = new Date(viewDate.getFullYear(), viewDate.getMonth() - 1, 1); rebuildGrid() } }
                Button { text: "Today"; flat: true; onClicked: { var n = new Date(); viewDate = new Date(n.getFullYear(), n.getMonth(), 1); selectedDay = n; rebuildGrid() } }
                Button { text: "\u203A"; flat: true; onClicked: { viewDate = new Date(viewDate.getFullYear(), viewDate.getMonth() + 1, 1); rebuildGrid() } }

                Text {
                    text: monthLabel()
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    color: "#1F1F1F"
                    Layout.minimumWidth: 150
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    text: "Sync"
                    flat: true
                    onClicked: Sync.syncCalendar(calView.currentAccountId())
                }

                Button {
                    text: "New event"
                    padding: 12
                    onClicked: { eventDialog.reset(); eventDialog.open() }

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

            // Calendar grid area
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Weekday header
                Row {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30

                    Repeater {
                        model: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]

                        Text {
                            width: parent.width / 7
                            height: parent.height
                            text: modelData
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            color: "#888888"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                // Day grid
                Grid {
                    id: dayGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    columns: 7
                    rows: 6

                    Repeater {
                        model: 42

                        Rectangle {
                            property date dayDate: calView.gridDays[index] ? calView.gridDays[index] : new Date()
                            property bool inMonth: dayDate.getMonth() === calView.viewDate.getMonth()
                            property bool isToday: dayDate.getFullYear() === new Date().getFullYear()
                                                 && dayDate.getMonth() === new Date().getMonth()
                                                 && dayDate.getDate() === new Date().getDate()
                            property bool isSelected: dayDate.getFullYear() === calView.selectedDay.getFullYear()
                                                     && dayDate.getMonth() === calView.selectedDay.getMonth()
                                                     && dayDate.getDate() === calView.selectedDay.getDate()
                            property var events: calView.gridDays[index] ? AppCore.calendar.eventsForDate(calView.startOfDay(dayDate)) : []

                            width: dayGrid.width / 7
                            height: dayGrid.height / 6
                            border.width: 0.5
                            border.color: "#E4E4E4"
                            color: isSelected ? "#E8F1FB" : (inMonth ? "#FFFFFF" : "#F7F7F7")

                            Column {
                                anchors.fill: parent
                                anchors.topMargin: 6
                                anchors.leftMargin: 6
                                anchors.rightMargin: 6
                                spacing: 2

                                Text {
                                    text: dayDate.getDate()
                                    font.pixelSize: 13
                                    font.weight: isToday ? Font.Bold : Font.Normal
                                    color: isToday ? "#0F6CBD" : (inMonth ? "#333333" : "#BBBBBB")
                                    width: 22
                                    height: 22
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Repeater {
                                    model: Math.min(2, events.length)
                                    Rectangle {
                                        width: parent.width - 4
                                        height: 18
                                        radius: 3
                                        color: "#D6E7F8"
                                        Text {
                                            anchors.fill: parent
                                            anchors.leftMargin: 4
                                            anchors.rightMargin: 4
                                            text: events[index].title
                                            font.pixelSize: 11
                                            color: "#0F5C9E"
                                            elide: Text.ElideRight
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                }

                                Text {
                                    text: events.length > 2 ? ("+" + (events.length - 2) + " more") : ""
                                    font.pixelSize: 10
                                    color: "#888888"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    calView.selectedDay = dayDate
                                }
                            }
                        }
                    }
                }
            }

            // Selected day events panel
            Rectangle {
                Layout.preferredWidth: 300
                Layout.fillHeight: true
                color: "#FFFFFF"
                border.width: 1
                border.color: "#E0E0E0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 8

                    Text {
                        text: calView.selectedDay.toLocaleDateString(Qt.locale(), "dddd, MMM d")
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                        color: "#1F1F1F"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    ListView {
                        id: dayEvents
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: AppCore.calendar.eventsForDate(calView.startOfDay(calView.selectedDay))

                        Text {
                            anchors.centerIn: parent
                            text: "No events"
                            color: "#999999"
                            visible: dayEvents.count === 0
                        }

                        delegate: Rectangle {
                            width: dayEvents.width
                            height: 52
                            radius: 8
                            color: "#F3F6FA"

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 2

                                Text {
                                    text: modelData.title
                                    font.pixelSize: 13
                                    font.weight: Font.DemiBold
                                    color: "#1F1F1F"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: (modelData.allDay ? "All day" : (modelData.startDate + " - " + modelData.endDate))
                                    font.pixelSize: 11
                                    color: "#888888"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    Sync.deleteEvent(calView.currentAccountId(), modelData.id, modelData.uid)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- New event dialog ----
    Popup {
        id: eventDialog
        width: 440
        modal: true
        anchors.centerIn: parent
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        function reset() {
            titleField.text = ""
            descField.text = ""
            startHour.value = 9
            startMin.value = 0
            endHour.value = 10
            endMin.value = 0
            allDaySwitch.checked = false
        }

        background: Rectangle { radius: 12; color: "#FFFFFF" }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            Text {
                text: "New event"
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: "#1F1F1F"
            }

            TextField {
                id: titleField
                Layout.fillWidth: true
                placeholderText: "Event title"
            }

            TextArea {
                id: descField
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                placeholderText: "Description (optional)"
            }

            RowLayout { Layout.fillWidth: true; spacing: 8
                Text { text: "All day"; font.pixelSize: 13; color: "#444444" }
                Switch { id: allDaySwitch }
                Item { Layout.fillWidth: true }
            }

            GridLayout { columns: 4; Layout.fillWidth: true
                Text { text: "Start"; font.pixelSize: 12; color: "#888888" }
                SpinBox { id: startHour; from: 0; to: 23; value: 9; editable: true }
                Text { text: ":"; font.pixelSize: 12 }
                SpinBox { id: startMin; from: 0; to: 59; value: 0; editable: true }

                Text { text: "End"; font.pixelSize: 12; color: "#888888" }
                SpinBox { id: endHour; from: 0; to: 23; value: 10; editable: true }
                Text { text: ":"; font.pixelSize: 12 }
                SpinBox { id: endMin; from: 0; to: 59; value: 0; editable: true }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 8

                Button { text: "Cancel"; flat: true; onClicked: eventDialog.close() }
                Button {
                    text: "Save"
                    padding: 12
                    enabled: titleField.text.length > 0
                    onClicked: {
                        var dayStart = calView.startOfDay(calView.selectedDay)
                        var startMs = dayStart + (startHour.value * 60 + startMin.value) * 60000
                        var endMs = dayStart + (endHour.value * 60 + endMin.value) * 60000
                        Sync.createEvent(calView.currentAccountId(), titleField.text, descField.text, "",
                                         startMs, endMs, allDaySwitch.checked)
                        eventDialog.close()
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
