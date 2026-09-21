#ifndef CALENDARLISTMODEL_H
#define CALENDARLISTMODEL_H

#include <QAbstractListModel>
#include <QList>

struct CalendarItem {
    int id = 0;
    int accountId = 0;
    QString name;
    QString color;
    QString type;
    QString sourceUrl;
    bool visible = true;
};

class CalendarListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QVariantList visibleCalendarIds READ visibleCalendarIds NOTIFY visibilityChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AccountIdRole,
        NameRole,
        ColorRole,
        TypeRole,
        SourceUrlRole,
        VisibleRole
    };
    Q_ENUM(Roles)

    explicit CalendarListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_items.size(); }

    Q_INVOKABLE void reload();
    Q_INVOKABLE int addLocalCalendar(const QString &name, const QString &color = QString());
    Q_INVOKABLE void removeCalendar(int id);
    Q_INVOKABLE void setVisible(int id, bool visible);
    Q_INVOKABLE int upsertCalendar(int accountId, const QString &name, const QString &color,
                                   const QString &type, const QString &sourceUrl);
    Q_INVOKABLE void clearAccountCalendars(int accountId);

    int calendarById(int id) const;
    QVariantList visibleCalendarIds() const;
    Q_INVOKABLE bool isVisible(int id) const;
    Q_INVOKABLE QString sourceUrl(int id) const;
    Q_INVOKABLE QString typeOf(int id) const;
    Q_INVOKABLE int accountIdOf(int id) const;

signals:
    void countChanged();
    void visibilityChanged();

private:
    void appendItem(const CalendarItem &it);
    void ensureLocalCalendar();
    QList<CalendarItem> m_items;
};

#endif // CALENDARLISTMODEL_H
