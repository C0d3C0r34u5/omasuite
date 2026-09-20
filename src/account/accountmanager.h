#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include <QQmlListProperty>
#include <QList>

class Account;

class AccountManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Account> accounts READ accounts NOTIFY accountsChanged)
    Q_PROPERTY(int count READ count NOTIFY accountsChanged)
    Q_PROPERTY(bool hasAccounts READ hasAccounts NOTIFY accountsChanged)

public:
    explicit AccountManager(QObject *parent = nullptr);

    QQmlListProperty<Account> accounts();
    int count() const { return m_accounts.size(); }
    bool hasAccounts() const { return !m_accounts.isEmpty(); }

    Account *accountById(int id) const;
    Account *firstAccount() const { return m_accounts.isEmpty() ? nullptr : m_accounts.first(); }

    void load();

    Q_INVOKABLE QObject *addAccount(const QVariantMap &data);
    Q_INVOKABLE void removeAccount(int id);
    Q_INVOKABLE void updateAccount(int id, const QVariantMap &data);

signals:
    void accountsChanged();

private:
    void append(Account *a);

    static qsizetype listCount(QQmlListProperty<Account> *list);
    static Account *listAt(QQmlListProperty<Account> *list, qsizetype index);

    QList<Account *> m_accounts;
};

#endif // ACCOUNTMANAGER_H
