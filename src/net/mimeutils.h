#ifndef MIMEUTILS_H
#define MIMEUTILS_H

#include <QByteArray>
#include <QString>

namespace MimeUtils {

// Decode RFC 2047 encoded-words in a mail header value (e.g. "=?UTF-8?B?...?=").
QString decodeHeader(const QByteArray &value);

// Decode quoted-printable content.
QByteArray decodeQuotedPrintable(const QByteArray &data);

// Extract a plain email address from a header field like "Name <user@host>" or "user@host".
QString extractAddress(const QString &value);

// Extract the display name (without address) from a header field.
QString extractName(const QString &value);

// Format a sender string for display: "Name" or "user@host".
QString displaySender(const QString &value);

// Build a MIME message (RFC 5322) from components. Returns full message bytes with CRLF.
QByteArray buildMessage(const QString &from, const QString &to, const QString &subject,
                        const QString &body, const QString &cc = QString());

}

#endif // MIMEUTILS_H
