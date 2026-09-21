#include "mailmodel.h"
#include "storage/database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

MailModel::MailModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MailModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant MailModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const MailItem &it = m_items.at(index.row());
    switch (role) {
    case IdRole:        return it.id;
    case AccountIdRole: return it.accountId;
    case UidRole:       return it.uid;
    case SubjectRole:   return it.subject;
    case SenderRole:    return it.sender;
    case RecipientRole: return it.recipient;
    case PreviewRole:   return it.preview;
    case BodyRole:      return it.body;
    case TimestampRole: return it.timestamp;
    case DateRole:
        return QDateTime::fromMSecsSinceEpoch(it.timestamp).toString(QStringLiteral("MMM d, h:mm AP"));
    case ReadRole:      return it.read;
    case StarredRole:   return it.starred;
    case TrashedRole:   return it.trashed;
    default:            return QVariant();
    }
}

QHash<int, QByteArray> MailModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AccountIdRole] = "accountId";
    roles[UidRole] = "uid";
    roles[SubjectRole] = "subject";
    roles[SenderRole] = "sender";
    roles[RecipientRole] = "recipient";
    roles[PreviewRole] = "preview";
    roles[BodyRole] = "body";
    roles[TimestampRole] = "timestamp";
    roles[DateRole] = "date";
    roles[ReadRole] = "read";
    roles[StarredRole] = "starred";
    roles[TrashedRole] = "trashed";
    return roles;
}

void MailModel::reload()
{
    beginResetModel();
    m_items.clear();

    QSqlQuery q(Database::instance()->connection());
    if (q.exec(QStringLiteral("SELECT * FROM emails ORDER BY timestamp DESC"))) {
        while (q.next()) {
            MailItem it;
            it.id = q.value(QStringLiteral("id")).toInt();
            it.accountId = q.value(QStringLiteral("account_id")).toInt();
            it.uid = q.value(QStringLiteral("uid")).toString();
            it.subject = q.value(QStringLiteral("subject")).toString();
            it.sender = q.value(QStringLiteral("sender")).toString();
            it.recipient = q.value(QStringLiteral("recipient")).toString();
            it.preview = q.value(QStringLiteral("preview")).toString();
            it.body = q.value(QStringLiteral("body")).toString();
            it.timestamp = q.value(QStringLiteral("timestamp")).toLongLong();
            it.read = q.value(QStringLiteral("is_read")).toBool();
            it.starred = q.value(QStringLiteral("is_starred")).toBool();
            it.trashed = q.value(QStringLiteral("is_trashed")).toBool();
            m_items.append(it);
        }
    } else {
        qWarning() << "MailModel reload failed:" << q.lastError().text();
    }

    endResetModel();
    emit countChanged();
}

int MailModel::add(int accountId, const QString &subject, const QString &sender,
                   const QString &recipient, const QString &body, qint64 timestamp)
{
    if (timestamp == 0)
        timestamp = QDateTime::currentMSecsSinceEpoch();

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO emails (account_id, subject, sender, recipient, body, preview, timestamp, is_read, is_starred) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 0, 0)"));
    q.addBindValue(accountId);
    q.addBindValue(subject);
    q.addBindValue(sender);
    q.addBindValue(recipient);
    q.addBindValue(body);
    q.addBindValue(body.left(160));
    q.addBindValue(timestamp);

    if (!q.exec()) {
        qWarning() << "MailModel add failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    MailItem it;
    it.id = id;
    it.accountId = accountId;
    it.subject = subject;
    it.sender = sender;
    it.recipient = recipient;
    it.preview = body.left(160);
    it.body = body;
    it.timestamp = timestamp;
    it.read = false;
    it.starred = false;

    beginInsertRows(QModelIndex(), 0, 0);
    m_items.prepend(it);
    endInsertRows();
    emit countChanged();

    return id;
}

bool MailModel::hasUid(const QString &uid) const
{
    for (const MailItem &it : m_items)
        if (it.uid == uid)
            return true;
    return false;
}

