#include "imapclient.h"
#include "net/mimeutils.h"

#include <QRegularExpression>
#include <QDateTime>
#include <QDebug>
#include <algorithm>

namespace {
const QByteArray LIT_START(1, 0x02);
const QByteArray LIT_END(1, 0x03);
}

ImapClient::ImapClient(QObject *parent)
    : QObject(parent)
{
}

void ImapClient::connectToServer(const QString &host, int port)
{
    m_error.clear();
    m_state = Connecting;

    m_socket = new QSslSocket(this);
    connect(m_socket, &QSslSocket::readyRead, this, &ImapClient::onReadyRead);
    connect(m_socket, &QSslSocket::connected, this, [this]() {
        m_state = Connected;
        emit connected();
    });
    connect(m_socket, &QSslSocket::encrypted, this, [this]() {
        // TLS handshake done; server greeting will arrive on readyRead.
    });
    connect(m_socket, &QSslSocket::disconnected, this, &ImapClient::onDisconnected);
    connect(m_socket, &QSslSocket::errorOccurred, this, [this](QAbstractSocket::SocketError e) {
        Q_UNUSED(e);
        m_error = m_socket->errorString();
        emit failed(m_error);
    });
    connect(m_socket, &QSslSocket::sslErrors, this, &ImapClient::onSslErrors);

    m_socket->connectToHostEncrypted(host, static_cast<quint16>(port));
}

void ImapClient::onSslErrors(const QList<QSslError> &errors)
{
    if (m_allowUntrusted) {
        m_socket->ignoreSslErrors();
        return;
    }
    QStringList msgs;
    for (const QSslError &e : errors)
        msgs << e.errorString();
    m_error = QStringLiteral("TLS verification failed: %1").arg(msgs.join(QStringLiteral("; ")));
    emit failed(m_error);
    m_socket->abort();
}

void ImapClient::onDisconnected()
{
    m_state = Disconnected;
}

void ImapClient::login(const QString &user, const QString &pass)
{
    m_user = user;
    m_pass = pass;
    sendCommand(QStringLiteral("LOGIN \"%1\" \"%2\"")
                    .arg(user, pass), [this](const QString &tag, const QString &status, const QString &text) {
        if (status == QLatin1String("OK")) {
            m_state = Authenticated;
            emit authenticated();
        } else {
            m_error = QStringLiteral("Login failed: %1").arg(text);
            emit failed(m_error);
        }
    });
}

void ImapClient::loginXoauth2(const QString &user, const QString &accessToken)
{
    m_user = user;
    const QByteArray ir = "user=" + user.toUtf8() + "\x01" + "auth=Bearer " + accessToken.toUtf8() + "\x01\x01";

    sendCommand(QStringLiteral("AUTHENTICATE XOAUTH2 %1").arg(QString::fromLatin1(ir.toBase64())),
                [this](const QString &, const QString &status, const QString &text) {
        if (status == QLatin1String("OK")) {
            m_state = Authenticated;
            emit authenticated();
        } else {
            m_error = QStringLiteral("XOAUTH2 failed: %1").arg(text);
            emit failed(m_error);
        }
    });
}

void ImapClient::listFolders()
{
    m_folderList.clear();
    sendCommand(QStringLiteral("LIST \"\" \"*\""), [this](const QString &, const QString &status, const QString &) {
        if (status == QLatin1String("OK"))
            emit foldersFetched(m_folderList);
    });
}

void ImapClient::selectFolder(const QString &folder)
{
    m_selectedFolder = folder;
    sendCommand(QStringLiteral("SELECT \"%1\"").arg(folder),
                [](const QString &, const QString &, const QString &) {});
}

void ImapClient::fetchFolder(const QString &folder, int limit)
{
    m_selectedFolder = folder;
    m_msgs.clear();
    m_msgParts.clear();
    m_fetchingBody = false;

    auto onSelected = [this, limit](const QString &, const QString &status, const QString &text) {
        if (status != QLatin1String("OK"))
            return;
        sendCommand(QStringLiteral("UID FETCH 1:* (UID FLAGS BODY.PEEK[HEADER.FIELDS (FROM TO SUBJECT DATE)])"),
                    [this, limit](const QString &, const QString &st, const QString &) {
            if (st != QLatin1String("OK"))
                return;
            completeFetch(limit);
        });
    };
    sendCommand(QStringLiteral("SELECT \"%1\"").arg(folder), onSelected);
}

