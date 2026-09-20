#ifndef CALENDARMODEL_H
#define CALENDARMODEL_H

#include <QAbstractListModel>
#include <QList>

struct EventItem {
    int id = 0;
    int accountId = 0;
    QString uid;
    QString title;
    QString description;
    QString location;
    qint64 start = 0;
    qint64 end = 0;
    bool allDay = false;
    int reminder = 0;
};

class CalendarModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AccountIdRole,
        UidRole,
        TitleRole,
        DescriptionRole,
        LocationRole,
        StartRole,
        EndRole,
        StartDateRole,
        EndDateRole,
        AllDayRole,
        ReminderRole
    };
    Q_ENUM(Roles)

    explicit CalendarModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_items.size(); }

    Q_INVOKABLE void reload();
    Q_INVOKABLE int addEvent(int accountId, const QString &title, const QString &description,
                             const QString &location, qint64 start, qint64 end, bool allDay = false);
    Q_INVOKABLE int addSynced(int accountId, const QString &uid, const QString &title,
                              const QString &description, const QString &location,
                              qint64 start, qint64 end, bool allDay);
    Q_INVOKABLE bool hasUid(const QString &uid) const;
    Q_INVOKABLE void removeEvent(int id);
    Q_INVOKABLE QVariantList eventsForDate(qint64 startOfDayMs) const;
    Q_INVOKABLE bool hasEventsOn(qint64 startOfDayMs) const;

signals:
    void countChanged();

private:
    QList<EventItem> m_items;
};

#endif // CALENDARMODEL_H
