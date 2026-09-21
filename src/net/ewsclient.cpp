#include "ewsclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QDebug>

namespace {

QString xmlEscape(const QString &s)
{
    QString out = s;
    out.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    out.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    out.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    out.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    return out;
}

QByteArray envelope(const QByteArray &body)
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
           "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" "
           "xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\" "
           "xmlns:m=\"http://schemas.microsoft.com/exchange/services/2006/messages\">"
           "<soap:Header><t:RequestServerVersion Version=\"Exchange2013\"/></soap:Header>"
           "<soap:Body>" + body + "</soap:Body></soap:Envelope>";
}

qint64 isoToMs(const QString &s)
{
    const QDateTime dt = QDateTime::fromString(s, Qt::ISODate);
    return dt.isValid() ? dt.toMSecsSinceEpoch() : 0;
}

QString isoToDisplay(const QString &s)
{
    const QDateTime dt = QDateTime::fromString(s, Qt::ISODate);
    return dt.isValid() ? dt.toString(QStringLiteral("MMM d, h:mm AP")) : s;
}

}

EwsClient::EwsClient(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void EwsClient::post(const QByteArray &soap, std::function<void(QXmlStreamReader &)> parser)
{
    if (m_endpoint.isEmpty()) {
        emit failed(QStringLiteral("No EWS endpoint configured"));
        return;
    }

    QNetworkRequest req{QUrl(m_endpoint)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/xml; charset=utf-8"));
    if (!m_bearer.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_bearer.toUtf8());
    else if (!m_user.isEmpty())
        req.setRawHeader("Authorization", "Basic " + (m_user + QLatin1Char(':') + m_pass).toUtf8().toBase64());

    QNetworkReply *reply = m_nam->post(req, soap);
    connect(reply, &QNetworkReply::finished, this, [this, reply, parser]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
                emit authenticationFailed();
                return;
            }
            emit failed(QStringLiteral("EWS request failed: %1").arg(reply->errorString()));
            return;
        }

        const QByteArray xml = reply->readAll();
        QXmlStreamReader r(xml);

        // Check for a failure ResponseCode anywhere in the response.
        QString responseClass;
        QString responseCode;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement()) {
                const QString n = r.name().toString();
                if (n == QLatin1String("ResponseCode"))
                    responseCode = r.readElementText();
                else if (n == QLatin1String("ResponseClass"))
                    responseClass = r.readElementText();
            }
        }
        if (!responseCode.isEmpty() && responseCode != QLatin1String("NoError") &&
            responseCode != QLatin1String("ErrorItemNotFound")) {
            emit failed(QStringLiteral("EWS error: %1").arg(responseCode));
            return;
        }
        Q_UNUSED(responseClass);

        // Re-parse from the start for the actual data.
        QXmlStreamReader r2(xml);
        parser(r2);
    });
}

// Generic item collector driven by an item tag.
static QVariantList collectItems(QXmlStreamReader &r, const QString &itemTag)
{
    QVariantList items;
    QVariantMap cur;
    bool inItem = false;
    QString currentElement;
    bool inFrom = false;
    QString fromName, fromEmail;
    int section = 0; // 0 = none, 1 = email addresses, 2 = phone numbers

    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            const QString n = r.name().toString();
            if (n == itemTag) {
                inItem = true;
                cur.clear();
                fromName.clear();
                fromEmail.clear();
            } else if (inItem) {
                currentElement = n;
                if (n == QLatin1String("From"))
                    inFrom = true;
                else if (n == QLatin1String("EmailAddresses"))
                    section = 1;
                else if (n == QLatin1String("PhoneNumbers"))
                    section = 2;
                else if (n == QLatin1String("ItemId")) {
                    const QXmlStreamAttributes attrs = r.attributes();
                    if (attrs.hasAttribute(QStringLiteral("Id")))
                        cur[QStringLiteral("uid")] = attrs.value(QStringLiteral("Id")).toString();
                }
            }
        } else if (r.isEndElement()) {
            const QString n = r.name().toString();
            if (n == itemTag) {
                if (cur.value(QStringLiteral("from")).toString().isEmpty()) {
                    cur[QStringLiteral("from")] = fromName.isEmpty() ? fromEmail : fromName;
                    if (cur.value(QStringLiteral("from")).toString().isEmpty())
                        cur[QStringLiteral("from")] = fromEmail;
                }
                items.append(cur);
                inItem = false;
            } else if (inItem && n == QLatin1String("From")) {
                inFrom = false;
            } else if (inItem && (n == QLatin1String("EmailAddresses") || n == QLatin1String("PhoneNumbers"))) {
                section = 0;
            }
        } else if (r.isCharacters() && inItem) {
            const QString txt = r.text().toString().trimmed();
            if (txt.isEmpty())
                continue;
            if (inFrom) {
                if (currentElement == QLatin1String("Name")) fromName = txt;
                else if (currentElement == QLatin1String("EmailAddress")) fromEmail = txt;
            } else if (currentElement == QLatin1String("Subject")) cur[QStringLiteral("subject")] = txt;
            else if (currentElement == QLatin1String("DateTimeReceived")) {
                cur[QStringLiteral("date")] = isoToDisplay(txt);
                cur[QStringLiteral("timestamp")] = isoToMs(txt);
            }
            else if (currentElement == QLatin1String("DateTimeSent")) {
                cur[QStringLiteral("date")] = isoToDisplay(txt);
                cur[QStringLiteral("timestamp")] = isoToMs(txt);
            }
            else if (currentElement == QLatin1String("IsRead")) cur[QStringLiteral("seen")] = (txt == QStringLiteral("true"));
            else if (currentElement == QLatin1String("Body")) cur[QStringLiteral("body")] = txt;
            else if (currentElement == QLatin1String("Start")) cur[QStringLiteral("start")] = isoToMs(txt);
            else if (currentElement == QLatin1String("End")) cur[QStringLiteral("end")] = isoToMs(txt);
            else if (currentElement == QLatin1String("Location")) cur[QStringLiteral("location")] = txt;
            else if (currentElement == QLatin1String("IsAllDayEvent")) cur[QStringLiteral("allDay")] = (txt == QStringLiteral("true"));
            else if (currentElement == QLatin1String("DisplayName")) cur[QStringLiteral("displayName")] = txt;
            else if (currentElement == QLatin1String("Entry") && section == 1) {
                if (cur.value(QStringLiteral("email")).toString().isEmpty())
                    cur[QStringLiteral("email")] = txt;
            }
            else if (currentElement == QLatin1String("Entry") && section == 2) {
                if (cur.value(QStringLiteral("phone")).toString().isEmpty())
                    cur[QStringLiteral("phone")] = txt;
            }
            else if (currentElement == QLatin1String("CompanyName")) cur[QStringLiteral("organization")] = txt;
            else if (currentElement == QLatin1String("DueDate")) cur[QStringLiteral("due")] = isoToMs(txt);
            else if (currentElement == QLatin1String("Status")) cur[QStringLiteral("status")] = txt;
        }
    }
    return items;
}

