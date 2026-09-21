#include "caldavclient.h"
#include "net/icalutils.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QUrlQuery>
#include <QDebug>
#include <QUuid>

namespace {
const QByteArray kBearerPrefix = "Bearer ";
}

QByteArray CaldavClient::authHeader(const QString &user, const QString &pass) const
{
    if (!m_bearer.isEmpty())
        return kBearerPrefix + m_bearer.toUtf8();
    const QByteArray creds = (user + QLatin1Char(':') + pass).toUtf8();
    return "Basic " + creds.toBase64();
}

CaldavClient::CaldavClient(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void CaldavClient::fetchEvents(const QString &url, const QString &user, const QString &pass)
{
    m_user = user;
    m_pass = pass;
    m_events.clear();
    m_todos.clear();
    m_visited.clear();
    m_inflight = 0;
    propfind(QUrl(url), true);
}

void CaldavClient::propfind(const QUrl &url, bool calendarData)
{
    if (m_visited.contains(url))
        return;
    m_visited.insert(url);
    m_inflight++;

    QNetworkRequest req(url);
    req.setRawHeader("Authorization", authHeader(m_user, m_pass));
    req.setRawHeader("Depth", "1");
    req.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QByteArray body;
    if (calendarData) {
        body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
               "<d:prop><d:resourcetype/><c:calendar-data/></d:prop>"
               "</d:propfind>";
    } else {
        body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<d:propfind xmlns:d=\"DAV:\"><d:prop><d:resourcetype/></d:prop></d:propfind>";
    }

    QNetworkReply *reply = m_nam->sendCustomRequest(req, "PROPFIND", body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        handlePropfindReply(reply);
    });
}

void CaldavClient::handlePropfindReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 401) {
            emit authenticationFailed();
            return;
        }
        emit failed(QStringLiteral("CalDAV request failed: %1").arg(reply->errorString()));
        return;
    }

    const QByteArray xml = reply->readAll();
    const QUrl base = reply->url();

    QXmlStreamReader reader(xml);
    QString currentHref;
    bool inResponse = false;

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            const QString name = reader.name().toString();
            if (name == QLatin1String("response")) {
                inResponse = true;
                currentHref.clear();
            } else if (name == QLatin1String("href") && inResponse) {
                currentHref = reader.readElementText();
            } else if (name == QLatin1String("calendar-data")) {
                const QString ics = reader.readElementText();
                const QVariantList events = ICalUtils::parseEvents(ics);
                for (const QVariant &ev : events) {
                    QVariantMap m = ev.toMap();
                    m[QStringLiteral("href")] = base.resolved(currentHref).toString();
                    m_events.append(m);
                }
                const QVariantList todos = ICalUtils::parseTodos(ics);
                for (const QVariant &td : todos) {
                    QVariantMap m = td.toMap();
                    m[QStringLiteral("href")] = base.resolved(currentHref).toString();
                    m_todos.append(m);
                }
            } else if (name == QLatin1String("calendar")) {
                // a calendar collection -> fetch its events next
                const QUrl sub = base.resolved(currentHref);
                propfind(sub, true);
            }
        } else if (reader.isEndElement()) {
            if (reader.name().toString() == QLatin1String("response"))
                inResponse = false;
        }
    }

    m_inflight--;
    if (m_inflight <= 0) {
        emit eventsFetched(m_events);
        emit todosFetched(m_todos);
    }
}

void CaldavClient::createEvent(const QString &url, const QString &user, const QString &pass,
                               const QVariantMap &event)
{
    m_user = user;
    m_pass = pass;

    QString uid = event.value(QStringLiteral("uid")).toString();
    if (uid.isEmpty()) {
        uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    QVariantMap copy = event;
    copy[QStringLiteral("uid")] = uid;
    const QByteArray ics = ICalUtils::buildEvent(copy).toUtf8();

    QUrl target(url);
    if (!target.path().endsWith(QLatin1Char('/')))
        target.setPath(target.path() + QLatin1Char('/'));
    target = target.resolved(QUrl(uid + QStringLiteral(".ics")));

    QNetworkRequest req(target);
    req.setRawHeader("Authorization", authHeader(user, pass));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/calendar; charset=utf-8"));

    QNetworkReply *reply = m_nam->put(req, ics);
    connect(reply, &QNetworkReply::finished, this, [this, reply, uid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
                emit authenticationFailed();
                return;
            }
            emit failed(QStringLiteral("CalDAV create failed: %1").arg(reply->errorString()));
            return;
        }
        emit eventCreated(uid);
    });
}

void CaldavClient::deleteEvent(const QString &url, const QString &user, const QString &pass,
                               const QString &uid)
{
    m_user = user;
    m_pass = pass;

    QUrl target(url);
    if (!target.path().endsWith(QLatin1Char('/')))
        target.setPath(target.path() + QLatin1Char('/'));
    target = target.resolved(QUrl(uid + QStringLiteral(".ics")));

    QNetworkRequest req(target);
    req.setRawHeader("Authorization", authHeader(user, pass));

    QNetworkReply *reply = m_nam->sendCustomRequest(req, "DELETE");
    connect(reply, &QNetworkReply::finished, this, [this, reply, uid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
                emit authenticationFailed();
                return;
            }
            emit failed(QStringLiteral("CalDAV delete failed: %1").arg(reply->errorString()));
            return;
        }
        emit eventDeleted(uid);
    });
}

void CaldavClient::createTodo(const QString &url, const QString &user, const QString &pass,
                              const QVariantMap &task)
{
    m_user = user;
    m_pass = pass;

    QString uid = task.value(QStringLiteral("uid")).toString();
    if (uid.isEmpty())
        uid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QVariantMap copy = task;
    copy[QStringLiteral("uid")] = uid;
    const QByteArray ics = ICalUtils::buildTodo(copy).toUtf8();

    QUrl target(url);
    if (!target.path().endsWith(QLatin1Char('/')))
        target.setPath(target.path() + QLatin1Char('/'));
    target = target.resolved(QUrl(uid + QStringLiteral(".ics")));

    QNetworkRequest req(target);
    req.setRawHeader("Authorization", authHeader(user, pass));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/calendar; charset=utf-8"));

    QNetworkReply *reply = m_nam->put(req, ics);
    connect(reply, &QNetworkReply::finished, this, [this, reply, uid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
                emit authenticationFailed();
                return;
            }
            emit failed(QStringLiteral("CalDAV todo create failed: %1").arg(reply->errorString()));
            return;
        }
        emit todoCreated(uid);
    });
}

void CaldavClient::deleteTodo(const QString &url, const QString &user, const QString &pass,
                              const QString &uid)
{
    m_user = user;
    m_pass = pass;

    QUrl target(url);
    if (!target.path().endsWith(QLatin1Char('/')))
        target.setPath(target.path() + QLatin1Char('/'));
    target = target.resolved(QUrl(uid + QStringLiteral(".ics")));

    QNetworkRequest req(target);
    req.setRawHeader("Authorization", authHeader(user, pass));

    QNetworkReply *reply = m_nam->sendCustomRequest(req, "DELETE");
    connect(reply, &QNetworkReply::finished, this, [this, reply, uid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401) {
                emit authenticationFailed();
                return;
            }
            emit failed(QStringLiteral("CalDAV todo delete failed: %1").arg(reply->errorString()));
            return;
        }
        emit todoDeleted(uid);
    });
}
