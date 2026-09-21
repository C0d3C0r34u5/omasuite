#include "oauth2manager.h"

#include "account/account.h"
#include "account/provider.h"
#include "storage/secretstore.h"

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

namespace {
QString tokenKey(const QString &email)
{
    return QStringLiteral("oauth2:") + email;
}

QJsonObject readTokens(const QString &email)
{
    const QString raw = SecretStore::instance()->lookup(tokenKey(email));
    if (raw.isEmpty())
        return QJsonObject();
    return QJsonDocument::fromJson(raw.toUtf8()).object();
}
}

OAuth2Manager::OAuth2Manager(QObject *parent)
    : QObject(parent)
{
}

bool OAuth2Manager::hasTokens(const QString &email) const
{
    return !readTokens(email).isEmpty();
}

QString OAuth2Manager::accessToken(const QString &email) const
{
    return readTokens(email).value(QStringLiteral("access_token")).toString();
}

QString OAuth2Manager::refreshToken(const QString &email) const
{
    return readTokens(email).value(QStringLiteral("refresh_token")).toString();
}

bool OAuth2Manager::accessTokenValid(const QString &email) const
{
    const QJsonObject obj = readTokens(email);
    const QString access = obj.value(QStringLiteral("access_token")).toString();
    if (access.isEmpty())
        return false;

    const qint64 expiry = static_cast<qint64>(obj.value(QStringLiteral("expires_at")).toDouble());
    if (expiry <= 0)
        return true; // no expiry recorded (legacy token) -> assume still valid

    // Treat as invalid 60 seconds before the actual expiry to leave headroom.
    return expiry > QDateTime::currentMSecsSinceEpoch() + 60000;
}

void OAuth2Manager::storeTokens(const QString &email, const QString &access, const QString &refresh,
                                qint64 expiresAtMs)
{
    QJsonObject obj = readTokens(email);
    if (!access.isEmpty())
        obj[QStringLiteral("access_token")] = access;
    if (!refresh.isEmpty())
        obj[QStringLiteral("refresh_token")] = refresh;
    if (expiresAtMs > 0)
        obj[QStringLiteral("expires_at")] = static_cast<double>(expiresAtMs);

    const QString json = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    SecretStore::instance()->store(tokenKey(email), json);
}

void OAuth2Manager::authorize(Account *account)
{
    if (!account)
        return;
    authorize(account->provider(), account->email(), account->oauthClientId(),
              account->oauthClientSecret());
}

void OAuth2Manager::authorize(const QString &provider, const QString &email,
                              const QString &clientId, const QString &clientSecret)
{
    startFlow(provider, email, clientId, clientSecret, false);
}

void OAuth2Manager::refresh(const QString &provider, const QString &email,
                            const QString &clientId, const QString &clientSecret)
{
    startFlow(provider, email, clientId, clientSecret, true);
}

void OAuth2Manager::startFlow(const QString &provider, const QString &email,
                              const QString &clientId, const QString &clientSecret,
                              bool refreshOnly)
{
    const QVariantMap preset = Provider::preset(Provider::stringToType(provider));
    const QUrl authUrl(preset.value(QStringLiteral("authUrl")).toString());
    const QUrl tokenUrl(preset.value(QStringLiteral("tokenUrl")).toString());
    const QString scopes = preset.value(QStringLiteral("scopes")).toString();

    if (authUrl.isEmpty() || tokenUrl.isEmpty()) {
        emit failed(email, QStringLiteral("This provider does not support OAuth2"));
        return;
    }

    m_activeEmail = email;
    m_refreshing = refreshOnly;

    if (m_flow) {
        m_flow->disconnect(this);
        m_flow->deleteLater();
        m_flow = nullptr;
    }
    if (m_handler) {
        m_handler->deleteLater();
        m_handler = nullptr;
    }

    m_flow = new QOAuth2AuthorizationCodeFlow(this);
    m_flow->setClientIdentifier(clientId);
    m_flow->setAuthorizationUrl(authUrl);
    m_flow->setTokenUrl(tokenUrl);
    m_flow->setScope(scopes);
    if (clientSecret.isEmpty()) {
        // Public client -> PKCE.
        m_flow->setPkceMethod(QOAuth2AuthorizationCodeFlow::PkceMethod::S256);
    } else {
        // Confidential client -> client secret in the token exchange.
        m_flow->setClientIdentifierSharedKey(clientSecret);
    }

    m_handler = new QOAuthHttpServerReplyHandler(0, this);
    m_handler->setCallbackHost(QStringLiteral("localhost"));
    m_flow->setReplyHandler(m_handler);

    connect(m_flow, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, [](const QUrl &url) { QDesktopServices::openUrl(url); });

    connect(m_flow, &QOAuth2AuthorizationCodeFlow::granted, this, [this]() {
        const QString access = m_flow->token();
        const QString refresh = m_flow->refreshToken();
        const qint64 expiry = m_flow->expirationAt().toMSecsSinceEpoch();
        storeTokens(m_activeEmail, access, refresh, expiry);
        if (m_refreshing)
            emit tokenRefreshed(m_activeEmail);
        else
            emit authorized(m_activeEmail);
    });

    connect(m_flow, &QOAuth2AuthorizationCodeFlow::serverReportedErrorOccurred, this,
            [this](const QString &error, const QString &desc, const QUrl &uri) {
        Q_UNUSED(uri);
        emit failed(m_activeEmail, desc.isEmpty() ? error : desc);
    });

    connect(m_flow, &QOAuth2AuthorizationCodeFlow::statusChanged, this,
            [this](QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::NotAuthenticated)
            return;
        if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived)
            return;
        if (status == QAbstractOAuth::Status::RefreshingToken)
            return;
        // Only treat a hard failure as fatal here; the granted() signal drives success.
        if (status == QAbstractOAuth::Status::Granted)
            return;
        qWarning() << "OAuth status:" << static_cast<int>(status);
    });

    if (refreshOnly) {
        const QString rt = refreshToken(m_activeEmail);
        if (rt.isEmpty()) {
            emit failed(m_activeEmail, QStringLiteral("No refresh token available"));
            return;
        }
        m_flow->setRefreshToken(rt);
        m_flow->refreshAccessToken();
    } else {
        m_flow->grant();
    }
}
