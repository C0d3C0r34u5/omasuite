#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <QVariantList>
#include <QSortFilterProxyModel>

#include "account/accountmanager.h"
#include "mail/mailmodel.h"
#include "calendar/calendarmodel.h"
#include "calendar/calendarlistmodel.h"
#include "contacts/contactmodel.h"
#include "tasks/taskmodel.h"
#include "net/synccontroller.h"
#include "net/oauth2manager.h"

class MailFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum Mode { Inbox, Trash };
    Q_ENUM(Mode)

    explicit MailFilterModel(Mode mode, QObject *parent = nullptr)
        : QSortFilterProxyModel(parent), m_mode(mode) {}

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const bool trashed = idx.data(MailModel::TrashedRole).toBool();
        return m_mode == Trash ? trashed : !trashed;
    }

private:
    Mode m_mode;
};

class AppCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountManager *accounts READ accountManager CONSTANT)
    Q_PROPERTY(MailModel *mail READ mailModel CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel *mailInbox READ mailInbox CONSTANT)
    Q_PROPERTY(QSortFilterProxyModel *mailTrash READ mailTrash CONSTANT)
    Q_PROPERTY(CalendarModel *calendar READ calendarModel CONSTANT)
    Q_PROPERTY(CalendarListModel *calendars READ calendarListModel CONSTANT)
    Q_PROPERTY(ContactModel *contacts READ contactModel CONSTANT)
    Q_PROPERTY(TaskModel *tasks READ taskModel CONSTANT)
    Q_PROPERTY(SyncController *sync READ syncController CONSTANT)
    Q_PROPERTY(OAuth2Manager *oauth READ oauthManager CONSTANT)
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    explicit AppCore(QObject *parent = nullptr);

    AccountManager *accountManager() const { return m_accounts; }
    MailModel *mailModel() const { return m_mail; }
    QSortFilterProxyModel *mailInbox() const { return m_mailInbox; }
    QSortFilterProxyModel *mailTrash() const { return m_mailTrash; }
    CalendarModel *calendarModel() const { return m_calendar; }
    CalendarListModel *calendarListModel() const { return m_calendars; }
    ContactModel *contactModel() const { return m_contacts; }
    TaskModel *taskModel() const { return m_tasks; }
    SyncController *syncController() const { return m_sync; }
    OAuth2Manager *oauthManager() const { return m_oauth; }

    QString appName() const { return QStringLiteral("OmaSuite"); }
    QString appVersion() const { return QStringLiteral("0.1.0"); }

    Q_INVOKABLE QVariantList providers() const;
    Q_INVOKABLE void refreshAll();

private:
    AccountManager *m_accounts = nullptr;
    MailModel *m_mail = nullptr;
    MailFilterModel *m_mailInbox = nullptr;
    MailFilterModel *m_mailTrash = nullptr;
    CalendarModel *m_calendar = nullptr;
    CalendarListModel *m_calendars = nullptr;
    ContactModel *m_contacts = nullptr;
    TaskModel *m_tasks = nullptr;
    SyncController *m_sync = nullptr;
    OAuth2Manager *m_oauth = nullptr;
};

#endif // APPCORE_H
