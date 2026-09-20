#ifndef SECRETSTORE_H
#define SECRETSTORE_H

#include <QString>

class SecretStore
{
public:
    static SecretStore *instance();

    bool available() const { return m_available; }

    bool store(const QString &account, const QString &secret);
    QString lookup(const QString &account) const;
    void clear(const QString &account);

private:
    SecretStore();
    bool m_available = false;
};

#endif // SECRETSTORE_H