void EwsClient::fetchMail(const QString &folder, int limit)
{
    Q_UNUSED(limit);
    const QByteArray body =
        "<m:FindItem Traversal=\"Shallow\">"
        "<m:ItemShape><t:BaseShape>Default</t:BaseShape></m:ItemShape>"
        "<m:ParentFolderIds><t:DistinguishedFolderId Id=\"" + folder.toUtf8() + "\"/></m:ParentFolderIds>"
        "</m:FindItem>";

    post(envelope(body), [this](QXmlStreamReader &r) {
        const QVariantList items = collectItems(r, QStringLiteral("Message"));
        emit messagesFetched(items);
    });
}

void EwsClient::sendMail(const QString &from, const QString &to, const QString &subject,
                         const QString &body)
{
    Q_UNUSED(from);
    const QByteArray soap =
        "<m:CreateItem MessageDisposition=\"SendAndSaveCopy\">"
        "<m:Items><t:Message>"
        "<t:Subject>" + xmlEscape(subject).toUtf8() + "</t:Subject>"
        "<t:Body BodyType=\"Text\">" + xmlEscape(body).toUtf8() + "</t:Body>"
        "<t:ToRecipients><t:Mailbox><t:EmailAddress>" + xmlEscape(to).toUtf8() + "</t:EmailAddress></t:Mailbox></t:ToRecipients>"
        "</t:Message></m:Items>"
        "</m:CreateItem>";

    post(envelope(soap), [this](QXmlStreamReader &) {
        emit sent();
    });
}

void EwsClient::fetchCalendar(qint64 start, qint64 end)
{
    const QString startIso = QDateTime::fromMSecsSinceEpoch(start).toUTC().toString(Qt::ISODate);
    const QString endIso = QDateTime::fromMSecsSinceEpoch(end).toUTC().toString(Qt::ISODate);

    const QByteArray body =
        "<m:FindItem Traversal=\"Shallow\">"
        "<m:ItemShape><t:BaseShape>Default</t:BaseShape></m:ItemShape>"
        "<m:CalendarView StartDate=\"" + startIso.toUtf8() + "\" EndDate=\"" + endIso.toUtf8() + "\"/>"
        "<m:ParentFolderIds><t:DistinguishedFolderId Id=\"calendar\"/></m:ParentFolderIds>"
        "</m:FindItem>";

    post(envelope(body), [this](QXmlStreamReader &r) {
        const QVariantList items = collectItems(r, QStringLiteral("CalendarItem"));
        emit eventsFetched(items);
    });
}

void EwsClient::fetchContacts()
{
    const QByteArray body =
        "<m:FindItem Traversal=\"Shallow\">"
        "<m:ItemShape><t:BaseShape>Default</t:BaseShape></m:ItemShape>"
        "<m:ParentFolderIds><t:DistinguishedFolderId Id=\"contacts\"/></m:ParentFolderIds>"
        "</m:FindItem>";

    post(envelope(body), [this](QXmlStreamReader &r) {
        const QVariantList items = collectItems(r, QStringLiteral("Contact"));
        emit contactsFetched(items);
    });
}

void EwsClient::fetchTasks()
{
    const QByteArray body =
        "<m:FindItem Traversal=\"Shallow\">"
        "<m:ItemShape><t:BaseShape>Default</t:BaseShape></m:ItemShape>"
        "<m:ParentFolderIds><t:DistinguishedFolderId Id=\"tasks\"/></m:ParentFolderIds>"
        "</m:FindItem>";

    post(envelope(body), [this](QXmlStreamReader &r) {
        const QVariantList items = collectItems(r, QStringLiteral("Task"));
        emit tasksFetched(items);
    });
}
