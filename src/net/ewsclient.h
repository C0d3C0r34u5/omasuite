#ifndef EWSCLIENT_H
#define EWSCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QVariantList>
#include <QUrl>
#include <QXmlStreamReader>
#include <functional>

class EwsClient : public QObject
{
    Q_OBJECT
public:
    explicit EwsClient(QObject *parent = nullptr);

    void setEndpoint(const QString &url) { m_endpoint = url; }
    void setBearerToken(const QString &token) { m_bearer = token; }
    void setBasicAuth(const QString &user, const QString &pass) { m_user = user; m_pass = pass; m_bearer.clear(); }
    void setAllowUntrusted(bool allow) { m_allowUntrusted = allow; }

    void fetchMail(const QString &folder, int limit = 100);
    void sendMail(const QString &from, const QString &to, const QString &subject, const QString &body);
    void fetchCalendar(qint64 start, qint64 end);
    void fetchContacts();
    void fetchTasks();

signals:
    void messagesFetched(const QVariantList &messages);
    void eventsFetched(const QVariantList &events);
    void contactsFetched(const QVariantList &contacts);
    void tasksFetched(const QVariantList &tasks);
    void sent();
    void failed(const QString &reason);

private:
    void post(const QByteArray &soap, std::function<void(QXmlStreamReader &)> parser);

    QNetworkAccessManager *m_nam = nullptr;
    QString m_endpoint;
    QString m_bearer;
    QString m_user;
    QString m_pass;
    bool m_allowUntrusted = false;
};

#endif // EWSCLIENT_H
