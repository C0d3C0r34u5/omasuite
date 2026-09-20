#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>

class Database : public QObject
{
    Q_OBJECT
public:
    static Database *instance();

    bool initialize();
    QSqlDatabase connection() const;
    QString dbPath() const { return m_dbPath; }

private:
    explicit Database(QObject *parent = nullptr);
    bool createSchema();

    QString m_dbPath;
    QSqlDatabase m_db;
};

#endif // DATABASE_H
