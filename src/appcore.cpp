#include "appcore.h"

#include "account/accountmanager.h"
#include "account/provider.h"
#include "mail/mailmodel.h"
#include "calendar/calendarmodel.h"
#include "contacts/contactmodel.h"
#include "tasks/taskmodel.h"

AppCore::AppCore(QObject *parent)
    : QObject(parent)
{
    m_accounts = new AccountManager(this);
    m_mail = new MailModel(this);
    m_calendar = new CalendarModel(this);
    m_contacts = new ContactModel(this);
    m_tasks = new TaskModel(this);
    m_oauth = new OAuth2Manager(this);
    m_sync = new SyncController(m_accounts, m_mail, m_calendar, m_contacts, m_tasks, m_oauth, this);
}

QVariantList AppCore::providers() const
{
    return Provider::presets();
}

void AppCore::refreshAll()
{
    m_mail->reload();
    m_calendar->reload();
    m_contacts->reload();
    m_tasks->reload();
}
