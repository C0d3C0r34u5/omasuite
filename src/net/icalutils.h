#ifndef ICALUTILS_H
#define ICALUTILS_H

#include <QVariantList>
#include <QVariantMap>
#include <QString>

namespace ICalUtils {

// Parse a VEVENT-containing iCalendar payload into a list of event maps.
QVariantList parseEvents(const QString &ics);

// Parse a VTODO-containing iCalendar payload into a list of task maps.
QVariantList parseTodos(const QString &ics);

// Build an iCalendar document containing a single VEVENT.
QString buildEvent(const QVariantMap &event);

// Build an iCalendar document containing a single VTODO.
QString buildTodo(const QVariantMap &task);

// Format a timestamp (ms) as an iCalendar UTC date-time.
QString formatDateTime(qint64 ms);
QString formatDate(qint64 ms);

}

#endif // ICALUTILS_H
