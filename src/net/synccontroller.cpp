#include "synccontroller.h"

#include "account/account.h"
#include "account/accountmanager.h"
#include "mail/mailmodel.h"
#include "mail/imapclient.h"
#include "mail/smtpclient.h"
#include "calendar/calendarmodel.h"
#include "calendar/caldavclient.h"
#include "contacts/contactmodel.h"
#include "contacts/carddavclient.h"
#include "tasks/taskmodel.h"

#include "net/oauth2manager.h"
#include "net/ewsclient.h"

#include <QUuid>
#include <QDateTime>
#include <QDebug>

SyncController::SyncController(AccountManager *accounts, MailModel *mail, CalendarModel *calendar,
                               ContactModel *contacts, TaskModel *tasks, OAuth2Manager *oauth,
                               QObject *parent)
    : QObject(parent)
    , m_accounts(accounts)
    , m_mail(mail)
    , m_calendar(calendar)
    , m_contacts(contacts)
    , m_tasks(tasks)
    , m_oauth(oauth)
{
    m_imap = new ImapClient(this);
    m_smtp = new SmtpClient(this);
    m_caldav = new CaldavClient(this);
    m_carddav = new CarddavClient(this);
    m_ews = new EwsClient(this);

    connect(m_oauth, &OAuth2Manager::authorized, this, [this](const QString &email) {
        emit accountAuthorized(email);
        if (m_retryAfterAuth) {
            auto f = m_retryAfterAuth;
            m_retryAfterAuth = nullptr;
            f();
        }
    });
    connect(m_oauth, &OAuth2Manager::tokenRefreshed, this, [this](const QString &) {
        if (m_retryAfterAuth) {
            auto f = m_retryAfterAuth;
            m_retryAfterAuth = nullptr;
            f();
        }
    });
    connect(m_oauth, &OAuth2Manager::failed, this, [this](const QString &, const QString &reason) {
        setSyncing(false, reason);
    });
}

bool SyncController::ensureToken(Account *a, std::function<void()> retry)
{
    if (a->authMethod() != QStringLiteral("oauth2"))
        return true;

    const QString token = m_oauth->accessToken(a->email());
    if (!token.isEmpty())
        return true;

    m_retryAfterAuth = retry;
    if (m_oauth->hasTokens(a->email())) {
        setSyncing(true, QStringLiteral("Refreshing token..."));
        m_oauth->refresh(a->provider(), a->email(), a->oauthClientId());
    } else {
        setSyncing(true, QStringLiteral("Sign in required - check your browser"));
        m_oauth->authorize(a->provider(), a->email(), a->oauthClientId());
    }
    return false;
}

