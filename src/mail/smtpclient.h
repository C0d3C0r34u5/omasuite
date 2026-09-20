#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QSslSocket>

class SmtpClient : public QObject
{
    Q_OBJECT
public:
    explicit SmtpClient(QObject *parent = nullptr);

    void send(const QString &host, int port, const QString &user, const QString &pass,
              const QString &from, const QString &to, const QString &subject, const QString &body,
              const QString &cc = QString());
    void sendOAuth2(const QString &host, int port, const QString &user, const QString &accessToken,
                    const QString &from, const QString &to, const QString &subject,
                    const QString &body, const QString &cc = QString());
    void setAllowUntrusted(bool allow) { m_allowUntrusted = allow; }

signals:
    void sent();
    void failed(const QString &reason);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    enum Step { Greeting, Ehlo, AuthUser, AuthPass, AuthResult, MailFrom, RcptTo, Data, Body, Quit, Done };
    void advance();
    void writeLine(const QByteArray &line);

    QSslSocket *m_socket = nullptr;
    QByteArray m_buffer;
    Step m_step = Greeting;

    QString m_host;
    int m_port = 465;
    QString m_user;
    QString m_pass;
    QString m_from;
    QString m_to;
    QString m_cc;
    QString m_subject;
    QString m_body;
    bool m_useAuth = false;
    bool m_allowUntrusted = false;
    bool m_useOAuth2 = false;
};

#endif // SMTPCLIENT_H
