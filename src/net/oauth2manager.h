#ifndef OAUTH2MANAGER_H
#define OAUTH2MANAGER_H

#include <QObject>

class Account;
class QOAuth2AuthorizationCodeFlow;
class QOAuthHttpServerReplyHandler;

class OAuth2Manager : public QObject
{
    Q_OBJECT
public:
    explicit OAuth2Manager(QObject *parent = nullptr);

    Q_INVOKABLE bool hasTokens(const QString &email) const;
    Q_INVOKABLE QString accessToken(const QString &email) const;
    Q_INVOKABLE QString refreshToken(const QString &email) const;
    Q_INVOKABLE bool accessTokenValid(const QString &email) const;
    void storeTokens(const QString &email, const QString &access, const QString &refresh,
                     qint64 expiresAtMs = 0);

    Q_INVOKABLE void authorize(const QString &provider, const QString &email, const QString &clientId,
                   const QString &clientSecret = QString());
    void authorize(Account *account);
    Q_INVOKABLE void refresh(const QString &provider, const QString &email, const QString &clientId,
                 const QString &clientSecret = QString());

signals:
    void authorized(const QString &email);
    void tokenRefreshed(const QString &email);
    void failed(const QString &email, const QString &reason);

private:
    void startFlow(const QString &provider, const QString &email, const QString &clientId,
                   const QString &clientSecret, bool refreshOnly);

    QOAuth2AuthorizationCodeFlow *m_flow = nullptr;
    QOAuthHttpServerReplyHandler *m_handler = nullptr;
    QString m_activeEmail;
    bool m_refreshing = false;
};

#endif // OAUTH2MANAGER_H
