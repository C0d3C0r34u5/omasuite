#include "carddavclient.h"
#include "net/vcardutils.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QDebug>
#include <QUuid>

namespace {
const QByteArray kBearerPrefix = "Bearer ";
}

QByteArray CarddavClient::authHeader(const QString &user, const QString &pass) const
{
    if (!m_bearer.isEmpty())
        return kBearerPrefix + m_bearer.toUtf8();
    const QByteArray creds = (user + QLatin1Char(':') + pass).toUtf8();
    return "Basic " + creds.toBase64();
}

CarddavClient::CarddavClient(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void CarddavClient::fetchContacts(const QString &url, const QString &user, const QString &pass)
{
    m_user = user;
    m_pass = pass;

    QNetworkRequest req{QUrl(url)};
    req.setRawHeader("Authorization", authHeader(user, pass));
    req.setRawHeader("Depth", "1");
    req.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    const QByteArray body =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:carddav\">"
        "<d:prop><c:address-data/></d:prop>"
        "</d:propfind>";

    QNetworkReply *reply = m_nam->sendCustomRequest(req, "PROPFIND", body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit failed(QStringLiteral("CardDAV request failed: %1").arg(reply->errorString()));
            return;
        }

        const QByteArray xml = reply->readAll();
        QXmlStreamReader reader(xml);
        QVariantList contacts;
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isStartElement() && reader.name().toString() == QLatin1String("address-data")) {
                const QString vcf = reader.readElementText();
                contacts.append(VCardUtils::parseVCards(vcf));
            }
        }
        emit contactsFetched(contacts);
    });
}

void CarddavClient::createContact(const QString &url, const QString &user, const QString &pass,
                                  const QVariantMap &contact)
{
    m_user = user;
    m_pass = pass;

    QString uid = contact.value(QStringLiteral("uid")).toString();
    if (uid.isEmpty())
        uid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QVariantMap copy = contact;
    copy[QStringLiteral("uid")] = uid;
    const QByteArray vcf = VCardUtils::buildVCard(copy).toUtf8();

    QUrl target(url);
    if (!target.path().endsWith(QLatin1Char('/')))
        target.setPath(target.path() + QLatin1Char('/'));
    target = target.resolved(QUrl(uid + QStringLiteral(".vcf")));

    QNetworkRequest req(target);
    req.setRawHeader("Authorization", authHeader(user, pass));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/vcard; charset=utf-8"));

    QNetworkReply *reply = m_nam->put(req, vcf);
    connect(reply, &QNetworkReply::finished, this, [this, reply, uid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit failed(QStringLiteral("CardDAV create failed: %1").arg(reply->errorString()));
            return;
        }
        emit contactCreated(uid);
    });
}

void CarddavClient::deleteContact(const QString &url, const QString &user, const QString &pass,
                                  const QString &uid)
{
    m_user = user;
    m_pass = pass;

    QUrl target(url);
    if (!target.path().endsWith(QLatin1Char('/')))
        target.setPath(target.path() + QLatin1Char('/'));
    target = target.resolved(QUrl(uid + QStringLiteral(".vcf")));

    QNetworkRequest req(target);
    req.setRawHeader("Authorization", authHeader(user, pass));

    QNetworkReply *reply = m_nam->sendCustomRequest(req, "DELETE");
    connect(reply, &QNetworkReply::finished, this, [this, reply, uid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit failed(QStringLiteral("CardDAV delete failed: %1").arg(reply->errorString()));
            return;
        }
        emit contactDeleted(uid);
    });
}
