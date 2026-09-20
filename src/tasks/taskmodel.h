#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QList>

struct TaskItem {
    int id = 0;
    int accountId = 0;
    QString uid;
    QString title;
    QString description;
    qint64 due = 0;
    bool completed = false;
    int priority = 0;
};

class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AccountIdRole,
        UidRole,
        TitleRole,
        DescriptionRole,
        DueRole,
        DueDateRole,
        CompletedRole,
        PriorityRole
    };
    Q_ENUM(Roles)

    explicit TaskModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_items.size(); }
    int pendingCount() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE int addTask(int accountId, const QString &title, const QString &description = QString(), qint64 due = 0);
    Q_INVOKABLE int addSynced(int accountId, const QString &uid, const QString &title,
                              const QString &description, qint64 due, bool completed);
    Q_INVOKABLE bool hasUid(const QString &uid) const;
    Q_INVOKABLE void removeTask(int id);
    Q_INVOKABLE void setCompleted(int id, bool completed);

signals:
    void countChanged();

private:
    QList<TaskItem> m_items;
};

#endif // TASKMODEL_H
