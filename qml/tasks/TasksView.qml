import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0

Rectangle {
    id: tasksView
    color: Theme.bg

    property int filter: 0 // 0 = all, 1 = pending, 2 = completed

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
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 12

                Text {
                    text: "Tasks"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }

                Text {
                    text: AppCore.tasks.pendingCount + " pending"
                    font.pixelSize: 13
                    color: Theme.textFaint
                }

                Item { Layout.fillWidth: true }

                TabBar {
                    id: taskFilter
                    currentIndex: tasksView.filter

                    TabButton { text: "All" }
                    TabButton { text: "Pending" }
                    TabButton { text: "Done" }

                    onCurrentIndexChanged: tasksView.filter = currentIndex
                }

                Button {
                    text: "Sync"
                    flat: true
                    onClicked: Sync.syncTasks(tasksView.currentAccountId())
                }

                Button {
                    text: "New task"
                    padding: 12
                    onClicked: { taskDialog.reset(); taskDialog.open() }

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

        // Task list
        ListView {
            id: taskList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 20
            clip: true
            spacing: 8
            model: AppCore.tasks

            Text {
                anchors.centerIn: parent
                text: "No tasks yet"
                color: Theme.textFaint
                visible: taskList.count === 0
            }

            delegate: Rectangle {
                width: taskList.width
                height: 60
                radius: 8
                color: Theme.surface
                border.width: 1
                border.color: Theme.border
                visible: {
                    if (tasksView.filter === 0) return true
                    if (tasksView.filter === 1) return !model.completed
                    return model.completed
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 12

                    CheckBox {
                        checked: model.completed
                        onToggled: AppCore.tasks.setCompleted(model.id, checked)
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        Text {
                            text: model.title
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            color: model.completed ? Theme.textFaint : Theme.textPrimary
                            font.strikeout: model.completed
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            text: model.dueDate ? ("Due " + model.dueDate) : ""
                            font.pixelSize: 11
                            color: Theme.textFaint
                        }
                    }

                    Button {
                        text: "\u2715"
                        flat: true
                        onClicked: Sync.deleteTask(tasksView.currentAccountId(), model.id, model.uid)
                    }
                }
            }
        }
    }

    // ---- New task dialog ----
    Popup {
        id: taskDialog
        width: 440
        modal: true
        anchors.centerIn: parent
        padding: 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        function reset() {
            titleField.text = ""
            descField.text = ""
            hasDue.checked = false
        }

        background: Rectangle { radius: 12; color: Theme.surface }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            Text {
                text: "New task"
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: Theme.textPrimary
            }

            TextField {
                id: titleField
                Layout.fillWidth: true
                placeholderText: "Task title"
            }

            TextArea {
                id: descField
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                placeholderText: "Description (optional)"
            }

            RowLayout { Layout.fillWidth: true; spacing: 8
                CheckBox { id: hasDue }
                Text { text: "Has due date"; font.pixelSize: 13; color: Theme.textSecondary }
                Item { Layout.fillWidth: true }
            }

            RowLayout { Layout.fillWidth: true; spacing: 8
                Text { text: "Due date"; font.pixelSize: 13; color: Theme.textSecondary }
                TextField {
                    id: dueField
                    Layout.fillWidth: true
                    placeholderText: "YYYY-MM-DD (optional)"
                    visible: hasDue.checked
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 8

                Button { text: "Cancel"; flat: true; onClicked: taskDialog.close() }
                Button {
                    text: "Save"
                    padding: 12
                    enabled: titleField.text.length > 0
                    onClicked: {
                        var due = 0
                        if (hasDue.checked && dueField.text.length > 0) {
                            var dt = new Date(dueField.text + "T00:00:00")
                            due = dt.getTime()
                        }
                        Sync.createTask(tasksView.currentAccountId(), titleField.text, descField.text, due)
                        taskDialog.close()
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
