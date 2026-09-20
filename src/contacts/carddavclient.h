#ifndef CARDDAVCLIENT_H
#define CARDDAVCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QVariantList>
#include <QUrl>

class CarddavClient : public QObject
{
    Q_OBJECT
public:
    explicit CarddavClient(QObject *parent = nullptr);

    void fetchContacts(const QString &url, const QString &user, const QString &pass);
    void createContact(const QString &url, const QString &user, const QString &pass, const QVariantMap &contact);
    void deleteContact(const QString &url, const QString &user, const QString &pass, const QString &uid);

    void setBearerToken(const QString &token) { m_bearer = token; }

signals:
    void contactsFetched(const QVariantList &contacts);
    void contactCreated(const QString &uid);
    void contactDeleted(const QString &uid);
    void failed(const QString &reason);

private:
    QByteArray authHeader(const QString &user, const QString &pass) const;

    QNetworkAccessManager *m_nam = nullptr;
    QString m_user;
    QString m_pass;
    QString m_bearer;
};

#endif // CARDDAVCLIENT_H
