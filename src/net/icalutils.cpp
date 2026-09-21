#include "icalutils.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include <QUuid>
#include <QTimeZone>

namespace ICalUtils {

static QString unfold(const QString &text)
{
    QString out = text;
    out.replace(QStringLiteral("\r\n "), QString());
    out.replace(QStringLiteral("\r\n\t"), QString());
    out.replace(QStringLiteral("\n "), QString());
    out.replace(QStringLiteral("\n\t"), QString());
    return out;
}

static QStringList blocks(const QString &text, const QString &type)
{
    QStringList result;
    const QString begin = QStringLiteral("BEGIN:") + type;
    const QString end = QStringLiteral("END:") + type;

    int pos = 0;
    while ((pos = text.indexOf(begin, pos)) >= 0) {
        const int stop = text.indexOf(end, pos);
        if (stop < 0)
            break;
        result.append(text.mid(pos, stop + end.length() - pos));
        pos = stop + end.length();
    }
    return result;
}

static QVariantMap parseProperties(const QString &block)
{
    QVariantMap props;
    QString normalized = block;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    const QStringList lines = normalized.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &raw : lines) {
        QString line = raw.trimmed();
        if (line.startsWith(QStringLiteral("BEGIN:")) || line.startsWith(QStringLiteral("END:")))
            continue;

        int idx = line.indexOf(QLatin1Char(':'));
        if (idx < 0)
            continue;

        QString name = line.left(idx);
        // strip parameters
        const int semi = name.indexOf(QLatin1Char(';'));
        if (semi >= 0)
            name = name.left(semi);
        name = name.toUpper();

        QString value = line.mid(idx + 1);
        // unescape
        value.replace(QStringLiteral("\\,"), QStringLiteral(","));
        value.replace(QStringLiteral("\\;"), QStringLiteral(";"));
        value.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
        value.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));

        props[name] = value;
    }
    return props;
}

struct DateTimeVal {
    qint64 ms = 0;
    bool allDay = false;
    bool valid = false;
};

static DateTimeVal parseICalDateTime(const QString &value)
{
    DateTimeVal r;
    QString v = value.trimmed();

    if (v.length() == 8) { // YYYYMMDD (all-day)
        const QDate d = QDate::fromString(v, QStringLiteral("yyyyMMdd"));
        if (d.isValid()) {
            r.ms = QDateTime(d, QTime(0, 0)).toMSecsSinceEpoch();
            r.allDay = true;
            r.valid = true;
        }
        return r;
    }

    // YYYYMMDDTHHMMSS[Z]
    bool utc = v.endsWith(QLatin1Char('Z'));
    if (utc)
        v.chop(1);

    QDateTime dt = QDateTime::fromString(v, QStringLiteral("yyyyMMddTHHmmss"));
    if (dt.isValid()) {
        if (utc) {
            dt.setTimeZone(QTimeZone::utc());
        }
        r.ms = dt.toMSecsSinceEpoch();
        r.valid = true;
    }
    return r;
}

QVariantList parseEvents(const QString &ics)
{
    QVariantList out;
    const QString unfolded = unfold(ics);
    const QStringList evts = blocks(unfolded, QStringLiteral("VEVENT"));

    for (const QString &blk : evts) {
        const QVariantMap p = parseProperties(blk);

        const DateTimeVal start = parseICalDateTime(p.value(QStringLiteral("DTSTART")).toString());
        DateTimeVal end = parseICalDateTime(p.value(QStringLiteral("DTEND")).toString());
        if (!end.valid) {
            // default: 1 hour, or 1 day for all-day
            end.ms = start.ms + (start.allDay ? 24LL * 3600 * 1000 : 3600 * 1000);
            end.allDay = start.allDay;
            end.valid = true;
        }
        if (!start.valid)
            continue;

        QVariantMap m;
        m[QStringLiteral("uid")] = p.value(QStringLiteral("UID")).toString();
        m[QStringLiteral("title")] = p.value(QStringLiteral("SUMMARY")).toString();
        m[QStringLiteral("description")] = p.value(QStringLiteral("DESCRIPTION")).toString();
        m[QStringLiteral("location")] = p.value(QStringLiteral("LOCATION")).toString();
        m[QStringLiteral("start")] = start.ms;
        m[QStringLiteral("end")] = end.ms;
        m[QStringLiteral("allDay")] = start.allDay;
        out.append(m);
    }
    return out;
}

