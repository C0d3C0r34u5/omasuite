#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <QVariantList>

#include "account/accountmanager.h"
#include "mail/mailmodel.h"
#include "calendar/calendarmodel.h"
#include "contacts/contactmodel.h"
#include "tasks/taskmodel.h"
#include "net/synccontroller.h"
#include "net/oauth2manager.h"

class AppCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountManager *accounts READ accountManager CONSTANT)
    Q_PROPERTY(MailModel *mail READ mailModel CONSTANT)
    Q_PROPERTY(CalendarModel *calendar READ calendarModel CONSTANT)
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
    CalendarModel *calendarModel() const { return m_calendar; }
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
    CalendarModel *m_calendar = nullptr;
    ContactModel *m_contacts = nullptr;
    TaskModel *m_tasks = nullptr;
    SyncController *m_sync = nullptr;
    OAuth2Manager *m_oauth = nullptr;
};

#endif // APPCORE_H