void SyncController::setSyncing(bool syncing, const QString &status)
{
    if (m_syncing != syncing) {
        m_syncing = syncing;
        emit syncingChanged();
    }
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

Account *SyncController::account(int id) const
{
    return m_accounts->accountById(id);
}

bool SyncController::isExchange(Account *a) const
{
    return a && a->provider() == QStringLiteral("exchange");
}

void SyncController::setupEws(Account *a)
{
    m_ews->setEndpoint(a->ewsUrl());
    m_ews->setAllowUntrusted(a->allowUntrusted());
    if (a->authMethod() == QStringLiteral("oauth2"))
        m_ews->setBearerToken(m_oauth->accessToken(a->email()));
    else
        m_ews->setBasicAuth(a->email(), a->password());
}

void SyncController::syncMail(int accountId)
{
    Account *a = account(accountId);
    if (!a)
        return;

    if (isExchange(a)) {
        if (!ensureToken(a, [this, accountId]() { syncMail(accountId); }))
            return;
        setupEws(a);
        setSyncing(true, QStringLiteral("Syncing mail..."));
        m_ews->disconnect(this);
        connect(m_ews, &EwsClient::messagesFetched, this, [this, accountId](const QVariantList &msgs) {
            int added = 0;
            for (const QVariant &v : msgs) {
                const QVariantMap m = v.toMap();
                const int id = m_mail->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                                 m.value(QStringLiteral("subject")).toString(),
                                                 m.value(QStringLiteral("from")).toString(),
                                                 QString(),
                                                 m.value(QStringLiteral("body")).toString(),
                                                 m.value(QStringLiteral("timestamp")).toLongLong(),
                                                 m.value(QStringLiteral("seen")).toBool(),
                                                 false);
                if (id > 0)
                    ++added;
            }
            setSyncing(false, QStringLiteral("Mail synced (%1 new)").arg(added));
        });
        connect(m_ews, &EwsClient::failed, this, [this](const QString &r) { setSyncing(false, r); });
        m_ews->fetchMail(QStringLiteral("inbox"), 200);
        return;
    }

    if (!ensureToken(a, [this, accountId]() { syncMail(accountId); }))
        return;

    m_syncAccountId = accountId;
    m_syncEmail = a->email();
    m_syncPass = a->password();
    const bool oauth = a->authMethod() == QStringLiteral("oauth2");
    const QString token = oauth ? m_oauth->accessToken(a->email()) : QString();

    m_imap->setAllowUntrusted(a->allowUntrusted());
    setSyncing(true, QStringLiteral("Connecting to %1...").arg(a->imapHost()));

    m_imap->disconnect(this);
    connect(m_imap, &ImapClient::connected, this, [this, oauth, token]() {
        setSyncing(true, QStringLiteral("Signing in..."));
        if (oauth)
            m_imap->loginXoauth2(m_syncEmail, token);
        else
            m_imap->login(m_syncEmail, m_syncPass);
    });
    connect(m_imap, &ImapClient::authenticated, this, [this]() {
        setSyncing(true, QStringLiteral("Downloading messages..."));
        m_imap->fetchFolder(QStringLiteral("INBOX"), 200);
    });
    connect(m_imap, &ImapClient::messagesFetched, this, [this](const QVariantList &msgs) {
        int added = 0;
        for (const QVariant &v : msgs) {
            const QVariantMap m = v.toMap();
            const int id = m_mail->addSynced(m_syncAccountId, m.value(QStringLiteral("uid")).toString(),
                                             m.value(QStringLiteral("subject")).toString(),
                                             m.value(QStringLiteral("from")).toString(),
                                             m.value(QStringLiteral("to")).toString(),
                                             QString(),
                                             m.value(QStringLiteral("timestamp")).toLongLong(),
                                             m.value(QStringLiteral("seen")).toBool(),
                                             m.value(QStringLiteral("starred")).toBool());
            if (id > 0)
                ++added;
        }
        setSyncing(false, QStringLiteral("Mail synced (%1 new)").arg(added));
    });
    connect(m_imap, &ImapClient::failed, this, [this](const QString &r) {
        setSyncing(false, r);
    });

    m_imap->connectToServer(a->imapHost(), a->imapPort());
}

void SyncController::fetchMailBody(int accountId, int uid)
{
    Account *a = account(accountId);
    if (!a)
        return;

    if (!ensureToken(a, [this, accountId, uid]() { fetchMailBody(accountId, uid); }))
        return;

    const bool oauth = a->authMethod() == QStringLiteral("oauth2");
    const QString token = oauth ? m_oauth->accessToken(a->email()) : QString();
    m_syncEmail = a->email();
    m_syncPass = a->password();

    m_imap->disconnect(this);
    connect(m_imap, &ImapClient::connected, this, [this, oauth, token, uid]() {
        if (oauth)
            m_imap->loginXoauth2(m_syncEmail, token);
        else
            m_imap->login(m_syncEmail, m_syncPass);
    });
    connect(m_imap, &ImapClient::authenticated, this, [this, uid]() {
        m_imap->fetchBody(QStringLiteral("INBOX"), uid);
    });
    connect(m_imap, &ImapClient::bodyFetched, this, [this](int uid, const QString &body) {
        m_mail->setBodyByUid(QString::number(uid), body);
        emit mailBodyFetched(uid, body);
    });
    m_imap->connectToServer(a->imapHost(), a->imapPort());
}

