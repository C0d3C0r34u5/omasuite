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

    bool hasTokens(const QString &email) const;
    QString accessToken(const QString &email) const;
    QString refreshToken(const QString &email) const;
    void storeTokens(const QString &email, const QString &access, const QString &refresh);

    void authorize(const QString &provider, const QString &email, const QString &clientId);
    void authorize(Account *account);
    void refresh(const QString &provider, const QString &email, const QString &clientId);

signals:
    void authorized(const QString &email);
    void tokenRefreshed(const QString &email);
    void failed(const QString &email, const QString &reason);

private:
    void startFlow(const QString &provider, const QString &email, const QString &clientId, bool refreshOnly);

    QOAuth2AuthorizationCodeFlow *m_flow = nullptr;
    QOAuthHttpServerReplyHandler *m_handler = nullptr;
    QString m_activeEmail;
    bool m_refreshing = false;
};

#endif // OAUTH2MANAGER_H
