#include "vcardutils.h"

#include <QUuid>

namespace VCardUtils {

static QString unfold(const QString &text)
{
    QString out = text;
    out.replace(QStringLiteral("\r\n "), QString());
    out.replace(QStringLiteral("\r\n\t"), QString());
    out.replace(QStringLiteral("\n "), QString());
    out.replace(QStringLiteral("\n\t"), QString());
    return out;
}

QVariantList parseVCards(const QString &vcf)
{
    QVariantList out;
    const QString text = unfold(vcf);

    int pos = 0;
    while ((pos = text.indexOf(QStringLiteral("BEGIN:VCARD"), pos, Qt::CaseInsensitive)) >= 0) {
        const int stop = text.indexOf(QStringLiteral("END:VCARD"), pos, Qt::CaseInsensitive);
        if (stop < 0)
            break;
        const QString block = text.mid(pos, stop - pos);
        pos = stop;

        QVariantMap m;
        const QStringList lines = block.split(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
        for (const QString &raw : lines) {
            const QString line = raw.trimmed();
            int idx = line.indexOf(QLatin1Char(':'));
            if (idx < 0)
                continue;
            QString name = line.left(idx);
            const int semi = name.indexOf(QLatin1Char(';'));
            if (semi >= 0)
                name = name.left(semi);
            name = name.toUpper();
            QString value = line.mid(idx + 1);
            value.replace(QStringLiteral("\\,"), QStringLiteral(","));
            value.replace(QStringLiteral("\\;"), QStringLiteral(";"));
            value.replace(QStringLiteral("\\n"), QStringLiteral("\n"));

            if (name == QLatin1String("FN")) m[QStringLiteral("fullName")] = value;
            else if (name == QLatin1String("UID")) m[QStringLiteral("uid")] = value;
            else if (name == QLatin1String("EMAIL")) m[QStringLiteral("email")] = value;
            else if (name == QLatin1String("TEL")) m[QStringLiteral("phone")] = value;
            else if (name == QLatin1String("ORG")) m[QStringLiteral("organization")] = value;
            else if (name == QLatin1String("NOTE")) m[QStringLiteral("notes")] = value;
            else if (name == QLatin1String("N")) {
                // N:Last;First;Middle;Prefix;Suffix
                const QStringList parts = value.split(QLatin1Char(';'));
                if (parts.size() > 0) m[QStringLiteral("lastName")] = parts.at(0);
                if (parts.size() > 1) m[QStringLiteral("firstName")] = parts.at(1);
            }
        }

        if (!m.contains(QStringLiteral("firstName")) && m.contains(QStringLiteral("fullName"))) {
            const QString fn = m.value(QStringLiteral("fullName")).toString();
            const int sp = fn.lastIndexOf(QLatin1Char(' '));
            if (sp > 0) {
                m[QStringLiteral("firstName")] = fn.left(sp);
                m[QStringLiteral("lastName")] = fn.mid(sp + 1);
            } else {
                m[QStringLiteral("firstName")] = fn;
            }
        }

        out.append(m);
    }
    return out;
}

QString buildVCard(const QVariantMap &c)
{
    QString uid = c.value(QStringLiteral("uid")).toString();
    if (uid.isEmpty())
        uid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    const QString first = c.value(QStringLiteral("firstName")).toString();
    const QString last = c.value(QStringLiteral("lastName")).toString();
    const QString fullName = c.value(QStringLiteral("fullName")).toString().isEmpty()
        ? (first + QLatin1Char(' ') + last).trimmed()
        : c.value(QStringLiteral("fullName")).toString();

    QString v;
    v += QStringLiteral("BEGIN:VCARD\r\nVERSION:3.0\r\n");
    v += QStringLiteral("UID:") + uid + QStringLiteral("\r\n");
    v += QStringLiteral("FN:") + fullName + QStringLiteral("\r\n");
    v += QStringLiteral("N:") + last + QLatin1Char(';') + first + QStringLiteral(";;;\r\n");
    if (!c.value(QStringLiteral("email")).toString().isEmpty())
        v += QStringLiteral("EMAIL:") + c.value(QStringLiteral("email")).toString() + QStringLiteral("\r\n");
    if (!c.value(QStringLiteral("phone")).toString().isEmpty())
        v += QStringLiteral("TEL:") + c.value(QStringLiteral("phone")).toString() + QStringLiteral("\r\n");
    if (!c.value(QStringLiteral("organization")).toString().isEmpty())
        v += QStringLiteral("ORG:") + c.value(QStringLiteral("organization")).toString() + QStringLiteral("\r\n");
    v += QStringLiteral("END:VCARD\r\n");
    return v;
}

}
