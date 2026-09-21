#include "smtpclient.h"
#include "net/mimeutils.h"

#include <QDebug>

SmtpClient::SmtpClient(QObject *parent)
    : QObject(parent)
{
}

void SmtpClient::send(const QString &host, int port, const QString &user, const QString &pass,
                      const QString &from, const QString &to, const QString &subject,
                      const QString &body, const QString &cc)
{
    m_useOAuth2 = false;
    m_host = host;
    m_port = port;
    m_user = user;
    m_pass = pass;
    m_from = from;
    m_to = to;
    m_cc = cc;
    m_subject = subject;
    m_body = body;
    m_useAuth = !user.isEmpty() && !pass.isEmpty();
    m_step = Greeting;

    m_socket = new QSslSocket(this);
    connect(m_socket, &QSslSocket::readyRead, this, &SmtpClient::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &SmtpClient::onDisconnected);
    connect(m_socket, &QSslSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        if (m_step != Done)
            emit failed(m_socket->errorString());
    });
    connect(m_socket, &QSslSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
        if (m_allowUntrusted) {
            m_socket->ignoreSslErrors();
            return;
        }
        QStringList msgs;
        for (const QSslError &e : errors)
            msgs << e.errorString();
        emit failed(QStringLiteral("TLS verification failed: %1").arg(msgs.join(QStringLiteral("; "))));
        m_socket->abort();
    });

    m_socket->connectToHostEncrypted(host, static_cast<quint16>(port));
}

void SmtpClient::sendOAuth2(const QString &host, int port, const QString &user, const QString &accessToken,
                            const QString &from, const QString &to, const QString &subject,
                            const QString &body, const QString &cc)
{
    m_useOAuth2 = true;
    m_pass = accessToken;
    send(host, port, user, QString(), from, to, subject, body, cc);
}

void SmtpClient::writeLine(const QByteArray &line)
{
    m_socket->write(line + "\r\n");
}

void SmtpClient::onDisconnected()
{
    if (m_step == Quit || m_step == Done)
        return;
    emit failed(QStringLiteral("Connection closed unexpectedly"));
}

void SmtpClient::onReadyRead()
{
    m_buffer += m_socket->readAll();

    while (m_step != Done) {
        const int crlf = m_buffer.indexOf("\r\n");
        if (crlf < 0)
            return;
        const QByteArray line = m_buffer.left(crlf);
        m_buffer.remove(0, crlf + 2);
        if (line.isEmpty())
            continue;

        const int code = line.left(3).toInt();
        if (line.size() >= 4 && line.at(3) == '-')
            continue; // multiline continuation, ignore

        switch (m_step) {
        case Greeting:
            if (code >= 200 && code < 300) {
                m_step = Ehlo;
                writeLine(QByteArray("EHLO ") + "omasuite");
            } else {
                emit failed(QString::fromLatin1(line));
                m_step = Done;
            }
            break;

        case Ehlo:
            if (m_useAuth) {
                m_step = AuthUser;
                writeLine(m_useOAuth2 ? "AUTH XOAUTH2" : "AUTH LOGIN");
            } else {
                m_step = MailFrom;
                advance();
            }
            break;

        case AuthUser:
            if (m_useOAuth2) {
                m_step = AuthResult;
                const QByteArray ir = "user=" + m_user.toUtf8() + "\x01" + "auth=Bearer " + m_pass.toUtf8() + "\x01\x01";
                writeLine(ir.toBase64());
            } else {
                m_step = AuthPass;
                writeLine(m_user.toUtf8().toBase64());
            }
            break;

        case AuthPass:
            m_step = AuthResult;
            writeLine(m_pass.toUtf8().toBase64());
            break;

        case AuthResult:
            if (code >= 200 && code < 300) {
                m_step = MailFrom;
                advance();
            } else if (m_useOAuth2) {
                emit authenticationFailed();
                m_step = Done;
            } else {
                emit failed(QStringLiteral("SMTP authentication failed"));
                m_step = Done;
            }
            break;

        case MailFrom:
            m_step = RcptTo;
            advance();
            break;

        case RcptTo:
            m_step = Data;
            advance();
            break;

        case Data:
            if (code == 354) {
                m_step = Body;
                const QByteArray msg = MimeUtils::buildMessage(m_from, m_to, m_subject, m_body, m_cc);
                m_socket->write(msg);
                writeLine(".");
            } else {
                emit failed(QString::fromLatin1(line));
                m_step = Done;
            }
            break;

        case Body:
            if (code >= 200 && code < 300) {
                m_step = Quit;
                writeLine("QUIT");
            } else {
                emit failed(QString::fromLatin1(line));
                m_step = Done;
            }
            break;

        case Quit:
            m_step = Done;
            emit sent();
            m_socket->disconnectFromHost();
            break;

        default:
            break;
        }

        if (m_step == Done)
            break;
    }
}

void SmtpClient::advance()
{
    switch (m_step) {
    case MailFrom:
        writeLine("MAIL FROM:<" + m_from.toUtf8() + ">");
        break;
    case RcptTo:
        writeLine("RCPT TO:<" + m_to.toUtf8() + ">");
        break;
    case Data:
        writeLine("DATA");
        break;
    default:
        break;
    }
}