QVariantList parseTodos(const QString &ics)
{
    QVariantList out;
    const QString unfolded = unfold(ics);
    const QStringList todos = blocks(unfolded, QStringLiteral("VTODO"));

    for (const QString &blk : todos) {
        const QVariantMap p = parseProperties(blk);

        QVariantMap m;
        m[QStringLiteral("uid")] = p.value(QStringLiteral("UID")).toString();
        m[QStringLiteral("title")] = p.value(QStringLiteral("SUMMARY")).toString();
        m[QStringLiteral("description")] = p.value(QStringLiteral("DESCRIPTION")).toString();
        const DateTimeVal due = parseICalDateTime(p.value(QStringLiteral("DUE")).toString());
        m[QStringLiteral("due")] = due.valid ? due.ms : 0;
        m[QStringLiteral("completed")] = !p.value(QStringLiteral("STATUS")).toString().isEmpty()
            && p.value(QStringLiteral("STATUS")).toString() == QStringLiteral("COMPLETED");
        out.append(m);
    }
    return out;
}

static QString uidOrNew(const QVariantMap &m)
{
    QString uid = m.value(QStringLiteral("uid")).toString();
    if (uid.isEmpty())
        uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return uid;
}

QString formatDateTime(qint64 ms)
{
    return QDateTime::fromMSecsSinceEpoch(ms).toUTC().toString(QStringLiteral("yyyyMMddTHHmmssZ"));
}

QString formatDate(qint64 ms)
{
    return QDateTime::fromMSecsSinceEpoch(ms).date().toString(QStringLiteral("yyyyMMdd"));
}

QString buildEvent(const QVariantMap &event)
{
    const QString uid = uidOrNew(event);
    const bool allDay = event.value(QStringLiteral("allDay")).toBool();
    const qint64 start = event.value(QStringLiteral("start")).toLongLong();
    const qint64 end = event.value(QStringLiteral("end")).toLongLong();

    QString ics;
    ics += QStringLiteral("BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//OmaSuite//EN\r\nBEGIN:VEVENT\r\n");
    ics += QStringLiteral("UID:") + uid + QStringLiteral("\r\n");
    if (allDay) {
        ics += QStringLiteral("DTSTART;VALUE=DATE:") + formatDate(start) + QStringLiteral("\r\n");
        ics += QStringLiteral("DTEND;VALUE=DATE:") + formatDate(end) + QStringLiteral("\r\n");
    } else {
        ics += QStringLiteral("DTSTART:") + formatDateTime(start) + QStringLiteral("\r\n");
        ics += QStringLiteral("DTEND:") + formatDateTime(end) + QStringLiteral("\r\n");
    }
    QString summary = event.value(QStringLiteral("title")).toString();
    QString desc = event.value(QStringLiteral("description")).toString();
    const QString loc = event.value(QStringLiteral("location")).toString();
    if (!summary.isEmpty())
        ics += QStringLiteral("SUMMARY:") + summary.replace(QStringLiteral("\r\n"), QStringLiteral("\\n")).replace(QStringLiteral("\n"), QStringLiteral("\\n")) + QStringLiteral("\r\n");
    if (!loc.isEmpty())
        ics += QStringLiteral("LOCATION:") + loc + QStringLiteral("\r\n");
    if (!desc.isEmpty())
        ics += QStringLiteral("DESCRIPTION:") + desc.replace(QStringLiteral("\r\n"), QStringLiteral("\\n")).replace(QStringLiteral("\n"), QStringLiteral("\\n")) + QStringLiteral("\r\n");
    ics += QStringLiteral("END:VEVENT\r\nEND:VCALENDAR\r\n");
    return ics;
}

QString buildTodo(const QVariantMap &task)
{
    const QString uid = uidOrNew(task);
    QString ics;
    ics += QStringLiteral("BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//OmaSuite//EN\r\nBEGIN:VTODO\r\n");
    ics += QStringLiteral("UID:") + uid + QStringLiteral("\r\n");
    ics += QStringLiteral("SUMMARY:") + task.value(QStringLiteral("title")).toString() + QStringLiteral("\r\n");
    const QString desc = task.value(QStringLiteral("description")).toString();
    if (!desc.isEmpty())
        ics += QStringLiteral("DESCRIPTION:") + desc + QStringLiteral("\r\n");
    const qint64 due = task.value(QStringLiteral("due")).toLongLong();
    if (due > 0)
        ics += QStringLiteral("DUE:") + formatDateTime(due) + QStringLiteral("\r\n");
    if (task.value(QStringLiteral("completed")).toBool())
        ics += QStringLiteral("STATUS:COMPLETED\r\n");
    ics += QStringLiteral("END:VTODO\r\nEND:VCALENDAR\r\n");
    return ics;
}

}