void ImapClient::fetchBody(const QString &folder, int uid)
{
    m_selectedFolder = folder;
    m_fetchingBody = true;
    m_pendingBodyUid = uid;
    m_pendingBody.clear();

    auto onSelected = [this, uid](const QString &, const QString &status, const QString &) {
        if (status != QLatin1String("OK"))
            return;
        sendCommand(QStringLiteral("UID FETCH %1 (BODY.PEEK[TEXT])").arg(uid),
                    [this](const QString &, const QString &st, const QString &) {
            if (st != QLatin1String("OK"))
                return;
            emit bodyFetched(m_pendingBodyUid, m_pendingBody);
        });
    };
    sendCommand(QStringLiteral("SELECT \"%1\"").arg(folder), onSelected);
}

void ImapClient::logout()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState)
        sendCommand(QStringLiteral("LOGOUT"), [](const QString &, const QString &, const QString &) {});
}

void ImapClient::sendCommand(const QString &cmd,
                             std::function<void(const QString &, const QString &, const QString &)> handler)
{
    const QString tag = QStringLiteral("A%1").arg(m_nextTag++);
    m_commands.insert(tag, { handler });
    m_socket->write((tag + QLatin1Char(' ') + cmd + QStringLiteral("\r\n")).toUtf8());
}

// ---- Stream parsing ----

void ImapClient::onReadyRead()
{
    m_buffer += m_socket->readAll();

    static const QRegularExpression litRe(QStringLiteral("\\{(\\d+)\\}\\s*$"));

    while (true) {
        if (m_literalRemaining >= 0) {
            if (m_buffer.size() < m_literalRemaining)
                return;
            m_line.append(LIT_START);
            m_line.append(m_buffer.left(m_literalRemaining));
            m_line.append(LIT_END);
            m_buffer.remove(0, m_literalRemaining);
            m_literalRemaining = -1;
            continue;
        }

        const int crlf = m_buffer.indexOf("\r\n");
        if (crlf < 0)
            return;

        const QByteArray chunk = m_buffer.left(crlf);
        m_buffer.remove(0, crlf + 2);
        m_line.append(chunk);

        const QRegularExpressionMatch lm = litRe.match(QString::fromLatin1(m_line));
        if (lm.hasMatch()) {
            m_literalRemaining = lm.captured(1).toInt();
            continue;
        }

        QByteArray line = m_line;
        m_line.clear();
        handleLine(line);
    }
}

void ImapClient::handleLine(const QByteArray &line)
{
    if (line.startsWith('*'))
        handleUntagged(line);
    else if (line.startsWith('+')) {
        // SASL continuation (e.g. XOAUTH2 error challenge) -> cancel with empty response.
        m_socket->write("\r\n");
    }
    else if (line.size() > 0)
        handleTagged(line);
}

void ImapClient::handleTagged(const QByteArray &line)
{
    const int sp = line.indexOf(' ');
    if (sp < 0)
        return;
    const QString tag = QString::fromLatin1(line.left(sp));
    const QByteArray rest = line.mid(sp + 1).trimmed();

    QString status = QStringLiteral("OK");
    QString text;
    if (rest.startsWith("OK")) {
        status = QStringLiteral("OK");
        text = QString::fromUtf8(rest.mid(2).trimmed());
    } else if (rest.startsWith("NO")) {
        status = QStringLiteral("NO");
        text = QString::fromUtf8(rest.mid(2).trimmed());
    } else if (rest.startsWith("BAD")) {
        status = QStringLiteral("BAD");
        text = QString::fromUtf8(rest.mid(3).trimmed());
    }

    if (m_commands.contains(tag)) {
        Command c = m_commands.take(tag);
        if (c.handler)
            c.handler(tag, status, text);
    }
}

void ImapClient::handleUntagged(const QByteArray &line)
{
    const QByteArray rest = line.mid(1).trimmed();

    if (rest.startsWith("OK")) {
        // greeting or untagged OK
        return;
    }
    if (rest.startsWith("LIST")) {
        parseList(line);
        return;
    }
    if (rest.contains("FETCH")) {
        parseFetch(line);
        return;
    }
    // EXISTS, FLAGS, BYE etc. ignored for now.
}

