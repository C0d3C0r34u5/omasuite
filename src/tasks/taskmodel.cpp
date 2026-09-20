#include "taskmodel.h"
#include "storage/database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <algorithm>

TaskModel::TaskModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const TaskItem &it = m_items.at(index.row());
    switch (role) {
    case IdRole:          return it.id;
    case AccountIdRole:   return it.accountId;
    case UidRole:         return it.uid;
    case TitleRole:       return it.title;
    case DescriptionRole: return it.description;
    case DueRole:         return it.due;
    case DueDateRole:
        return it.due > 0
            ? QDateTime::fromMSecsSinceEpoch(it.due).toString(QStringLiteral("MMM d"))
            : QString();
    case CompletedRole:   return it.completed;
    case PriorityRole:    return it.priority;
    default:              return QVariant();
    }
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AccountIdRole] = "accountId";
    roles[UidRole] = "uid";
    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";
    roles[DueRole] = "due";
    roles[DueDateRole] = "dueDate";
    roles[CompletedRole] = "completed";
    roles[PriorityRole] = "priority";
    return roles;
}

int TaskModel::pendingCount() const
{
    int n = 0;
    for (const TaskItem &it : m_items)
        if (!it.completed) ++n;
    return n;
}

void TaskModel::reload()
{
    beginResetModel();
    m_items.clear();

    QSqlQuery q(Database::instance()->connection());
    if (q.exec(QStringLiteral("SELECT * FROM tasks ORDER BY completed ASC, due ASC"))) {
        while (q.next()) {
            TaskItem it;
            it.id = q.value(QStringLiteral("id")).toInt();
            it.accountId = q.value(QStringLiteral("account_id")).toInt();
            it.uid = q.value(QStringLiteral("uid")).toString();
            it.title = q.value(QStringLiteral("title")).toString();
            it.description = q.value(QStringLiteral("description")).toString();
            it.due = q.value(QStringLiteral("due")).toLongLong();
            it.completed = q.value(QStringLiteral("completed")).toBool();
            it.priority = q.value(QStringLiteral("priority")).toInt();
            m_items.append(it);
        }
    } else {
        qWarning() << "TaskModel reload failed:" << q.lastError().text();
    }

    endResetModel();
    emit countChanged();
}

int TaskModel::addTask(int accountId, const QString &title, const QString &description, qint64 due)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO tasks (account_id, title, description, due, completed) VALUES (?, ?, ?, ?, 0)"));
    q.addBindValue(accountId);
    q.addBindValue(title);
    q.addBindValue(description);
    q.addBindValue(due);

    if (!q.exec()) {
        qWarning() << "TaskModel addTask failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    TaskItem it;
    it.id = id;
    it.accountId = accountId;
    it.title = title;
    it.description = description;
    it.due = due;
    it.completed = false;

    int row = 0;
    while (row < m_items.size() && m_items.at(row).completed)
        ++row;

    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, it);
    endInsertRows();
    emit countChanged();

    return id;
}

bool TaskModel::hasUid(const QString &uid) const
{
    for (const TaskItem &it : m_items)
        if (it.uid == uid)
            return true;
    return false;
}

int TaskModel::addSynced(int accountId, const QString &uid, const QString &title,
                         const QString &description, qint64 due, bool completed)
{
    if (!uid.isEmpty() && hasUid(uid))
        return -1;

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO tasks (account_id, uid, title, description, due, completed) VALUES (?, ?, ?, ?, ?, ?)"));
    q.addBindValue(accountId);
    q.addBindValue(uid);
    q.addBindValue(title);
    q.addBindValue(description);
    q.addBindValue(due);
    q.addBindValue(completed ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "TaskModel addSynced failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    TaskItem it;
    it.id = id;
    it.accountId = accountId;
    it.uid = uid;
    it.title = title;
    it.description = description;
    it.due = due;
    it.completed = completed;

    int row = 0;
    while (row < m_items.size() && m_items.at(row).completed)
        ++row;
    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, it);
    endInsertRows();
    emit countChanged();

    return id;
}

void TaskModel::removeTask(int id)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM tasks WHERE id = ?"));
    q.addBindValue(id);
    q.exec();

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
            emit countChanged();
            break;
        }
    }
}

void TaskModel::setCompleted(int id, bool completed)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("UPDATE tasks SET completed = ? WHERE id = ?"));
    q.addBindValue(completed ? 1 : 0);
    q.addBindValue(id);
    q.exec();

    int found = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            found = i;
            break;
        }
    }
    if (found < 0)
        return;

    m_items[found].completed = completed;
    emit dataChanged(index(found), index(found), { CompletedRole });

    // Re-sort: completed items sink to the bottom.
    beginResetModel();
    std::stable_sort(m_items.begin(), m_items.end(), [](const TaskItem &a, const TaskItem &b) {
        if (a.completed != b.completed) return !a.completed;
        return a.due < b.due;
    });
    endResetModel();
    emit countChanged();
}
