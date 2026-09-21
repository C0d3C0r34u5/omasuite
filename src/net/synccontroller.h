#ifndef SYNCCONTROLLER_H
#define SYNCCONTROLLER_H

#include <QObject>
#include <QPair>
#include <QList>
#include <functional>

class AccountManager;
class Account;
class MailModel;
class CalendarModel;
class CalendarListModel;
class ContactModel;
class TaskModel;
class ImapClient;
class SmtpClient;
class CaldavClient;
class CarddavClient;
class OAuth2Manager;
class EwsClient;

class SyncController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool syncing READ syncing NOTIFY syncingChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit SyncController(AccountManager *accounts, MailModel *mail, CalendarModel *calendar,
                            CalendarListModel *calendars, ContactModel *contacts, TaskModel *tasks,
                            OAuth2Manager *oauth, QObject *parent = nullptr);

    bool syncing() const { return m_syncing; }
    QString status() const { return m_status; }

    Q_INVOKABLE void syncMail(int accountId);
    Q_INVOKABLE void syncCalendar(int accountId);
    Q_INVOKABLE void syncCalendars(int accountId);
    Q_INVOKABLE void syncContacts(int accountId);
    Q_INVOKABLE void syncTasks(int accountId);
    Q_INVOKABLE void syncAll();

    Q_INVOKABLE void sendMail(int accountId, const QString &to, const QString &subject,
                              const QString &body, const QString &cc = QString());
    Q_INVOKABLE void fetchMailBody(int accountId, int uid);

    Q_INVOKABLE int createEvent(int accountId, int calendarId, const QString &title, const QString &description,
                                const QString &location, qint64 start, qint64 end, bool allDay);
    Q_INVOKABLE void deleteEvent(int accountId, int calendarId, int localId, const QString &uid);

    Q_INVOKABLE int createContact(int accountId, const QString &firstName, const QString &lastName,
                                  const QString &email, const QString &phone, const QString &organization);
    Q_INVOKABLE void deleteContact(int accountId, int localId, const QString &uid);

    Q_INVOKABLE int createTask(int accountId, const QString &title, const QString &description, qint64 due);
    Q_INVOKABLE void deleteTask(int accountId, int localId, const QString &uid);

signals:
    void syncingChanged();
    void statusChanged();
    void mailBodyFetched(int uid, const QString &body);
    void accountAuthorized(const QString &email);

private:
    void setSyncing(bool syncing, const QString &status);
    Account *account(int id) const;
    bool ensureToken(Account *a, std::function<void()> retry);
    void requestTokenRefresh(Account *a, std::function<void()> retry);
    void fetchNextCalendarEvents();
    bool isExchange(Account *a) const;
    void setupEws(Account *a);

    AccountManager *m_accounts = nullptr;
    MailModel *m_mail = nullptr;
    CalendarModel *m_calendar = nullptr;
    CalendarListModel *m_calendars = nullptr;
    ContactModel *m_contacts = nullptr;
    TaskModel *m_tasks = nullptr;
    OAuth2Manager *m_oauth = nullptr;

    ImapClient *m_imap = nullptr;
    SmtpClient *m_smtp = nullptr;
    CaldavClient *m_caldav = nullptr;
    CarddavClient *m_carddav = nullptr;
    EwsClient *m_ews = nullptr;

    bool m_syncing = false;
    QString m_status;

    int m_syncAccountId = 0;
    QString m_syncEmail;
    QString m_syncPass;
    std::function<void()> m_retryAfterAuth;

    // Calendar sync state: pending per-calendar event fetches.
    int m_syncCalendarAccountId = 0;
    QList<QPair<int, QString>> m_calendarFetchQueue; // (calendarId, href)
    int m_currentFetchCalendarId = 0;
    int m_syncCalendarAdded = 0;
};

#endif // SYNCCONTROLLER_H
