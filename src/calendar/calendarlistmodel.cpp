#include "calendarlistmodel.h"
#include "storage/database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

CalendarListModel::CalendarListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CalendarListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant CalendarListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const CalendarItem &it = m_items.at(index.row());
    switch (role) {
    case IdRole:         return it.id;
    case AccountIdRole:  return it.accountId;
    case NameRole:       return it.name;
    case ColorRole:      return it.color;
    case TypeRole:       return it.type;
    case SourceUrlRole:  return it.sourceUrl;
    case VisibleRole:    return it.visible;
    default:             return QVariant();
    }
}

QHash<int, QByteArray> CalendarListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AccountIdRole] = "accountId";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    roles[TypeRole] = "type";
    roles[SourceUrlRole] = "sourceUrl";
    roles[VisibleRole] = "visible";
    return roles;
}

void CalendarListModel::reload()
{
    beginResetModel();
    m_items.clear();

    QSqlQuery q(Database::instance()->connection());
    if (q.exec(QStringLiteral("SELECT * FROM calendars ORDER BY id ASC"))) {
        while (q.next()) {
            CalendarItem it;
            it.id = q.value(QStringLiteral("id")).toInt();
            it.accountId = q.value(QStringLiteral("account_id")).toInt();
            it.name = q.value(QStringLiteral("name")).toString();
            it.color = q.value(QStringLiteral("color")).toString();
            it.type = q.value(QStringLiteral("type")).toString();
            it.sourceUrl = q.value(QStringLiteral("source_url")).toString();
            it.visible = q.value(QStringLiteral("visible")).toBool();
            m_items.append(it);
        }
    } else {
        qWarning() << "CalendarListModel reload failed:" << q.lastError().text();
    }

    endResetModel();
    emit countChanged();

    ensureLocalCalendar();
}

void CalendarListModel::ensureLocalCalendar()
{
    for (const CalendarItem &it : m_items)
        if (it.accountId == 0 && it.type == QStringLiteral("local"))
            return;

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO calendars (account_id, name, color, type, source_url, visible) "
        "VALUES (0, ?, ?, 'local', '', 1)"));
    q.addBindValue(QStringLiteral("Local"));
    q.addBindValue(QStringLiteral("#4CAF50"));
    if (!q.exec()) {
        qWarning() << "ensureLocalCalendar failed:" << q.lastError().text();
        return;
    }

    CalendarItem it;
    it.id = q.lastInsertId().toInt();
    it.accountId = 0;
    it.name = QStringLiteral("Local");
    it.color = QStringLiteral("#4CAF50");
    it.type = QStringLiteral("local");
    it.visible = true;

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(it);
    endInsertRows();
    emit countChanged();
}

int CalendarListModel::addLocalCalendar(const QString &name, const QString &color)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO calendars (account_id, name, color, type, source_url, visible) "
        "VALUES (0, ?, ?, 'local', '', 1)"));
    q.addBindValue(name);
    q.addBindValue(color);
    if (!q.exec()) {
        qWarning() << "addLocalCalendar failed:" << q.lastError().text();
        return -1;
    }

    CalendarItem it;
    it.id = q.lastInsertId().toInt();
    it.accountId = 0;
    it.name = name;
    it.color = color;
    it.type = QStringLiteral("local");
    it.sourceUrl.clear();
    it.visible = true;

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(it);
    endInsertRows();
    emit countChanged();
    return it.id;
}

void CalendarListModel::removeCalendar(int id)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM calendars WHERE id = ?"));
    q.addBindValue(id);
    q.exec();

    QSqlQuery ev(Database::instance()->connection());
    ev.prepare(QStringLiteral("DELETE FROM events WHERE calendar_id = ?"));
    ev.addBindValue(id);
    ev.exec();

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

void CalendarListModel::setVisible(int id, bool visible)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("UPDATE calendars SET visible = ? WHERE id = ?"));
    q.addBindValue(visible ? 1 : 0);
    q.addBindValue(id);
    q.exec();

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            m_items[i].visible = visible;
            const QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, { VisibleRole });
            break;
        }
    }
    emit visibilityChanged();
}

int CalendarListModel::upsertCalendar(int accountId, const QString &name, const QString &color,
                                      const QString &type, const QString &sourceUrl)
{
    // Find existing by account + source url (or name for local).
    for (int i = 0; i < m_items.size(); ++i) {
        const CalendarItem &it = m_items.at(i);
        if (it.accountId == accountId && it.sourceUrl == sourceUrl && !sourceUrl.isEmpty())
            return it.id;
    }

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO calendars (account_id, name, color, type, source_url, visible) "
        "VALUES (?, ?, ?, ?, ?, 1)"));
    q.addBindValue(accountId);
    q.addBindValue(name);
    q.addBindValue(color);
    q.addBindValue(type);
    q.addBindValue(sourceUrl);
    if (!q.exec()) {
        qWarning() << "upsertCalendar failed:" << q.lastError().text();
        return -1;
    }

    CalendarItem it;
    it.id = q.lastInsertId().toInt();
    it.accountId = accountId;
    it.name = name;
    it.color = color;
    it.type = type;
    it.sourceUrl = sourceUrl;
    it.visible = true;

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(it);
    endInsertRows();
    emit countChanged();
    return it.id;
}

void CalendarListModel::clearAccountCalendars(int accountId)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM calendars WHERE account_id = ?"));
    q.addBindValue(accountId);
    q.exec();

    for (int i = m_items.size() - 1; i >= 0; --i) {
        if (m_items.at(i).accountId == accountId) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
        }
    }
    emit countChanged();
}

int CalendarListModel::calendarById(int id) const
{
    for (const CalendarItem &it : m_items)
        if (it.id == id)
            return it.id;
    return 0;
}

QVariantList CalendarListModel::visibleCalendarIds() const
{
    QVariantList ids;
    for (const CalendarItem &it : m_items)
        if (it.visible)
            ids.append(it.id);
    return ids;
}

bool CalendarListModel::isVisible(int id) const
{
    for (const CalendarItem &it : m_items)
        if (it.id == id)
            return it.visible;
    return true;
}

QString CalendarListModel::sourceUrl(int id) const
{
    for (const CalendarItem &it : m_items)
        if (it.id == id)
            return it.sourceUrl;
    return QString();
}

QString CalendarListModel::typeOf(int id) const
{
    for (const CalendarItem &it : m_items)
        if (it.id == id)
            return it.type;
    return QString();
}

int CalendarListModel::accountIdOf(int id) const
{
    for (const CalendarItem &it : m_items)
        if (it.id == id)
            return it.accountId;
    return 0;
}
