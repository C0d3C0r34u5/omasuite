#include "theme.h"

#include <QGuiApplication>
#include <QStyleHints>

Theme::Theme(QObject *parent)
    : QObject(parent)
{
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &Theme::refresh);
}

bool Theme::dark() const
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void Theme::refresh()
{
    emit darkChanged();
}
