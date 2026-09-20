#ifndef VCARDUTILS_H
#define VCARDUTILS_H

#include <QVariantList>
#include <QVariantMap>
#include <QString>

namespace VCardUtils {

QVariantList parseVCards(const QString &vcf);
QString buildVCard(const QVariantMap &contact);

}

#endif // VCARDUTILS_H
