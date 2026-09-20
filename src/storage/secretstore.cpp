#include <libsecret/secret.h>

#include "secretstore.h"

#include <QDebug>

namespace {
const SecretSchema *omasuiteSchema()
{
    static const SecretSchema schema = {
        "org.omarchy.OmaSuite",
        SECRET_SCHEMA_NONE,
        {
            { "account", SECRET_SCHEMA_ATTRIBUTE_STRING },
            { nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING }
        }
    };
    return &schema;
}
}

SecretStore *SecretStore::instance()
{
    static SecretStore *store = nullptr;
    if (!store)
        store = new SecretStore();
    return store;
}

SecretStore::SecretStore()
{
    // Probe for the Secret Service; if unavailable we fall back to in-memory only.
    GError *error = nullptr;
    gchar *probe = secret_password_lookup_sync(omasuiteSchema(), nullptr, &error,
                                               "account", "__omasuite_probe__", nullptr);
    if (error) {
        qWarning() << "SecretStore: Secret Service unavailable:" << error->message;
        g_error_free(error);
        m_available = false;
        return;
    }
    if (probe) {
        secret_password_free(probe);
    }
    m_available = true;
}

bool SecretStore::store(const QString &account, const QString &secret)
{
    if (!m_available)
        return false;

    GError *error = nullptr;
    const QByteArray key = account.toUtf8();
    const QByteArray val = secret.toUtf8();
    const QByteArray label = ("OmaSuite account " + account).toUtf8();

    const gboolean ok = secret_password_store_sync(
        omasuiteSchema(), SECRET_COLLECTION_DEFAULT, label.constData(), val.constData(),
        nullptr, &error, "account", key.constData(), nullptr);

    if (error) {
        qWarning() << "SecretStore::store failed:" << error->message;
        g_error_free(error);
        return false;
    }
    return ok == TRUE;
}

QString SecretStore::lookup(const QString &account) const
{
    if (!m_available)
        return QString();

    GError *error = nullptr;
    const QByteArray key = account.toUtf8();

    gchar *pw = secret_password_lookup_sync(omasuiteSchema(), nullptr, &error,
                                            "account", key.constData(), nullptr);
    if (error) {
        qWarning() << "SecretStore::lookup failed:" << error->message;
        g_error_free(error);
        return QString();
    }
    if (!pw)
        return QString();

    const QString result = QString::fromUtf8(pw);
    secret_password_free(pw);
    return result;
}

void SecretStore::clear(const QString &account)
{
    if (!m_available)
        return;

    GError *error = nullptr;
    const QByteArray key = account.toUtf8();

    secret_password_clear_sync(omasuiteSchema(), nullptr, &error,
                               "account", key.constData(), nullptr);
    if (error) {
        qWarning() << "SecretStore::clear failed:" << error->message;
        g_error_free(error);
    }
}
