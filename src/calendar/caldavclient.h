#ifndef CALDAVCLIENT_H
#define CALDAVCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QVariantList>
#include <QList>
#include <QSet>
#include <QUrl>

class CaldavClient : public QObject
{
    Q_OBJECT
public:
    explicit CaldavClient(QObject *parent = nullptr);

    void fetchEvents(const QString &url, const QString &user, const QString &pass);
    void createEvent(const QString &url, const QString &user, const QString &pass, const QVariantMap &event);
    void deleteEvent(const QString &url, const QString &user, const QString &pass, const QString &uid);
    void createTodo(const QString &url, const QString &user, const QString &pass, const QVariantMap &task);
    void deleteTodo(const QString &url, const QString &user, const QString &pass, const QString &uid);

    void setBearerToken(const QString &token) { m_bearer = token; }

signals:
    void eventsFetched(const QVariantList &events);
    void todosFetched(const QVariantList &todos);
    void eventCreated(const QString &uid);
    void eventDeleted(const QString &uid);
    void todoCreated(const QString &uid);
    void todoDeleted(const QString &uid);
    void failed(const QString &reason);

private:
    void propfind(const QUrl &url, bool calendarData);
    void putEvent(const QUrl &url, const QByteArray &ics);
    void handlePropfindReply(QNetworkReply *reply);
    QByteArray authHeader(const QString &user, const QString &pass) const;

    QNetworkAccessManager *m_nam = nullptr;
    QString m_user;
    QString m_pass;
    QString m_bearer;
    QSet<QUrl> m_visited;
    QVariantList m_events;
    QVariantList m_todos;
    int m_inflight = 0;
};

#endif // CALDAVCLIENT_H
