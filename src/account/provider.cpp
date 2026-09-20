#include "provider.h"

namespace Provider {

QString typeToString(Type t)
{
    switch (t) {
    case Gmail:    return QStringLiteral("gmail");
    case Outlook:  return QStringLiteral("outlook");
    case Yahoo:    return QStringLiteral("yahoo");
    case Apple:    return QStringLiteral("apple");
    case Exchange: return QStringLiteral("exchange");
    case Manual:
    default:       return QStringLiteral("manual");
    }
}

Type stringToType(const QString &s)
{
    if (s == QStringLiteral("gmail"))    return Gmail;
    if (s == QStringLiteral("outlook"))  return Outlook;
    if (s == QStringLiteral("yahoo"))    return Yahoo;
    if (s == QStringLiteral("apple"))    return Apple;
    if (s == QStringLiteral("exchange")) return Exchange;
    return Manual;
}

QVariantList presets()
{
    QVariantList list;
    list << preset(Gmail)
         << preset(Outlook)
         << preset(Yahoo)
         << preset(Apple)
         << preset(Exchange)
         << preset(Manual);
    return list;
}

QVariantMap preset(Type t)
{
    QVariantMap m;
    switch (t) {
    case Gmail:
        m[QStringLiteral("id")] = QStringLiteral("gmail");
        m[QStringLiteral("name")] = QStringLiteral("Google");
        m[QStringLiteral("brand")] = QStringLiteral("Gmail");
        m[QStringLiteral("color")] = QStringLiteral("#EA4335");
        m[QStringLiteral("letter")] = QStringLiteral("G");
        m[QStringLiteral("description")] = QStringLiteral("Gmail and Google Workspace");
        m[QStringLiteral("imapHost")] = QStringLiteral("imap.gmail.com");
        m[QStringLiteral("imapPort")] = 993;
        m[QStringLiteral("smtpHost")] = QStringLiteral("smtp.gmail.com");
        m[QStringLiteral("smtpPort")] = 465;
        m[QStringLiteral("caldavUrl")] = QStringLiteral("https://apidata.googleusercontent.com/caldav/v2/%1/events");
        m[QStringLiteral("carddavUrl")] = QStringLiteral("https://www.googleapis.com/carddav/v1/principals/%1/lists/default/");
        m[QStringLiteral("supportsMail")] = true;
        m[QStringLiteral("supportsCalendar")] = true;
        m[QStringLiteral("supportsContacts")] = true;
        m[QStringLiteral("supportsOAuth")] = true;
        m[QStringLiteral("authUrl")] = QStringLiteral("https://accounts.google.com/o/oauth2/v2/auth");
        m[QStringLiteral("tokenUrl")] = QStringLiteral("https://oauth2.googleapis.com/token");
        m[QStringLiteral("scopes")] = QStringLiteral("openid email https://mail.google.com/ https://www.googleapis.com/auth/calendar https://www.googleapis.com/auth/contacts");
        break;

    case Outlook:
        m[QStringLiteral("id")] = QStringLiteral("outlook");
        m[QStringLiteral("name")] = QStringLiteral("Microsoft");
        m[QStringLiteral("brand")] = QStringLiteral("Outlook");
        m[QStringLiteral("color")] = QStringLiteral("#0078D4");
        m[QStringLiteral("letter")] = QStringLiteral("O");
        m[QStringLiteral("description")] = QStringLiteral("Outlook.com and Microsoft 365");
        m[QStringLiteral("imapHost")] = QStringLiteral("outlook.office365.com");
        m[QStringLiteral("imapPort")] = 993;
        m[QStringLiteral("smtpHost")] = QStringLiteral("smtp.office365.com");
        m[QStringLiteral("smtpPort")] = 587;
        m[QStringLiteral("caldavUrl")] = QString();
        m[QStringLiteral("carddavUrl")] = QString();
        m[QStringLiteral("supportsMail")] = true;
        m[QStringLiteral("supportsCalendar")] = false;
        m[QStringLiteral("supportsContacts")] = false;
        m[QStringLiteral("supportsOAuth")] = true;
        m[QStringLiteral("authUrl")] = QStringLiteral("https://login.microsoftonline.com/common/oauth2/v2.0/authorize");
        m[QStringLiteral("tokenUrl")] = QStringLiteral("https://login.microsoftonline.com/common/oauth2/v2.0/token");
        m[QStringLiteral("scopes")] = QStringLiteral("openid email offline_access https://outlook.office.com/IMAP.AccessAsUser.All https://outlook.office.com/SMTP.Send https://outlook.office.com/EWS.AccessAsUser.All https://outlook.office.com/Calendars.ReadWrite https://outlook.office.com/Contacts.ReadWrite https://outlook.office.com/Tasks.ReadWrite");
        break;

    case Yahoo:
        m[QStringLiteral("id")] = QStringLiteral("yahoo");
        m[QStringLiteral("name")] = QStringLiteral("Yahoo");
        m[QStringLiteral("brand")] = QStringLiteral("Yahoo Mail");
        m[QStringLiteral("color")] = QStringLiteral("#6001D2");
        m[QStringLiteral("letter")] = QStringLiteral("Y");
        m[QStringLiteral("description")] = QStringLiteral("Yahoo Mail");
        m[QStringLiteral("imapHost")] = QStringLiteral("imap.mail.yahoo.com");
        m[QStringLiteral("imapPort")] = 993;
        m[QStringLiteral("smtpHost")] = QStringLiteral("smtp.mail.yahoo.com");
        m[QStringLiteral("smtpPort")] = 465;
        m[QStringLiteral("caldavUrl")] = QStringLiteral("https://caldav.calendar.yahoo.com/");
        m[QStringLiteral("carddavUrl")] = QString();
        m[QStringLiteral("supportsMail")] = true;
        m[QStringLiteral("supportsCalendar")] = true;
        m[QStringLiteral("supportsContacts")] = false;
        m[QStringLiteral("supportsOAuth")] = true;
        m[QStringLiteral("authUrl")] = QStringLiteral("https://api.login.yahoo.com/oauth2/request_auth");
        m[QStringLiteral("tokenUrl")] = QStringLiteral("https://api.login.yahoo.com/oauth2/get_token");
        m[QStringLiteral("scopes")] = QStringLiteral("openid email sdct-r");
        break;

    case Apple:
        m[QStringLiteral("id")] = QStringLiteral("apple");
        m[QStringLiteral("name")] = QStringLiteral("Apple");
        m[QStringLiteral("brand")] = QStringLiteral("iCloud Mail");
        m[QStringLiteral("color")] = QStringLiteral("#555555");
        m[QStringLiteral("letter")] = QStringLiteral("");
        m[QStringLiteral("description")] = QStringLiteral("iCloud Mail, Calendar and Contacts");
        m[QStringLiteral("imapHost")] = QStringLiteral("imap.mail.me.com");
        m[QStringLiteral("imapPort")] = 993;
        m[QStringLiteral("smtpHost")] = QStringLiteral("smtp.mail.me.com");
        m[QStringLiteral("smtpPort")] = 587;
        m[QStringLiteral("caldavUrl")] = QStringLiteral("https://caldav.icloud.com/");
        m[QStringLiteral("carddavUrl")] = QStringLiteral("https://contacts.icloud.com/");
        m[QStringLiteral("supportsMail")] = true;
        m[QStringLiteral("supportsCalendar")] = true;
        m[QStringLiteral("supportsContacts")] = true;
        break;

    case Exchange:
        m[QStringLiteral("id")] = QStringLiteral("exchange");
        m[QStringLiteral("name")] = QStringLiteral("Microsoft");
        m[QStringLiteral("brand")] = QStringLiteral("Exchange");
        m[QStringLiteral("color")] = QStringLiteral("#00A4EF");
        m[QStringLiteral("letter")] = QStringLiteral("X");
        m[QStringLiteral("description")] = QStringLiteral("Microsoft Exchange (EWS)");
        m[QStringLiteral("imapHost")] = QStringLiteral("outlook.office365.com");
        m[QStringLiteral("imapPort")] = 993;
        m[QStringLiteral("smtpHost")] = QStringLiteral("smtp.office365.com");
        m[QStringLiteral("smtpPort")] = 587;
        m[QStringLiteral("caldavUrl")] = QString();
        m[QStringLiteral("carddavUrl")] = QString();
        m[QStringLiteral("supportsMail")] = true;
        m[QStringLiteral("supportsCalendar")] = false;
        m[QStringLiteral("supportsContacts")] = false;
        m[QStringLiteral("supportsOAuth")] = true;
        m[QStringLiteral("authUrl")] = QStringLiteral("https://login.microsoftonline.com/common/oauth2/v2.0/authorize");
        m[QStringLiteral("tokenUrl")] = QStringLiteral("https://login.microsoftonline.com/common/oauth2/v2.0/token");
        m[QStringLiteral("scopes")] = QStringLiteral("openid email offline_access https://outlook.office.com/EWS.AccessAsUser.All");
        m[QStringLiteral("ewsUrl")] = QStringLiteral("https://outlook.office365.com/EWS/Exchange.asmx");
        break;

    case Manual:
    default:
        m[QStringLiteral("id")] = QStringLiteral("manual");
        m[QStringLiteral("name")] = QStringLiteral("Other");
        m[QStringLiteral("brand")] = QStringLiteral("Manual setup");
        m[QStringLiteral("color")] = QStringLiteral("#7A7A7A");
        m[QStringLiteral("letter")] = QStringLiteral("+");
        m[QStringLiteral("description")] = QStringLiteral("Configure any IMAP / SMTP / CalDAV / CardDAV server");
        m[QStringLiteral("imapHost")] = QString();
        m[QStringLiteral("imapPort")] = 993;
        m[QStringLiteral("smtpHost")] = QString();
        m[QStringLiteral("smtpPort")] = 465;
        m[QStringLiteral("caldavUrl")] = QString();
        m[QStringLiteral("carddavUrl")] = QString();
        m[QStringLiteral("supportsMail")] = true;
        m[QStringLiteral("supportsCalendar")] = true;
        m[QStringLiteral("supportsContacts")] = true;
        break;
    }

    return m;
}

QVariantMap resolveUrls(const QVariantMap &preset, const QString &email)
{
    QVariantMap m = preset;
    const QString caldav = preset.value(QStringLiteral("caldavUrl")).toString();
    const QString carddav = preset.value(QStringLiteral("carddavUrl")).toString();

    if (caldav.contains(QStringLiteral("%1")))
        m[QStringLiteral("caldavUrl")] = caldav.arg(email);
    if (carddav.contains(QStringLiteral("%1")))
        m[QStringLiteral("carddavUrl")] = carddav.arg(email);

    return m;
}

}
