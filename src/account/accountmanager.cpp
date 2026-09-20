#include "accountmanager.h"
#include "account.h"
#include "provider.h"
#include "storage/database.h"
#include "storage/secretstore.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
{
}

QQmlListProperty<Account> AccountManager::accounts()
{
    return QQmlListProperty<Account>(this, nullptr, &AccountManager::listCount, &AccountManager::listAt);
}

qsizetype AccountManager::listCount(QQmlListProperty<Account> *list)
{
    return static_cast<AccountManager *>(list->object)->m_accounts.size();
}

Account *AccountManager::listAt(QQmlListProperty<Account> *list, qsizetype index)
{
    return static_cast<AccountManager *>(list->object)->m_accounts.at(index);
}

void AccountManager::append(Account *a)
{
    m_accounts.append(a);
    emit accountsChanged();
}

Account *AccountManager::accountById(int id) const
{
    for (Account *a : m_accounts) {
        if (a->id() == id)
            return a;
    }
    return nullptr;
}

void AccountManager::load()
{
    QSqlQuery q(Database::instance()->connection());
    if (!q.exec(QStringLiteral("SELECT * FROM accounts ORDER BY id ASC"))) {
        qWarning() << "Failed to load accounts:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        auto *a = new Account(this);
        a->setId(q.value(QStringLiteral("id")).toInt());
        a->setName(q.value(QStringLiteral("name")).toString());
        a->setEmail(q.value(QStringLiteral("email")).toString());
        a->setProvider(q.value(QStringLiteral("provider")).toString());
        a->setImapHost(q.value(QStringLiteral("imap_host")).toString());
        a->setImapPort(q.value(QStringLiteral("imap_port")).toInt());
        a->setSmtpHost(q.value(QStringLiteral("smtp_host")).toString());
        a->setSmtpPort(q.value(QStringLiteral("smtp_port")).toInt());
        a->setCaldavUrl(q.value(QStringLiteral("caldav_url")).toString());
        a->setCarddavUrl(q.value(QStringLiteral("carddav_url")).toString());
        a->setAllowUntrusted(q.value(QStringLiteral("allow_untrusted")).toBool());
        a->setAuthMethod(q.value(QStringLiteral("auth_method")).toString());
        if (a->authMethod().isEmpty())
            a->setAuthMethod(QStringLiteral("password"));
        a->setOauthClientId(q.value(QStringLiteral("oauth_client_id")).toString());
        a->setEwsUrl(q.value(QStringLiteral("ews_url")).toString());
        m_accounts.append(a);

        // Migration: move any legacy plaintext password into the keyring, then erase it.
        const QString legacyPw = q.value(QStringLiteral("password")).toString();
        if (!legacyPw.isEmpty() && SecretStore::instance()->store(a->email(), legacyPw)) {
            QSqlQuery up(Database::instance()->connection());
            up.prepare(QStringLiteral("UPDATE accounts SET password = '' WHERE id = ?"));
            up.addBindValue(a->id());
            up.exec();
        }
    }
    emit accountsChanged();
}

