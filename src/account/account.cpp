#include "account.h"
#include "provider.h"
#include "storage/secretstore.h"

#include <QStringList>

Account::Account(QObject *parent)
    : QObject(parent)
{
}

void Account::setName(const QString &v)
{
    if (m_name == v) return;
    m_name = v;
    emit nameChanged();
}

void Account::setEmail(const QString &v)
{
    if (m_email == v) return;
    m_email = v;
    emit emailChanged();
}

void Account::setProvider(const QString &v)
{
    if (m_provider == v) return;
    m_provider = v;
    const QVariantMap p = Provider::preset(Provider::stringToType(v));
    m_providerName = p.value(QStringLiteral("brand")).toString();
    m_color = p.value(QStringLiteral("color")).toString();
    emit providerChanged();
}

void Account::setImapHost(const QString &v)
{
    if (m_imapHost == v) return;
    m_imapHost = v;
    emit imapHostChanged();
}

void Account::setImapPort(int v)
{
    if (m_imapPort == v) return;
    m_imapPort = v;
    emit imapPortChanged();
}

void Account::setSmtpHost(const QString &v)
{
    if (m_smtpHost == v) return;
    m_smtpHost = v;
    emit smtpHostChanged();
}

void Account::setSmtpPort(int v)
{
    if (m_smtpPort == v) return;
    m_smtpPort = v;
    emit smtpPortChanged();
}

void Account::setCaldavUrl(const QString &v)
{
    if (m_caldavUrl == v) return;
    m_caldavUrl = v;
    emit caldavUrlChanged();
}

void Account::setCarddavUrl(const QString &v)
{
    if (m_carddavUrl == v) return;
    m_carddavUrl = v;
    emit carddavUrlChanged();
}

QString Account::password() const
{
    return SecretStore::instance()->lookup(m_email);
}

void Account::setPassword(const QString &v)
{
    const QString current = SecretStore::instance()->lookup(m_email);
    if (current == v)
        return;
    if (!v.isEmpty())
        SecretStore::instance()->store(m_email, v);
    else
        SecretStore::instance()->clear(m_email);
    emit passwordChanged();
}

void Account::setAllowUntrusted(bool v)
{
    if (m_allowUntrusted == v) return;
    m_allowUntrusted = v;
    emit allowUntrustedChanged();
}

void Account::setAuthMethod(const QString &v)
{
    if (m_authMethod == v) return;
    m_authMethod = v;
    emit authMethodChanged();
}

void Account::setOauthClientId(const QString &v)
{
    if (m_oauthClientId == v) return;
    m_oauthClientId = v;
    emit oauthClientIdChanged();
}

void Account::setOauthClientSecret(const QString &v)
{
    if (m_oauthClientSecret == v) return;
    m_oauthClientSecret = v;
    emit oauthClientSecretChanged();
}

void Account::setEwsUrl(const QString &v)
{
    if (m_ewsUrl == v) return;
    m_ewsUrl = v;
    emit ewsUrlChanged();
}

bool Account::oauthReady() const
{
    return !SecretStore::instance()->lookup(QStringLiteral("oauth2:") + m_email).isEmpty();
}

QString Account::initials() const
{
    QStringList parts = m_name.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QString result;
    for (int i = 0; i < parts.size() && i < 2; ++i)
        result += parts.at(i).left(1).toUpper();
    if (result.isEmpty() && !m_email.isEmpty())
        result = m_email.left(1).toUpper();
    return result;
}

QVariantMap Account::toMap() const
{
    QVariantMap m;
    m[QStringLiteral("id")] = m_id;
    m[QStringLiteral("name")] = m_name;
    m[QStringLiteral("email")] = m_email;
    m[QStringLiteral("provider")] = m_provider;
    m[QStringLiteral("imapHost")] = m_imapHost;
    m[QStringLiteral("imapPort")] = m_imapPort;
    m[QStringLiteral("smtpHost")] = m_smtpHost;
    m[QStringLiteral("smtpPort")] = m_smtpPort;
    m[QStringLiteral("caldavUrl")] = m_caldavUrl;
    m[QStringLiteral("carddavUrl")] = m_carddavUrl;
    return m;
}