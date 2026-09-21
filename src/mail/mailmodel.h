#ifndef MAILMODEL_H
#define MAILMODEL_H

#include <QAbstractListModel>
#include <QList>

struct MailItem {
    int id = 0;
    int accountId = 0;
    QString uid;
    QString subject;
    QString sender;
    QString recipient;
    QString preview;
    QString body;
    qint64 timestamp = 0;
    bool read = false;
    bool starred = false;
    bool trashed = false;
};

class MailModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int trashCount READ trashCount NOTIFY trashCountChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AccountIdRole,
        UidRole,
        SubjectRole,
        SenderRole,
        RecipientRole,
        PreviewRole,
        BodyRole,
        TimestampRole,
        DateRole,
        ReadRole,
        StarredRole,
        TrashedRole
    };
    Q_ENUM(Roles)

    explicit MailModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_items.size(); }
    int trashCount() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE int add(int accountId, const QString &subject, const QString &sender,
                        const QString &recipient, const QString &body, qint64 timestamp = 0);
    Q_INVOKABLE int addSynced(int accountId, const QString &uid, const QString &subject,
                              const QString &sender, const QString &recipient, const QString &body,
                              qint64 timestamp, bool seen, bool starred);
    Q_INVOKABLE bool hasUid(const QString &uid) const;
    Q_INVOKABLE void setBodyByUid(const QString &uid, const QString &body);
    Q_INVOKABLE void remove(int id);
    Q_INVOKABLE void setRead(int id, bool read);
    Q_INVOKABLE void toggleStarred(int id);
    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE QVariantMap getById(int id) const;
    Q_INVOKABLE void trash(int id);
    Q_INVOKABLE void restore(int id);
    Q_INVOKABLE void purge(int id);
    Q_INVOKABLE void emptyTrash();

signals:
    void countChanged();
    void trashCountChanged();

private:
    void setTrashed(int id, bool trashed);

    QList<MailItem> m_items;
};

#endif // MAILMODEL_H