QObject *AccountManager::addAccount(const QVariantMap &data)
{
    auto *a = new Account(this);

    a->setName(data.value(QStringLiteral("name")).toString());
    a->setEmail(data.value(QStringLiteral("email")).toString());
    a->setProvider(data.value(QStringLiteral("provider")).toString());

    const QVariantMap preset = Provider::resolveUrls(
        Provider::preset(Provider::stringToType(a->provider())), a->email());

    a->setImapHost(data.value(QStringLiteral("imapHost"), preset.value(QStringLiteral("imapHost"))).toString());
    a->setImapPort(data.value(QStringLiteral("imapPort"), preset.value(QStringLiteral("imapPort"))).toInt());
    a->setSmtpHost(data.value(QStringLiteral("smtpHost"), preset.value(QStringLiteral("smtpHost"))).toString());
    a->setSmtpPort(data.value(QStringLiteral("smtpPort"), preset.value(QStringLiteral("smtpPort"))).toInt());
    a->setCaldavUrl(data.value(QStringLiteral("caldavUrl"), preset.value(QStringLiteral("caldavUrl"))).toString());
    a->setCarddavUrl(data.value(QStringLiteral("carddavUrl"), preset.value(QStringLiteral("carddavUrl"))).toString());
    a->setPassword(data.value(QStringLiteral("password")).toString());
    a->setAllowUntrusted(data.value(QStringLiteral("allowUntrusted")).toBool());
    a->setAuthMethod(data.value(QStringLiteral("authMethod"), QStringLiteral("password")).toString());
    a->setOauthClientId(data.value(QStringLiteral("oauthClientId")).toString());
    a->setEwsUrl(data.value(QStringLiteral("ewsUrl"), preset.value(QStringLiteral("ewsUrl"))).toString());

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO accounts (name, email, provider, imap_host, imap_port, smtp_host, smtp_port, caldav_url, carddav_url, allow_untrusted, auth_method, oauth_client_id, ews_url) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(a->name());
    q.addBindValue(a->email());
    q.addBindValue(a->provider());
    q.addBindValue(a->imapHost());
    q.addBindValue(a->imapPort());
    q.addBindValue(a->smtpHost());
    q.addBindValue(a->smtpPort());
    q.addBindValue(a->caldavUrl());
    q.addBindValue(a->carddavUrl());
    q.addBindValue(a->allowUntrusted() ? 1 : 0);
    q.addBindValue(a->authMethod());
    q.addBindValue(a->oauthClientId());
    q.addBindValue(a->ewsUrl());

    if (!q.exec()) {
        qWarning() << "Failed to insert account:" << q.lastError().text();
    }
    a->setId(q.lastInsertId().toInt());

    append(a);
    return a;
}

void AccountManager::removeAccount(int id)
{
    Account *a = accountById(id);
    if (a)
        SecretStore::instance()->clear(a->email());

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM accounts WHERE id = ?"));
    q.addBindValue(id);
    if (!q.exec())
        qWarning() << "Failed to remove account:" << q.lastError().text();

    for (int i = 0; i < m_accounts.size(); ++i) {
        if (m_accounts.at(i)->id() == id) {
            m_accounts.at(i)->deleteLater();
            m_accounts.removeAt(i);
            break;
        }
    }
    emit accountsChanged();
}

void AccountManager::updateAccount(int id, const QVariantMap &data)
{
    Account *a = accountById(id);
    if (!a)
        return;

    if (data.contains(QStringLiteral("name"))) a->setName(data.value(QStringLiteral("name")).toString());
    if (data.contains(QStringLiteral("email"))) a->setEmail(data.value(QStringLiteral("email")).toString());
    if (data.contains(QStringLiteral("imapHost"))) a->setImapHost(data.value(QStringLiteral("imapHost")).toString());
    if (data.contains(QStringLiteral("imapPort"))) a->setImapPort(data.value(QStringLiteral("imapPort")).toInt());
    if (data.contains(QStringLiteral("smtpHost"))) a->setSmtpHost(data.value(QStringLiteral("smtpHost")).toString());
    if (data.contains(QStringLiteral("smtpPort"))) a->setSmtpPort(data.value(QStringLiteral("smtpPort")).toInt());
    if (data.contains(QStringLiteral("caldavUrl"))) a->setCaldavUrl(data.value(QStringLiteral("caldavUrl")).toString());
    if (data.contains(QStringLiteral("carddavUrl"))) a->setCarddavUrl(data.value(QStringLiteral("carddavUrl")).toString());
    if (data.contains(QStringLiteral("password"))) a->setPassword(data.value(QStringLiteral("password")).toString());

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "UPDATE accounts SET name=?, email=?, imap_host=?, imap_port=?, smtp_host=?, smtp_port=?, caldav_url=?, carddav_url=? WHERE id=?"));
    q.addBindValue(a->name());
    q.addBindValue(a->email());
    q.addBindValue(a->imapHost());
    q.addBindValue(a->imapPort());
    q.addBindValue(a->smtpHost());
    q.addBindValue(a->smtpPort());
    q.addBindValue(a->caldavUrl());
    q.addBindValue(a->carddavUrl());
    q.addBindValue(id);
    q.exec();
}
