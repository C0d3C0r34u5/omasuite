#ifndef PROVIDER_H
#define PROVIDER_H

#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace Provider {

enum Type {
    Manual = 0,
    Gmail,
    Outlook,
    Yahoo,
    Apple,
    Exchange
};

QString typeToString(Type t);
Type stringToType(const QString &s);

// Returns a list of provider preset maps for the setup wizard.
QVariantList presets();

// Returns the preset map for a given provider type.
QVariantMap preset(Type t);

// Fills in templated CalDAV/CardDAV URLs for a given email address.
QVariantMap resolveUrls(const QVariantMap &preset, const QString &email);

}

#endif // PROVIDER_H
