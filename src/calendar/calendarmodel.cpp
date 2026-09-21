#include "calendarmodel.h"
#include "storage/database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

CalendarModel::CalendarModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CalendarModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant CalendarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const EventItem &it = m_items.at(index.row());
    switch (role) {
    case IdRole:        return it.id;
    case AccountIdRole: return it.accountId;
    case CalendarIdRole: return it.calendarId;
    case UidRole:       return it.uid;
    case TitleRole:     return it.title;
    case DescriptionRole: return it.description;
    case LocationRole:  return it.location;
    case StartRole:     return it.start;
    case EndRole:       return it.end;
    case StartDateRole:
        return QDateTime::fromMSecsSinceEpoch(it.start).toString(QStringLiteral("h:mm AP"));
    case EndDateRole:
        return QDateTime::fromMSecsSinceEpoch(it.end).toString(QStringLiteral("h:mm AP"));
    case AllDayRole:    return it.allDay;
    case ReminderRole:  return it.reminder;
    default:            return QVariant();
    }
}

QHash<int, QByteArray> CalendarModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AccountIdRole] = "accountId";
    roles[CalendarIdRole] = "calendarId";
    roles[UidRole] = "uid";
    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";
    roles[LocationRole] = "location";
    roles[StartRole] = "start";
    roles[EndRole] = "end";
    roles[StartDateRole] = "startDate";
    roles[EndDateRole] = "endDate";
    roles[AllDayRole] = "allDay";
    roles[ReminderRole] = "reminder";
    return roles;
}

void CalendarModel::reload()
{
    beginResetModel();
    m_items.clear();

    QSqlQuery q(Database::instance()->connection());
    if (q.exec(QStringLiteral("SELECT * FROM events ORDER BY start ASC"))) {
        while (q.next()) {
            EventItem it;
            it.id = q.value(QStringLiteral("id")).toInt();
            it.accountId = q.value(QStringLiteral("account_id")).toInt();
            it.calendarId = q.value(QStringLiteral("calendar_id")).toInt();
            it.uid = q.value(QStringLiteral("uid")).toString();
            it.title = q.value(QStringLiteral("title")).toString();
            it.description = q.value(QStringLiteral("description")).toString();
            it.location = q.value(QStringLiteral("location")).toString();
            it.start = q.value(QStringLiteral("start")).toLongLong();
            it.end = q.value(QStringLiteral("end")).toLongLong();
            it.allDay = q.value(QStringLiteral("all_day")).toBool();
            it.reminder = q.value(QStringLiteral("reminder")).toInt();
            m_items.append(it);
        }
    } else {
        qWarning() << "CalendarModel reload failed:" << q.lastError().text();
    }

    endResetModel();
    emit countChanged();
}

int CalendarModel::addEvent(int accountId, int calendarId, const QString &title, const QString &description,
                            const QString &location, qint64 start, qint64 end, bool allDay)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO events (account_id, calendar_id, title, description, location, start, end, all_day) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(accountId);
    q.addBindValue(calendarId);
    q.addBindValue(title);
    q.addBindValue(description);
    q.addBindValue(location);
    q.addBindValue(start);
    q.addBindValue(end);
    q.addBindValue(allDay ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "CalendarModel addEvent failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    EventItem it;
    it.id = id;
    it.accountId = accountId;
    it.calendarId = calendarId;
    it.title = title;
    it.description = description;
    it.location = location;
    it.start = start;
    it.end = end;
    it.allDay = allDay;

    int row = 0;
    while (row < m_items.size() && m_items.at(row).start <= start)
        ++row;

    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, it);
    endInsertRows();
    emit countChanged();

    return id;
}

bool CalendarModel::hasUid(const QString &uid) const
{
    for (const EventItem &it : m_items)
        if (it.uid == uid)
            return true;
    return false;
}

int CalendarModel::addSynced(int accountId, int calendarId, const QString &uid, const QString &title,
                             const QString &description, const QString &location,
                             qint64 start, qint64 end, bool allDay)
{
    if (!uid.isEmpty() && hasUid(uid))
        return -1;

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO events (account_id, calendar_id, uid, title, description, location, start, end, all_day) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(accountId);
    q.addBindValue(calendarId);
    q.addBindValue(uid);
    q.addBindValue(title);
    q.addBindValue(description);
    q.addBindValue(location);
    q.addBindValue(start);
    q.addBindValue(end);
    q.addBindValue(allDay ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "CalendarModel addSynced failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    EventItem it;
    it.id = id;
    it.accountId = accountId;
    it.calendarId = calendarId;
    it.uid = uid;
    it.title = title;
    it.description = description;
    it.location = location;
    it.start = start;
    it.end = end;
    it.allDay = allDay;

    int row = 0;
    while (row < m_items.size() && m_items.at(row).start <= start)
        ++row;
    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, it);
    endInsertRows();
    emit countChanged();

    return id;
}

void CalendarModel::removeEvent(int id)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM events WHERE id = ?"));
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

QVariantList CalendarModel::eventsForDate(qint64 startOfDayMs) const
{
    const qint64 endOfDayMs = startOfDayMs + 24LL * 60 * 60 * 1000;

    QVariantList result;
    for (const EventItem &it : m_items) {
        const bool overlaps = (it.start < endOfDayMs && it.end > startOfDayMs);
        if (overlaps) {
            QVariantMap m;
            m[QStringLiteral("id")] = it.id;
            m[QStringLiteral("uid")] = it.uid;
            m[QStringLiteral("calendarId")] = it.calendarId;
            m[QStringLiteral("title")] = it.title;
            m[QStringLiteral("start")] = it.start;
            m[QStringLiteral("end")] = it.end;
            m[QStringLiteral("allDay")] = it.allDay;
            m[QStringLiteral("startDate")] = QDateTime::fromMSecsSinceEpoch(it.start).toString(QStringLiteral("h:mm AP"));
            m[QStringLiteral("endDate")] = QDateTime::fromMSecsSinceEpoch(it.end).toString(QStringLiteral("h:mm AP"));
            result.append(m);
        }
    }
    return result;
}

bool CalendarModel::hasEventsOn(qint64 startOfDayMs) const
{
    const qint64 endOfDayMs = startOfDayMs + 24LL * 60 * 60 * 1000;
    for (const EventItem &it : m_items) {
        if (it.start < endOfDayMs && it.end > startOfDayMs)
            return true;
    }
    return false;
}
