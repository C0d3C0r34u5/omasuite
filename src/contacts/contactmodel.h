#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include <QAbstractListModel>
#include <QList>

struct ContactItem {
    int id = 0;
    int accountId = 0;
    QString uid;
    QString firstName;
    QString lastName;
    QString email;
    QString phone;
    QString organization;
    QString notes;
};

class ContactModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AccountIdRole,
        UidRole,
        FirstNameRole,
        LastNameRole,
        FullNameRole,
        EmailRole,
        PhoneRole,
        OrganizationRole,
        NotesRole,
        InitialsRole
    };
    Q_ENUM(Roles)

    explicit ContactModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_items.size(); }

    QString filterText() const { return m_filter; }
    void setFilterText(const QString &text);

    Q_INVOKABLE void reload();
    Q_INVOKABLE int addContact(int accountId, const QString &firstName, const QString &lastName,
                               const QString &email, const QString &phone = QString(),
                               const QString &organization = QString());
    Q_INVOKABLE int addSynced(int accountId, const QString &uid, const QString &firstName,
                              const QString &lastName, const QString &email, const QString &phone,
                              const QString &organization);
    Q_INVOKABLE bool hasUid(const QString &uid) const;
    Q_INVOKABLE void removeContact(int id);
    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE void setFilter(const QString &text) { setFilterText(text); }

signals:
    void countChanged();
    void filterChanged();

private:
    void applyFilter();
    static bool matches(const ContactItem &it, const QString &filter);

    QList<ContactItem> m_all;
    QList<ContactItem> m_items;
    QString m_filter;
};

#endif // CONTACTMODEL_H