void SyncController::sendMail(int accountId, const QString &to, const QString &subject,
                              const QString &body, const QString &cc)
{
    Account *a = account(accountId);
    if (!a)
        return;

    if (isExchange(a)) {
        if (!ensureToken(a, [this, accountId, to, subject, body, cc]() {
                sendMail(accountId, to, subject, body, cc);
            }))
            return;
        setupEws(a);
        setSyncing(true, QStringLiteral("Sending message..."));
        m_ews->disconnect(this);
        connect(m_ews, &EwsClient::sent, this, [this, accountId, to, subject, body]() {
            m_mail->add(accountId, subject, QStringLiteral("Me"), to, body);
            setSyncing(false, QStringLiteral("Message sent"));
        });
        connect(m_ews, &EwsClient::failed, this, [this](const QString &r) { setSyncing(false, r); });
        m_ews->sendMail(a->email(), to, subject, body);
        return;
    }

    if (!ensureToken(a, [this, accountId, to, subject, body, cc]() {
            sendMail(accountId, to, subject, body, cc);
        }))
        return;

    setSyncing(true, QStringLiteral("Sending message..."));

    connect(m_smtp, &SmtpClient::sent, this, [this, accountId, to, subject, body]() {
        m_mail->add(accountId, subject, QStringLiteral("Me"), to, body);
        setSyncing(false, QStringLiteral("Message sent"));
    });
    connect(m_smtp, &SmtpClient::failed, this, [this](const QString &r) {
        setSyncing(false, r);
    });

    m_smtp->setAllowUntrusted(a->allowUntrusted());
    if (a->authMethod() == QStringLiteral("oauth2")) {
        m_smtp->sendOAuth2(a->smtpHost(), a->smtpPort(), a->email(),
                           m_oauth->accessToken(a->email()),
                           a->email(), to, subject, body, cc);
    } else {
        m_smtp->send(a->smtpHost(), a->smtpPort(), a->email(), a->password(),
                     a->email(), to, subject, body, cc);
    }
}

void SyncController::syncCalendar(int accountId)
{
    Account *a = account(accountId);
    if (!a)
        return;

    if (isExchange(a)) {
        if (!ensureToken(a, [this, accountId]() { syncCalendar(accountId); }))
            return;
        setupEws(a);
        setSyncing(true, QStringLiteral("Syncing calendar..."));
        m_ews->disconnect(this);
        connect(m_ews, &EwsClient::eventsFetched, this, [this, accountId](const QVariantList &events) {
            int added = 0;
            for (const QVariant &v : events) {
                const QVariantMap m = v.toMap();
                const int id = m_calendar->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                                     m.value(QStringLiteral("subject")).toString(),
                                                     QString(),
                                                     m.value(QStringLiteral("location")).toString(),
                                                     m.value(QStringLiteral("start")).toLongLong(),
                                                     m.value(QStringLiteral("end")).toLongLong(),
                                                     m.value(QStringLiteral("allDay")).toBool());
                if (id > 0)
                    ++added;
            }
            setSyncing(false, QStringLiteral("Calendar synced (%1 new)").arg(added));
        });
        connect(m_ews, &EwsClient::failed, this, [this](const QString &r) { setSyncing(false, r); });
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        m_ews->fetchCalendar(now - 30LL * 24 * 3600 * 1000, now + 365LL * 24 * 3600 * 1000);
        return;
    }

    if (!a->caldavUrl().isEmpty()) {
        if (!ensureToken(a, [this, accountId]() { syncCalendar(accountId); }))
            return;
    }

    if (a->caldavUrl().isEmpty())
        return;

    setSyncing(true, QStringLiteral("Syncing calendar..."));
    if (a->authMethod() == QStringLiteral("oauth2"))
        m_caldav->setBearerToken(m_oauth->accessToken(a->email()));
    else
        m_caldav->setBearerToken(QString());

    m_caldav->disconnect(this);
    connect(m_caldav, &CaldavClient::eventsFetched, this, [this, accountId](const QVariantList &events) {
        int added = 0;
        for (const QVariant &v : events) {
            const QVariantMap m = v.toMap();
            const int id = m_calendar->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                                 m.value(QStringLiteral("title")).toString(),
                                                 m.value(QStringLiteral("description")).toString(),
                                                 m.value(QStringLiteral("location")).toString(),
                                                 m.value(QStringLiteral("start")).toLongLong(),
                                                 m.value(QStringLiteral("end")).toLongLong(),
                                                 m.value(QStringLiteral("allDay")).toBool());
            if (id > 0)
                ++added;
        }
        setSyncing(false, QStringLiteral("Calendar synced (%1 new)").arg(added));
    });
    connect(m_caldav, &CaldavClient::failed, this, [this](const QString &r) {
        setSyncing(false, r);
    });

    m_caldav->fetchEvents(a->caldavUrl(), a->email(), a->password());
}

