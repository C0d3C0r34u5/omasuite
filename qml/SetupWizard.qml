import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OmaSuite 1.0
import "components"

Item {
    id: wizard

    property var selectedProvider: (AppCore.providers().length > 0) ? AppCore.providers()[0] : ({})
    property string presetImapHost: selectedProvider.imapHost ? selectedProvider.imapHost : ""
    property string presetSmtpHost: selectedProvider.smtpHost ? selectedProvider.smtpHost : ""
    property int presetImapPort: selectedProvider.imapPort ? selectedProvider.imapPort : 993
    property int presetSmtpPort: selectedProvider.smtpPort ? selectedProvider.smtpPort : 465

    property bool waitingForOAuth: false
    property var pendingOAuthData: ({})

    Connections {
        target: OAuth
        function onAuthorized(email) {
            if (wizard.waitingForOAuth) {
                wizard.waitingForOAuth = false
                Accounts.addAccount(wizard.pendingOAuthData)
                wizard.pendingOAuthData = ({})
            }
        }
        function onFailed(email, reason) {
            if (wizard.waitingForOAuth) {
                wizard.waitingForOAuth = false
                oauthErrorText.text = reason
                oauthErrorText.visible = true
            }
        }
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: welcomePage
    }

    // ---- Page 1: Welcome ----
    Component {
        id: welcomePage

        Rectangle {
            color: Theme.bg

            ColumnLayout {
                anchors.centerIn: parent
                width: 420
                spacing: 8

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 84
                    height: 84
                    radius: 20
                    color: Theme.accent

                    Text {
                        anchors.centerIn: parent
                        text: "Oma"
                        color: "white"
                        font.pixelSize: 28
                        font.bold: true
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 12
                    text: "Welcome to OmaSuite"
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Mail, calendar, contacts and tasks — all in one place."
                    font.pixelSize: 14
                    color: Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    Layout.maximumWidth: 380
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 8
                    text: "Start by adding your first email account."
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 24
                    text: "Add account"
                    padding: 14
                    onClicked: stack.push(providerPage)

                    background: Rectangle {
                        color: parent.hovered ? Theme.accentHover : Theme.accent
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    // ---- Page 2: Choose provider ----
    Component {
        id: providerPage

        Rectangle {
            color: Theme.bg

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 32
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "\u2190  Back"
                        flat: true
                        onClicked: stack.pop()
                    }

                    Text {
                        text: "Choose your provider"
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                        color: Theme.textPrimary
                    }
                }

                Text {
                    Layout.leftMargin: 4
                    text: "Select the service for the account you want to add."
                    font.pixelSize: 13
                    color: Theme.textMuted
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.topMargin: 8
                    Layout.maximumWidth: 520
                    Layout.maximumHeight: 460
                    radius: 12
                    color: "transparent"

                    Column {
                        anchors.fill: parent
                        spacing: 8

                        Repeater {
                            model: AppCore.providers()

                            ProviderButton {
                                provider: modelData
                                selected: wizard.selectedProvider.id === modelData.id
                                onClicked: {
                                    wizard.selectedProvider = modelData
                                    stack.push(detailsPage)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Page 3: Account details ----
    Component {
        id: detailsPage

        Rectangle {
            color: Theme.bg

            property string providerId: wizard.selectedProvider.id
            property string providerBrand: wizard.selectedProvider.brand

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 32
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "\u2190  Back"
                        flat: true
                        onClicked: stack.pop()
                    }

                    Text {
                        text: "Add " + detailsPage.providerBrand + " account"
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                        color: Theme.textPrimary
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: 560

                    ColumnLayout {
                        width: 560
                        spacing: 12

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: formColumn.implicitHeight + 40
                            radius: 12
                            color: Theme.surface
                            border.width: 1
                            border.color: Theme.border

                            ColumnLayout {
                                id: formColumn
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.leftMargin: 20
                                anchors.rightMargin: 20
                                anchors.topMargin: 20
                                spacing: 14

                                // Display name
                                ColumnLayout { Layout.fillWidth: true; spacing: 4
                                    Text { text: "Display name"; font.pixelSize: 12; color: Theme.textSecondary }
                                    TextField {
                                        id: nameField
                                        Layout.fillWidth: true
                                        placeholderText: "Your name"
                                        font.pixelSize: 14
                                    }
                                }

                                // Email
                                ColumnLayout { Layout.fillWidth: true; spacing: 4
                                    Text { text: "Email address"; font.pixelSize: 12; color: Theme.textSecondary }
                                    TextField {
                                        id: emailField
                                        Layout.fillWidth: true
                                        placeholderText: "you@example.com"
                                        inputMethodHints: Qt.ImhEmailCharactersOnly
                                        font.pixelSize: 14
                                        onTextChanged: {
                                            if (wizard.selectedProvider.id !== "manual") {
                                                var cal = String(wizard.selectedProvider.caldavUrl || "")
                                                var card = String(wizard.selectedProvider.carddavUrl || "")
                                                if (cal.indexOf("%1") >= 0) caldavField.text = cal.replace("%1", text)
                                                if (card.indexOf("%1") >= 0) carddavField.text = card.replace("%1", text)
                                            }
                                        }
                                    }
                                }

                                // Password
                                ColumnLayout { Layout.fillWidth: true; spacing: 4
                                    visible: authMethodCombo.currentIndex === 0
                                    Text { text: "Password or app password"; font.pixelSize: 12; color: Theme.textSecondary }
                                    TextField {
                                        id: passwordField
                                        Layout.fillWidth: true
                                        echoMode: TextInput.Password
                                        placeholderText: "Required for syncing"
                                        font.pixelSize: 14
                                    }
                                }

                                // Authentication method
                                RowLayout { Layout.fillWidth: true; spacing: 8
                                    Text { text: "Sign in with"; font.pixelSize: 13; color: Theme.textSecondary }
                                    ComboBox {
                                        id: authMethodCombo
                                        model: wizard.selectedProvider.supportsOAuth ? ["App password", "OAuth2 (browser)"] : ["App password"]
                                    }
                                    Item { Layout.fillWidth: true }
                                }

                                // OAuth client id (advanced)
                                ColumnLayout { Layout.fillWidth: true; spacing: 4
                                    visible: authMethodCombo.currentIndex === 1
                                    Text { text: "OAuth client ID (optional)"; font.pixelSize: 12; color: Theme.textSecondary }
                                    TextField {
                                        id: oauthClientIdField
                                        Layout.fillWidth: true
                                        placeholderText: "Leave blank if using a shared client"
                                        font.pixelSize: 14
                                    }
                                }

                                // OAuth client secret (only some providers, e.g. Yahoo)
                                ColumnLayout { Layout.fillWidth: true; spacing: 4
                                    visible: authMethodCombo.currentIndex === 1
                                    Text { text: "OAuth client secret (optional)"; font.pixelSize: 12; color: Theme.textSecondary }
                                    TextField {
                                        id: oauthClientSecretField
                                        Layout.fillWidth: true
                                        echoMode: TextInput.Password
                                        placeholderText: "Only required by some providers (e.g. Yahoo)"
                                        font.pixelSize: 14
                                    }
                                }

                                Text {
                                    id: oauthErrorText
                                    Layout.fillWidth: true
                                    visible: false
                                    color: Theme.danger
                                    font.pixelSize: 12
                                    wrapMode: Text.Wrap
                                }

                                // Advanced settings toggle
                                Button {
                                    text: advanced.visible ? "Hide server settings" : "Show server settings"
                                    flat: true
                                    onClicked: advanced.visible = !advanced.visible
                                }

                                // Advanced server settings
                                ColumnLayout {
                                    id: advanced
                                    Layout.fillWidth: true
                                    spacing: 12
                                    visible: wizard.selectedProvider.id === "manual"

                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 1
                                        color: Theme.border
                                    }

                                    Text { text: "Incoming (IMAP)"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                                    RowLayout { Layout.fillWidth: true; spacing: 8
                                        TextField { id: imapHostField; Layout.fillWidth: true; placeholderText: "IMAP server"; text: wizard.selectedProvider.imapHost ? wizard.selectedProvider.imapHost : "" }
                                        SpinBox { id: imapPortField; from: 1; to: 65535; value: wizard.selectedProvider.imapPort ? wizard.selectedProvider.imapPort : 993; editable: true }
                                    }

                                    Text { text: "Outgoing (SMTP)"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                                    RowLayout { Layout.fillWidth: true; spacing: 8
                                        TextField { id: smtpHostField; Layout.fillWidth: true; placeholderText: "SMTP server"; text: wizard.selectedProvider.smtpHost ? wizard.selectedProvider.smtpHost : "" }
                                        SpinBox { id: smtpPortField; from: 1; to: 65535; value: wizard.selectedProvider.smtpPort ? wizard.selectedProvider.smtpPort : 465; editable: true }
                                    }

                                    Text { text: "Calendar (CalDAV)"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                                    TextField { id: caldavField; Layout.fillWidth: true; placeholderText: "https://..."; text: wizard.selectedProvider.caldavUrl ? wizard.selectedProvider.caldavUrl : "" }

                                    Text { text: "Contacts (CardDAV)"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                                    TextField { id: carddavField; Layout.fillWidth: true; placeholderText: "https://..."; text: wizard.selectedProvider.carddavUrl ? wizard.selectedProvider.carddavUrl : "" }

                                    ColumnLayout { Layout.fillWidth: true; spacing: 4
                                        visible: wizard.selectedProvider.id === "exchange"
                                        Text { text: "Exchange Web Services URL"; font.pixelSize: 13; font.weight: Font.DemiBold; color: Theme.textPrimary }
                                        TextField { id: ewsUrlField; Layout.fillWidth: true; placeholderText: "https://outlook.office365.com/EWS/Exchange.asmx"; text: wizard.selectedProvider.ewsUrl ? wizard.selectedProvider.ewsUrl : "" }
                                    }

                                    RowLayout { Layout.fillWidth: true; spacing: 8
                                        CheckBox { id: allowUntrustedBox }
                                        Text { text: "Trust invalid TLS certificate (self-signed servers)"; font.pixelSize: 12; color: Theme.textMuted }
                                    }
                                }
                            }
                        }

                        Button {
                            Layout.fillWidth: true
                            text: authMethodCombo.currentIndex === 1 ? "Sign in with " + detailsPage.providerBrand : "Add account"
                            padding: 14
                            enabled: emailField.text.length > 0 && !wizard.waitingForOAuth

                            onClicked: {
                                var data = {
                                    name: nameField.text.length > 0 ? nameField.text : emailField.text,
                                    email: emailField.text,
                                    provider: wizard.selectedProvider.id,
                                    imapHost: imapHostField.text,
                                    imapPort: imapPortField.value,
                                    smtpHost: smtpHostField.text,
                                    smtpPort: smtpPortField.value,
                                    caldavUrl: caldavField.text,
                                    carddavUrl: carddavField.text,
                                    password: passwordField.text,
                                    allowUntrusted: allowUntrustedBox.checked,
                                    authMethod: authMethodCombo.currentIndex === 1 ? "oauth2" : "password",
                                    oauthClientId: oauthClientIdField.text,
                                    oauthClientSecret: oauthClientSecretField.text,
                                    ewsUrl: ewsUrlField.text
                                }

                                if (authMethodCombo.currentIndex === 1) {
                                    wizard.waitingForOAuth = true
                                    wizard.pendingOAuthData = data
                                    oauthErrorText.visible = false
                                    OAuth.authorize(wizard.selectedProvider.id, emailField.text, oauthClientIdField.text, oauthClientSecretField.text)
                                } else {
                                    Accounts.addAccount(data)
                                    stack.push(donePage)
                                }
                            }

                            background: Rectangle {
                                color: parent.enabled ? (parent.hovered ? Theme.accentHover : Theme.accent) : Theme.accentDisabled
                                radius: 8
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font.pixelSize: 15
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Page 4: Done ----
    Component {
        id: donePage

        Rectangle {
            color: Theme.bg

            ColumnLayout {
                anchors.centerIn: parent
                width: 420
                spacing: 8

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 84
                    height: 84
                    radius: 42
                    color: Theme.accent

                    Text {
                        anchors.centerIn: parent
                        text: "\u2713"
                        color: "white"
                        font.pixelSize: 40
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 12
                    text: "You're all set!"
                    font.pixelSize: 26
                    font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Your account has been added. You can add more accounts anytime from Settings."
                    font.pixelSize: 13
                    color: Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    Layout.maximumWidth: 360
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 24
                    text: "Go to OmaSuite"
                    padding: 14
                    onClicked: stack.clear()

                    background: Rectangle {
                        color: parent.hovered ? Theme.accentHover : Theme.accent
                        radius: 8
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
