#include "database.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database::Database(QObject *parent)
    : QObject(parent)
{
}

Database *Database::instance()
{
    static Database *db = nullptr;
    if (!db) {
        db = new Database();
    }
    return db;
}

bool Database::initialize()
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + QStringLiteral("/omasuite.db");

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    return createSchema();
}

QSqlDatabase Database::connection() const
{
    return m_db;
}

bool Database::createSchema()
{
    QSqlQuery q(m_db);

    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS accounts ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT NOT NULL,"
            "email TEXT NOT NULL,"
            "provider TEXT NOT NULL DEFAULT 'manual',"
            "imap_host TEXT,"
            "imap_port INTEGER DEFAULT 993,"
            "smtp_host TEXT,"
            "smtp_port INTEGER DEFAULT 465,"
            "caldav_url TEXT,"
            "carddav_url TEXT,"
            "password TEXT,"
            "allow_untrusted INTEGER DEFAULT 0,"
            "auth_method TEXT DEFAULT 'password',"
            "oauth_client_id TEXT DEFAULT '',"
            "ews_url TEXT DEFAULT '',"
            "created_at TEXT DEFAULT (datetime('now'))"
            ")"),

        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS folders ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "account_id INTEGER NOT NULL,"
            "name TEXT NOT NULL,"
            "type TEXT NOT NULL DEFAULT 'user',"
            "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
            ")"),

        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS emails ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "account_id INTEGER NOT NULL,"
            "folder_id INTEGER,"
            "uid TEXT,"
            "subject TEXT DEFAULT '',"
            "sender TEXT DEFAULT '',"
            "recipient TEXT DEFAULT '',"
            "preview TEXT DEFAULT '',"
            "body TEXT DEFAULT '',"
            "timestamp INTEGER DEFAULT 0,"
            "is_read INTEGER DEFAULT 0,"
            "is_starred INTEGER DEFAULT 0,"
            "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
            ")"),

        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS events ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "account_id INTEGER NOT NULL,"
            "uid TEXT,"
            "title TEXT DEFAULT '',"
            "description TEXT DEFAULT '',"
            "location TEXT DEFAULT '',"
            "start INTEGER DEFAULT 0,"
            "end INTEGER DEFAULT 0,"
            "all_day INTEGER DEFAULT 0,"
            "reminder INTEGER DEFAULT 0,"
            "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
            ")"),

        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS contacts ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "account_id INTEGER NOT NULL,"
            "uid TEXT,"
            "first_name TEXT DEFAULT '',"
            "last_name TEXT DEFAULT '',"
            "email TEXT DEFAULT '',"
            "phone TEXT DEFAULT '',"
            "organization TEXT DEFAULT '',"
            "notes TEXT DEFAULT '',"
            "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
            ")"),

        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS tasks ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "account_id INTEGER NOT NULL,"
            "uid TEXT,"
            "title TEXT DEFAULT '',"
            "description TEXT DEFAULT '',"
            "due INTEGER DEFAULT 0,"
            "completed INTEGER DEFAULT 0,"
            "priority INTEGER DEFAULT 0,"
            "FOREIGN KEY(account_id) REFERENCES accounts(id) ON DELETE CASCADE"
            ")")
    };

    for (const QString &s : statements) {
        if (!q.exec(s)) {
            qWarning() << "Schema error:" << q.lastError().text();
            return false;
        }
    }

    // Migration: add password column if missing (pre-network-layer databases).
    q.exec(QStringLiteral("SELECT password FROM accounts LIMIT 1"));
    if (q.lastError().isValid()) {
        QSqlQuery alt(m_db);
        if (!alt.exec(QStringLiteral("ALTER TABLE accounts ADD COLUMN password TEXT"))) {
            qWarning() << "Migration failed:" << alt.lastError().text();
        }
    }

    // Migration: add allow_untrusted column if missing.
    q.exec(QStringLiteral("SELECT allow_untrusted FROM accounts LIMIT 1"));
    if (q.lastError().isValid()) {
        QSqlQuery alt(m_db);
        alt.exec(QStringLiteral("ALTER TABLE accounts ADD COLUMN allow_untrusted INTEGER DEFAULT 0"));
    }

    // Migration: add auth_method column if missing.
    q.exec(QStringLiteral("SELECT auth_method FROM accounts LIMIT 1"));
    if (q.lastError().isValid()) {
        QSqlQuery alt(m_db);
        alt.exec(QStringLiteral("ALTER TABLE accounts ADD COLUMN auth_method TEXT DEFAULT 'password'"));
    }

    // Migration: add oauth_client_id column if missing.
    q.exec(QStringLiteral("SELECT oauth_client_id FROM accounts LIMIT 1"));
    if (q.lastError().isValid()) {
        QSqlQuery alt(m_db);
        alt.exec(QStringLiteral("ALTER TABLE accounts ADD COLUMN oauth_client_id TEXT DEFAULT ''"));
    }

    // Migration: add ews_url column if missing.
    q.exec(QStringLiteral("SELECT ews_url FROM accounts LIMIT 1"));
    if (q.lastError().isValid()) {
        QSqlQuery alt(m_db);
        alt.exec(QStringLiteral("ALTER TABLE accounts ADD COLUMN ews_url TEXT DEFAULT ''"));
    }

    return true;
}