void SyncController::syncContacts(int accountId)
{
    Account *a = account(accountId);
    if (!a)
        return;

    if (isExchange(a)) {
        if (!ensureToken(a, [this, accountId]() { syncContacts(accountId); }))
            return;
        setupEws(a);
        setSyncing(true, QStringLiteral("Syncing contacts..."));
        m_ews->disconnect(this);
        connect(m_ews, &EwsClient::contactsFetched, this, [this, accountId](const QVariantList &contacts) {
            int added = 0;
            for (const QVariant &v : contacts) {
                const QVariantMap m = v.toMap();
                const QString display = m.value(QStringLiteral("displayName")).toString();
                QString first = display, last;
                const int sp = display.lastIndexOf(QLatin1Char(' '));
                if (sp > 0) {
                    first = display.left(sp);
                    last = display.mid(sp + 1);
                }
                const int id = m_contacts->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                                     first, last,
                                                     m.value(QStringLiteral("email")).toString(),
                                                     m.value(QStringLiteral("phone")).toString(),
                                                     m.value(QStringLiteral("organization")).toString());
                if (id > 0)
                    ++added;
            }
            setSyncing(false, QStringLiteral("Contacts synced (%1 new)").arg(added));
        });
        connect(m_ews, &EwsClient::failed, this, [this](const QString &r) { setSyncing(false, r); });
        m_ews->fetchContacts();
        return;
    }

    if (!a->carddavUrl().isEmpty()) {
        if (!ensureToken(a, [this, accountId]() { syncContacts(accountId); }))
            return;
    }

    if (a->carddavUrl().isEmpty())
        return;

    setSyncing(true, QStringLiteral("Syncing contacts..."));
    if (a->authMethod() == QStringLiteral("oauth2"))
        m_carddav->setBearerToken(m_oauth->accessToken(a->email()));
    else
        m_carddav->setBearerToken(QString());

    m_carddav->disconnect(this);
    connect(m_carddav, &CarddavClient::contactsFetched, this, [this, accountId](const QVariantList &contacts) {
        int added = 0;
        for (const QVariant &v : contacts) {
            const QVariantMap m = v.toMap();
            const int id = m_contacts->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                                 m.value(QStringLiteral("firstName")).toString(),
                                                 m.value(QStringLiteral("lastName")).toString(),
                                                 m.value(QStringLiteral("email")).toString(),
                                                 m.value(QStringLiteral("phone")).toString(),
                                                 m.value(QStringLiteral("organization")).toString());
            if (id > 0)
                ++added;
        }
        setSyncing(false, QStringLiteral("Contacts synced (%1 new)").arg(added));
    });
    connect(m_carddav, &CarddavClient::failed, this, [this](const QString &r) {
        setSyncing(false, r);
    });

    m_carddav->fetchContacts(a->carddavUrl(), a->email(), a->password());
}

void SyncController::syncTasks(int accountId)
{
    Account *a = account(accountId);
    if (!a)
        return;

    if (isExchange(a)) {
        if (!ensureToken(a, [this, accountId]() { syncTasks(accountId); }))
            return;
        setupEws(a);
        setSyncing(true, QStringLiteral("Syncing tasks..."));
        m_ews->disconnect(this);
        connect(m_ews, &EwsClient::tasksFetched, this, [this, accountId](const QVariantList &tasks) {
            int added = 0;
            for (const QVariant &v : tasks) {
                const QVariantMap m = v.toMap();
                const bool completed = m.value(QStringLiteral("status")).toString() == QStringLiteral("Completed");
                const int id = m_tasks->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                                  m.value(QStringLiteral("subject")).toString(),
                                                  QString(),
                                                  m.value(QStringLiteral("due")).toLongLong(),
                                                  completed);
                if (id > 0)
                    ++added;
            }
            setSyncing(false, QStringLiteral("Tasks synced (%1 new)").arg(added));
        });
        connect(m_ews, &EwsClient::failed, this, [this](const QString &r) { setSyncing(false, r); });
        m_ews->fetchTasks();
        return;
    }

    if (!a->caldavUrl().isEmpty()) {
        if (!ensureToken(a, [this, accountId]() { syncTasks(accountId); }))
            return;
    }

    if (a->caldavUrl().isEmpty())
        return;

    setSyncing(true, QStringLiteral("Syncing tasks..."));
    if (a->authMethod() == QStringLiteral("oauth2"))
        m_caldav->setBearerToken(m_oauth->accessToken(a->email()));
    else
        m_caldav->setBearerToken(QString());

    m_caldav->disconnect(this);
    connect(m_caldav, &CaldavClient::todosFetched, this, [this, accountId](const QVariantList &todos) {
        int added = 0;
        for (const QVariant &v : todos) {
            const QVariantMap m = v.toMap();
            const int id = m_tasks->addSynced(accountId, m.value(QStringLiteral("uid")).toString(),
                                              m.value(QStringLiteral("title")).toString(),
                                              m.value(QStringLiteral("description")).toString(),
                                              m.value(QStringLiteral("due")).toLongLong(),
                                              m.value(QStringLiteral("completed")).toBool());
            if (id > 0)
                ++added;
        }
        setSyncing(false, QStringLiteral("Tasks synced (%1 new)").arg(added));
    });
    connect(m_caldav, &CaldavClient::failed, this, [this](const QString &r) {
        setSyncing(false, r);
    });

    m_caldav->fetchEvents(a->caldavUrl(), a->email(), a->password());
}

