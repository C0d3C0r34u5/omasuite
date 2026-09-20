#include "mimeutils.h"

#include <QRegularExpression>
#include <QDateTime>

namespace MimeUtils {

QString decodeHeader(const QByteArray &value)
{
    QString text = QString::fromLatin1(value);

    // Regex for RFC 2047 encoded-words: =?charset?encoding?data?=
    static const QRegularExpression re(
        QStringLiteral("=\\?([^?]+)\\?([bBqQ])\\?([^?]*)\\?="));
    int offset = 0;
    QRegularExpressionMatch m = re.match(text, offset);
    QString result;
    int last = 0;
    while (m.hasMatch()) {
        result += text.mid(last, m.capturedStart() - last);
        const QString charset = m.captured(1);
        const QString enc = m.captured(2).toUpper();
        const QByteArray data = m.captured(3).toLatin1();
        QByteArray decoded;
        if (enc == QLatin1String("B")) {
            decoded = QByteArray::fromBase64(data);
        } else { // Q
            decoded = data;
            decoded.replace('_', ' ');
            decoded = QByteArray::fromPercentEncoding(decoded);
        }
        const QString cs = charset.toLower();
        if (cs.contains(QStringLiteral("utf-8")) || cs.contains(QStringLiteral("utf8")))
            result += QString::fromUtf8(decoded);
        else if (cs.contains(QStringLiteral("latin")) || cs.contains(QStringLiteral("iso-8859")))
            result += QString::fromLatin1(decoded);
        else
            result += QString::fromUtf8(decoded);
        last = m.capturedEnd();
        offset = last;
        m = re.match(text, offset);
    }
    result += text.mid(last);
    return result;
}

QByteArray decodeQuotedPrintable(const QByteArray &data)
{
    QByteArray out;
    out.reserve(data.size());
    for (int i = 0; i < data.size(); ++i) {
        const char c = data.at(i);
        if (c == '=') {
            if (i + 2 < data.size()) {
                const QByteArray hex = data.mid(i + 1, 2);
                bool ok = false;
                const int v = hex.toInt(&ok, 16);
                if (ok) {
                    out.append(static_cast<char>(v));
                    i += 2;
                    continue;
                }
            }
            if (i + 1 < data.size() && (data.at(i + 1) == '\r' || data.at(i + 1) == '\n')) {
                // soft line break
                if (i + 2 < data.size() && data.at(i + 1) == '\r' && data.at(i + 2) == '\n')
                    i += 2;
                else
                    i += 1;
                continue;
            }
            out.append(c);
        } else {
            out.append(c);
        }
    }
    return out;
}

QString extractAddress(const QString &value)
{
    static const QRegularExpression re(QStringLiteral("<([^>]*)>"));
    QRegularExpressionMatch m = re.match(value);
    if (m.hasMatch())
        return m.captured(1).trimmed();
    return value.trimmed();
}

QString extractName(const QString &value)
{
    const int lt = value.indexOf(QLatin1Char('<'));
    if (lt >= 0) {
        QString name = value.left(lt).trimmed();
        if (name.startsWith(QLatin1Char('"')) && name.endsWith(QLatin1Char('"')))
            name = name.mid(1, name.length() - 2);
        return name;
    }
    return QString();
}

QString displaySender(const QString &value)
{
    const QString name = extractName(value);
    if (!name.isEmpty())
        return name;
    const QString addr = extractAddress(value);
    return addr.isEmpty() ? value.trimmed() : addr;
}

QByteArray buildMessage(const QString &from, const QString &to, const QString &subject,
                        const QString &body, const QString &cc)
{
    QByteArray msg;
    msg += "From: " + from.toUtf8() + "\r\n";
    msg += "To: " + to.toUtf8() + "\r\n";
    if (!cc.isEmpty())
        msg += "Cc: " + cc.toUtf8() + "\r\n";
    msg += "Subject: " + subject.toUtf8() + "\r\n";
    msg += "Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8() + "\r\n";
    msg += "MIME-Version: 1.0\r\n";
    msg += "Content-Type: text/plain; charset=utf-8\r\n";
    msg += "Content-Transfer-Encoding: 8bit\r\n";
    msg += "\r\n";
    msg += body.toUtf8() + "\r\n";
    return msg;
}

}