void ImapClient::parseList(const QByteArray &line)
{
    // * LIST (\HasNoChildren) "/" "INBOX"
    static const QRegularExpression re(
        QStringLiteral("\\* LIST \\(([^)]*)\\)\\s+\"([^\"]*)\"\\s+\"?([^\"\\r\\n]*)\"?"));
    const QRegularExpressionMatch m = re.match(QString::fromLatin1(line));
    if (!m.hasMatch())
        return;

    QVariantMap folder;
    folder[QStringLiteral("flags")] = m.captured(1).trimmed();
    folder[QStringLiteral("delimiter")] = m.captured(2);
    folder[QStringLiteral("name")] = m.captured(3).trimmed();
    m_folderList.append(folder);
}

void ImapClient::parseFetch(const QByteArray &line)
{
    // * N FETCH (...)
    const int sp = line.indexOf(' ');
    if (sp < 0)
        return;
    int p = sp + 1;
    while (p < line.size() && line.at(p) == ' ')
        ++p;
    int end = p;
    while (end < line.size() && line.at(end) >= '0' && line.at(end) <= '9')
        ++end;
    if (end == p)
        return;
    const int seq = line.mid(p, end - p).toInt();

    MsgPart part;
    part.seq = seq;

    // UID
    static const QRegularExpression uidRe(QStringLiteral("UID (\\d+)"));
    QRegularExpressionMatch um = uidRe.match(QString::fromLatin1(line));
    if (um.hasMatch())
        part.uid = um.captured(1).toInt();

    // FLAGS
    static const QRegularExpression flagsRe(QStringLiteral("FLAGS \\(([^)]*)\\)"));
    QRegularExpressionMatch fm = flagsRe.match(QString::fromLatin1(line));
    if (fm.hasMatch()) {
        const QString flags = fm.captured(1);
        part.seen = flags.contains(QStringLiteral("\\Seen"));
        part.flagged = flags.contains(QStringLiteral("\\Flagged"));
    }

    // Extract literals (delimited by STX/ETX).
    QList<QByteArray> literals;
    int i = 0;
    while (i < line.size()) {
        if (line.at(i) == LIT_START.at(0)) {
            const int close = line.indexOf(LIT_END, i + 1);
            if (close < 0)
                break;
            literals.append(line.mid(i + 1, close - i - 1));
            i = close + 1;
        } else {
            ++i;
        }
    }

    // First literal is the header block (for fetchFolder), or the body (for fetchBody).
    if (m_fetchingBody) {
        if (!literals.isEmpty())
            m_pendingBody = QString::fromUtf8(literals.first());
        return;
    }

    if (!literals.isEmpty()) {
        const QByteArray header = literals.first();
        const QList<QByteArray> hlines = header.split('\n');
        for (const QByteArray &hl : hlines) {
            const QByteArray h = hl.trimmed();
            const int colon = h.indexOf(':');
            if (colon < 0)
                continue;
            const QByteArray name = h.left(colon).trimmed().toLower();
            const QByteArray rawValue = h.mid(colon + 1).trimmed();
            const QString value = MimeUtils::decodeHeader(rawValue);
            if (name == "from") part.from = MimeUtils::displaySender(value);
            else if (name == "to") part.to = MimeUtils::displaySender(value);
            else if (name == "subject") part.subject = value;
            else if (name == "date") {
                const QDateTime dt = QDateTime::fromString(value, Qt::RFC2822Date);
                if (dt.isValid()) {
                    part.timestampMs = dt.toMSecsSinceEpoch();
                    part.date = dt.toString(QStringLiteral("MMM d, h:mm AP"));
                } else {
                    part.date = value;
                }
            }
        }
    }

    m_msgs.insert(part.seq, part);
}

void ImapClient::completeFetch(int limit)
{
    QList<MsgPart> list = m_msgs.values();
    std::sort(list.begin(), list.end(), [](const MsgPart &a, const MsgPart &b) {
        return a.uid > b.uid;
    });

    QVariantList out;
    int n = 0;
    for (const MsgPart &p : list) {
        if (limit > 0 && n >= limit)
            break;
        QVariantMap m;
        m[QStringLiteral("uid")] = p.uid;
        m[QStringLiteral("seq")] = p.seq;
        m[QStringLiteral("from")] = p.from;
        m[QStringLiteral("to")] = p.to;
        m[QStringLiteral("subject")] = p.subject;
        m[QStringLiteral("date")] = p.date;
        m[QStringLiteral("timestamp")] = p.timestampMs;
        m[QStringLiteral("seen")] = p.seen;
        m[QStringLiteral("starred")] = p.flagged;
        m[QStringLiteral("preview")] = QString();
        out.append(m);
        ++n;
    }
    emit messagesFetched(out);
}