void SyncController::syncAll()
{
    Account *first = m_accounts->firstAccount();
    if (!first)
        return;
    const int id = first->id();
    syncMail(id);
    syncContacts(id);
    syncCalendar(id);
    syncTasks(id);
}

int SyncController::createEvent(int accountId, const QString &title, const QString &description,
                                const QString &location, qint64 start, qint64 end, bool allDay)
{
    const QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const int localId = m_calendar->addSynced(accountId, uid, title, description, location, start, end, allDay);

    Account *a = account(accountId);
    if (a && !a->caldavUrl().isEmpty()) {
        QVariantMap ev;
        ev[QStringLiteral("uid")] = uid;
        ev[QStringLiteral("title")] = title;
        ev[QStringLiteral("description")] = description;
        ev[QStringLiteral("location")] = location;
        ev[QStringLiteral("start")] = start;
        ev[QStringLiteral("end")] = end;
        ev[QStringLiteral("allDay")] = allDay;
        m_caldav->createEvent(a->caldavUrl(), a->email(), a->password(), ev);
    }
    return localId;
}

void SyncController::deleteEvent(int accountId, int localId, const QString &uid)
{
    m_calendar->removeEvent(localId);
    Account *a = account(accountId);
    if (a && !uid.isEmpty() && !a->caldavUrl().isEmpty())
        m_caldav->deleteEvent(a->caldavUrl(), a->email(), a->password(), uid);
}

int SyncController::createContact(int accountId, const QString &firstName, const QString &lastName,
                                  const QString &email, const QString &phone, const QString &organization)
{
    const QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const int localId = m_contacts->addSynced(accountId, uid, firstName, lastName, email, phone, organization);

    Account *a = account(accountId);
    if (a && !a->carddavUrl().isEmpty()) {
        QVariantMap c;
        c[QStringLiteral("uid")] = uid;
        c[QStringLiteral("firstName")] = firstName;
        c[QStringLiteral("lastName")] = lastName;
        c[QStringLiteral("email")] = email;
        c[QStringLiteral("phone")] = phone;
        c[QStringLiteral("organization")] = organization;
        m_carddav->createContact(a->carddavUrl(), a->email(), a->password(), c);
    }
    return localId;
}

void SyncController::deleteContact(int accountId, int localId, const QString &uid)
{
    m_contacts->removeContact(localId);
    Account *a = account(accountId);
    if (a && !uid.isEmpty() && !a->carddavUrl().isEmpty())
        m_carddav->deleteContact(a->carddavUrl(), a->email(), a->password(), uid);
}

int SyncController::createTask(int accountId, const QString &title, const QString &description, qint64 due)
{
    const QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const int localId = m_tasks->addSynced(accountId, uid, title, description, due, false);

    Account *a = account(accountId);
    if (a && !a->caldavUrl().isEmpty()) {
        QVariantMap t;
        t[QStringLiteral("uid")] = uid;
        t[QStringLiteral("title")] = title;
        t[QStringLiteral("description")] = description;
        t[QStringLiteral("due")] = due;
        t[QStringLiteral("completed")] = false;
        m_caldav->createTodo(a->caldavUrl(), a->email(), a->password(), t);
    }
    return localId;
}

void SyncController::deleteTask(int accountId, int localId, const QString &uid)
{
    m_tasks->removeTask(localId);
    Account *a = account(accountId);
    if (a && !uid.isEmpty() && !a->caldavUrl().isEmpty())
        m_caldav->deleteTodo(a->caldavUrl(), a->email(), a->password(), uid);
}
