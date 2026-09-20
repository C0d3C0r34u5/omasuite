#include "contactmodel.h"
#include "storage/database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ContactModel::ContactModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ContactModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const ContactItem &it = m_items.at(index.row());
    switch (role) {
    case IdRole:         return it.id;
    case AccountIdRole:  return it.accountId;
    case UidRole:        return it.uid;
    case FirstNameRole:  return it.firstName;
    case LastNameRole:   return it.lastName;
    case FullNameRole:   return (it.firstName + QLatin1Char(' ') + it.lastName).trimmed();
    case EmailRole:      return it.email;
    case PhoneRole:      return it.phone;
    case OrganizationRole: return it.organization;
    case NotesRole:      return it.notes;
    case InitialsRole: {
        QString in;
        if (!it.firstName.isEmpty()) in += it.firstName.left(1).toUpper();
        if (!it.lastName.isEmpty()) in += it.lastName.left(1).toUpper();
        if (in.isEmpty() && !it.email.isEmpty()) in = it.email.left(1).toUpper();
        return in;
    }
    default:             return QVariant();
    }
}

QHash<int, QByteArray> ContactModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AccountIdRole] = "accountId";
    roles[UidRole] = "uid";
    roles[FirstNameRole] = "firstName";
    roles[LastNameRole] = "lastName";
    roles[FullNameRole] = "fullName";
    roles[EmailRole] = "email";
    roles[PhoneRole] = "phone";
    roles[OrganizationRole] = "organization";
    roles[NotesRole] = "notes";
    roles[InitialsRole] = "initials";
    return roles;
}

bool ContactModel::matches(const ContactItem &it, const QString &filter)
{
    if (filter.isEmpty())
        return true;
    const QString f = filter.toLower();
    return it.firstName.toLower().contains(f)
        || it.lastName.toLower().contains(f)
        || it.email.toLower().contains(f)
        || it.phone.toLower().contains(f)
        || it.organization.toLower().contains(f);
}

void ContactModel::applyFilter()
{
    beginResetModel();
    m_items.clear();
    for (const ContactItem &it : m_all) {
        if (matches(it, m_filter))
            m_items.append(it);
    }
    endResetModel();
    emit countChanged();
}

void ContactModel::setFilterText(const QString &text)
{
    if (m_filter == text)
        return;
    m_filter = text;
    applyFilter();
    emit filterChanged();
}

void ContactModel::reload()
{
    m_all.clear();

    QSqlQuery q(Database::instance()->connection());
    if (q.exec(QStringLiteral("SELECT * FROM contacts ORDER BY first_name ASC, last_name ASC"))) {
        while (q.next()) {
            ContactItem it;
            it.id = q.value(QStringLiteral("id")).toInt();
            it.accountId = q.value(QStringLiteral("account_id")).toInt();
            it.uid = q.value(QStringLiteral("uid")).toString();
            it.firstName = q.value(QStringLiteral("first_name")).toString();
            it.lastName = q.value(QStringLiteral("last_name")).toString();
            it.email = q.value(QStringLiteral("email")).toString();
            it.phone = q.value(QStringLiteral("phone")).toString();
            it.organization = q.value(QStringLiteral("organization")).toString();
            it.notes = q.value(QStringLiteral("notes")).toString();
            m_all.append(it);
        }
    } else {
        qWarning() << "ContactModel reload failed:" << q.lastError().text();
    }

    applyFilter();
}

int ContactModel::addContact(int accountId, const QString &firstName, const QString &lastName,
                             const QString &email, const QString &phone, const QString &organization)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO contacts (account_id, first_name, last_name, email, phone, organization) "
        "VALUES (?, ?, ?, ?, ?, ?)"));
    q.addBindValue(accountId);
    q.addBindValue(firstName);
    q.addBindValue(lastName);
    q.addBindValue(email);
    q.addBindValue(phone);
    q.addBindValue(organization);

    if (!q.exec()) {
        qWarning() << "ContactModel addContact failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    ContactItem it;
    it.id = id;
    it.accountId = accountId;
    it.firstName = firstName;
    it.lastName = lastName;
    it.email = email;
    it.phone = phone;
    it.organization = organization;

    m_all.append(it);

    if (matches(it, m_filter)) {
        int row = m_items.size();
        beginInsertRows(QModelIndex(), row, row);
        m_items.append(it);
        endInsertRows();
        emit countChanged();
    }

    return id;
}

bool ContactModel::hasUid(const QString &uid) const
{
    for (const ContactItem &it : m_all)
        if (it.uid == uid)
            return true;
    return false;
}

int ContactModel::addSynced(int accountId, const QString &uid, const QString &firstName,
                            const QString &lastName, const QString &email, const QString &phone,
                            const QString &organization)
{
    if (!uid.isEmpty() && hasUid(uid))
        return -1;

    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral(
        "INSERT INTO contacts (account_id, uid, first_name, last_name, email, phone, organization) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(accountId);
    q.addBindValue(uid);
    q.addBindValue(firstName);
    q.addBindValue(lastName);
    q.addBindValue(email);
    q.addBindValue(phone);
    q.addBindValue(organization);

    if (!q.exec()) {
        qWarning() << "ContactModel addSynced failed:" << q.lastError().text();
        return -1;
    }

    const int id = q.lastInsertId().toInt();

    ContactItem it;
    it.id = id;
    it.accountId = accountId;
    it.uid = uid;
    it.firstName = firstName;
    it.lastName = lastName;
    it.email = email;
    it.phone = phone;
    it.organization = organization;

    m_all.append(it);
    if (matches(it, m_filter)) {
        int row = m_items.size();
        beginInsertRows(QModelIndex(), row, row);
        m_items.append(it);
        endInsertRows();
        emit countChanged();
    }

    return id;
}

void ContactModel::removeContact(int id)
{
    QSqlQuery q(Database::instance()->connection());
    q.prepare(QStringLiteral("DELETE FROM contacts WHERE id = ?"));
    q.addBindValue(id);
    q.exec();

    for (int i = 0; i < m_all.size(); ++i) {
        if (m_all.at(i).id == id) {
            m_all.removeAt(i);
            break;
        }
    }

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

QVariantMap ContactModel::get(int row) const
{
    QVariantMap m;
    if (row < 0 || row >= m_items.size())
        return m;

    const ContactItem &it = m_items.at(row);
    m[QStringLiteral("id")] = it.id;
    m[QStringLiteral("uid")] = it.uid;
    m[QStringLiteral("firstName")] = it.firstName;
    m[QStringLiteral("lastName")] = it.lastName;
    m[QStringLiteral("fullName")] = (it.firstName + QLatin1Char(' ') + it.lastName).trimmed();
    m[QStringLiteral("email")] = it.email;
    m[QStringLiteral("phone")] = it.phone;
    m[QStringLiteral("organization")] = it.organization;
    m[QStringLiteral("notes")] = it.notes;
    return m;
}