int MailModel::addSynced(int accountId, const QString &uid, const QString &subject,
                         const QString &sender, const QString &recipient, const QString &body,
                         qint64 timestamp, bool seen, bool starred)
{
    if (!uid.isEmpty() && hasUid(uid))
        return -1;

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO emails (account_id, uid, subject, sender, recipient, body, preview, timestamp, is_read, is_starred) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(accountId);
    q.addBindValue(uid);
    q.addBindValue(subject);
    q.addBindValue(sender);
    q.addBindValue(recipient);
    q.addBindValue(body);
    q.addBindValue(body.left(160));
    q.addBindValue(timestamp);
    q.addBindValue(seen ? 1 : 0);
    q.addBindValue(starred ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "MailModel addSynced failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    MailItem it;
    it.id = id;
    it.accountId = accountId;
    it.uid = uid;
    it.subject = subject;
    it.sender = sender;
    it.recipient = recipient;
    it.preview = body.left(160);
    it.body = body;
    it.timestamp = timestamp;
    it.read = seen;
    it.starred = starred;

    // Insert sorted by timestamp descending.
    int row = 0;
    while (row < m_items.size() && m_items.at(row).timestamp >= timestamp)
        ++row;
    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, it);
    endInsertRows();
    emit countChanged();

    return id;
}

void MailModel::setBodyByUid(const QString &uid, const QString &body)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].uid == uid) {
            m_items[i].body = body;
            emit dataChanged(index(i), index(i), { BodyRole, PreviewRole });
            QSqlQuery q(Database::instance()->connection());
            q.prepare(QStringLiteral("UPDATE emails SET body = ? WHERE uid = ?"));
            q.addBindValue(body);
            q.addBindValue(uid);
            q.exec();
            return;
        }
    }
}

void MailModel::remove(int id)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM emails WHERE id = ?"));
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

void MailModel::setRead(int id, bool read)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("UPDATE emails SET is_read = ? WHERE id = ?"));
    q.addBindValue(read ? 1 : 0);
    q.addBindValue(id);
    q.exec();

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            m_items[i].read = read;
            emit dataChanged(index(i), index(i), { ReadRole });
            break;
        }
    }
}

void MailModel::toggleStarred(int id)
{
    bool target = false;
    for (const MailItem &it : m_items) {
        if (it.id == id) {
            target = !it.starred;
            break;
        }
    }

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("UPDATE emails SET is_starred = ? WHERE id = ?"));
    q.addBindValue(target ? 1 : 0);
    q.addBindValue(id);
    q.exec();

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            m_items[i].starred = target;
            emit dataChanged(index(i), index(i), { StarredRole });
            break;
        }
    }
}

QVariantMap MailModel::get(int row) const
{
    QVariantMap m;
    if (row < 0 || row >= m_items.size())
        return m;

    const MailItem &it = m_items.at(row);
    m[QStringLiteral("id")] = it.id;
    m[QStringLiteral("uid")] = it.uid;
    m[QStringLiteral("subject")] = it.subject;
    m[QStringLiteral("sender")] = it.sender;
    m[QStringLiteral("recipient")] = it.recipient;
    m[QStringLiteral("body")] = it.body;
    m[QStringLiteral("date")] = QDateTime::fromMSecsSinceEpoch(it.timestamp).toString(QStringLiteral("MMM d, yyyy h:mm AP"));
    return m;
}

QVariantMap MailModel::getById(int id) const
{
    for (int i = 0; i < m_items.size(); ++i)
        if (m_items.at(i).id == id)
            return get(i);
    return QVariantMap();
}

int MailModel::trashCount() const
{
    int n = 0;
    for (const MailItem &it : m_items)
        if (it.trashed)
            ++n;
    return n;
}

void MailModel::setTrashed(int id, bool trashed)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("UPDATE emails SET is_trashed = ? WHERE id = ?"));
    q.addBindValue(trashed ? 1 : 0);
    q.addBindValue(id);
    q.exec();

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            m_items[i].trashed = trashed;
            emit dataChanged(index(i), index(i), { TrashedRole });
            break;
        }
    }
    emit trashCountChanged();
}

void MailModel::trash(int id)
{
    setTrashed(id, true);
}

void MailModel::restore(int id)
{
    setTrashed(id, false);
}

void MailModel::purge(int id)
{
    remove(id);
    emit trashCountChanged();
}

void MailModel::emptyTrash()
{
    QSqlQuery q(Database::instance()->connection());
    q.exec(QStringLiteral("DELETE FROM emails WHERE is_trashed = 1"));

    for (int i = m_items.size() - 1; i >= 0; --i) {
        if (m_items.at(i).trashed) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
        }
    }
    emit countChanged();
    emit trashCountChanged();
}
