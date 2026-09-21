#include "theme.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QFile>
#include <QFileSystemWatcher>
#include <QTextStream>
#include <QDir>

namespace {

QColor hexColor(const QString &s, const QColor &fallback)
{
    if (s.size() >= 7 && s[0] == QLatin1Char('#')) {
        const QColor c(s.left(7));
        if (c.isValid())
            return c;
    }
    return fallback;
}

// Returns a*(1-amount) + b*amount.
QColor mix(const QColor &a, const QColor &b, qreal amount)
{
    return QColor::fromRgbF(
        a.redF() * (1.0 - amount) + b.redF() * amount,
        a.greenF() * (1.0 - amount) + b.greenF() * amount,
        a.blueF() * (1.0 - amount) + b.blueF() * amount);
}

} // namespace

Theme::Theme(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &Theme::reload);
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &Theme::reload);

    setupWatch();
    reload();
}

void Theme::setupWatch()
{
    const QStringList dirs = m_watcher->directories();
    if (!dirs.isEmpty())
        m_watcher->removePaths(dirs);

    const QString stateHome = qEnvironmentVariable(
        "XDG_STATE_HOME",
        QDir::homePath() + QStringLiteral("/.local/state"));
    const QString current = stateHome + QStringLiteral("/omarchy/current");
    const QString omarchy = stateHome + QStringLiteral("/omarchy");

    if (QDir(current).exists())
        m_watcher->addPath(current);
    else if (QDir(omarchy).exists())
        m_watcher->addPath(omarchy);
}

QString Theme::resolve(const QStringList &keys) const
{
    for (const QString &k : keys) {
        const auto it = m_colors.constFind(k);
        if (it != m_colors.constEnd())
            return it.value();
    }
    return QString();
}

void Theme::reload()
{
    m_colors.clear();

    const QString stateHome = qEnvironmentVariable(
        "XDG_STATE_HOME",
        QDir::homePath() + QStringLiteral("/.local/state"));
    const QString themeDir = stateHome + QStringLiteral("/omarchy/current/theme");
    const QString colorsFile = themeDir + QStringLiteral("/colors.toml");

    bool haveOmarchy = false;
    QFile f(colorsFile);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
                continue;
            const int eq = line.indexOf(QLatin1Char('='));
            if (eq < 0)
                continue;
            QString key = line.left(eq).trimmed();
            QString value = line.mid(eq + 1).trimmed();
            if (value.size() >= 2
                && ((value.front() == QLatin1Char('"') && value.back() == QLatin1Char('"'))
                    || (value.front() == QLatin1Char('\'') && value.back() == QLatin1Char('\''))))
                value = value.mid(1, value.size() - 2);
            else {
                const int hash = value.indexOf(QLatin1Char('#'));
                if (hash > 0)
                    value = value.left(hash).trimmed();
            }
            m_colors.insert(key, value);
        }
        haveOmarchy = true;
    }

    bool dark = false;
    QString accentStr, bgStr, fgStr, lighterBgStr, darkerBgStr, mutedStr, selectionStr, redStr, yellowStr;

    if (haveOmarchy) {
        // Mirror omarchy-theme-color's mode resolution: `mode`, legacy
        // `theme_type`, a light.mode marker file, then background luminance.
        QString mode = resolve(QStringList{QStringLiteral("mode"), QStringLiteral("theme_type")});
        if (mode.isEmpty() && QFile::exists(themeDir + QStringLiteral("/light.mode")))
            mode = QStringLiteral("light");
        if (mode.isEmpty()) {
            const QString bg = resolve(QStringList{QStringLiteral("background"),
                                                  QStringLiteral("bg"),
                                                  QStringLiteral("color0")});
            const QColor b = hexColor(bg, QColor());
            if (b.isValid())
                mode = (b.red() + b.green() + b.blue()) > 382 ? QStringLiteral("light")
                                                               : QStringLiteral("dark");
        }
        dark = (mode != QStringLiteral("light"));

        accentStr = resolve(QStringList{QStringLiteral("accent"),
                                        QStringLiteral("blue"),
                                        QStringLiteral("color4")});
        bgStr = resolve(QStringList{QStringLiteral("background"),
                                    QStringLiteral("bg"),
                                    QStringLiteral("color0")});
        fgStr = resolve(QStringList{QStringLiteral("foreground"),
                                    QStringLiteral("fg"),
                                    QStringLiteral("color7")});
        lighterBgStr = resolve(QStringList{QStringLiteral("lighter_background"),
                                           QStringLiteral("lighter_bg"),
                                           QStringLiteral("color0"),
                                           QStringLiteral("background")});
        darkerBgStr = resolve(QStringList{QStringLiteral("darker_background"),
                                          QStringLiteral("darker_bg")});
        mutedStr = resolve(QStringList{QStringLiteral("muted"),
                                       QStringLiteral("color8"),
                                       QStringLiteral("dark_foreground")});
        selectionStr = resolve(QStringList{QStringLiteral("selection"),
                                           QStringLiteral("selection_background"),
                                           QStringLiteral("color8"),
                                           QStringLiteral("color0"),
                                           QStringLiteral("background")});
        redStr = resolve(QStringList{QStringLiteral("red"), QStringLiteral("color1")});
        yellowStr = resolve(QStringList{QStringLiteral("yellow"), QStringLiteral("color3")});
    } else {
        dark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
    }

    if (haveOmarchy) {
        const QColor bg = hexColor(bgStr, dark ? QColor(QStringLiteral("#1e1e2e"))
                                               : QColor(QStringLiteral("#eff1f5")));
        const QColor fg = hexColor(fgStr, dark ? QColor(QStringLiteral("#cdd6f4"))
                                               : QColor(QStringLiteral("#4c4f69")));
        const QColor accent = hexColor(accentStr, dark ? QColor(QStringLiteral("#89b4fa"))
                                                       : QColor(QStringLiteral("#1e66f5")));
        const QColor lighterBg = hexColor(lighterBgStr,
                                          mix(bg, QColor(QStringLiteral("#ffffff")), dark ? 0.05 : 0.85));
        const QColor darkerBg = darkerBgStr.isEmpty()
                                    ? mix(bg, QColor(QStringLiteral("#000000")), dark ? 0.25 : 0.06)
                                    : hexColor(darkerBgStr, bg);
        const QColor muted = hexColor(mutedStr, mix(fg, bg, 0.45));
        const QColor selection = hexColor(selectionStr, mix(accent, bg, 0.15));
        const QColor red = hexColor(redStr, dark ? QColor(QStringLiteral("#f38ba8"))
                                                 : QColor(QStringLiteral("#d20f39")));
        const QColor yellow = hexColor(yellowStr, QColor(QStringLiteral("#f9e2af")));

        m_dark = dark;
        m_accent = accent;
        m_accentHover = mix(accent, QColor(QStringLiteral("#000000")), 0.15);
        m_accentDisabled = mix(fg, bg, 0.35);
        m_accentLight = selection;
        m_bg = bg;
        m_surface = lighterBg;
        m_surfaceAlt = mix(bg, lighterBg, 0.5);
        m_sidebar = darkerBg;
        m_sidebarAlt = mix(darkerBg, lighterBg, 0.4);
        m_textPrimary = fg;
        m_textBody = mix(fg, bg, 0.10);
        m_textSecondary = mix(fg, bg, 0.28);
        m_textMuted = muted;
        m_textFaint = mix(fg, bg, 0.58);
        m_border = mix(fg, bg, 0.82);
        m_danger = red;
        m_hoverOverlay = QColor(fg.red(), fg.green(), fg.blue(), dark ? 18 : 12);
        m_star = yellow;
        m_eventChip = mix(accent, lighterBg, 0.80);
        m_eventChipText = accent;
        m_eventCard = mix(lighterBg, bg, 0.4);
    } else {
        m_dark = dark;
        m_accent = QColor(QStringLiteral("#0F6CBD"));
        m_accentHover = QColor(QStringLiteral("#0A5CAD"));
        m_accentDisabled = dark ? QColor(QStringLiteral("#5A5A5A")) : QColor(QStringLiteral("#B0B0B0"));
        m_accentLight = dark ? QColor(QStringLiteral("#1E3A5F")) : QColor(QStringLiteral("#E8F1FB"));
        m_bg = dark ? QColor(QStringLiteral("#1E1F22")) : QColor(QStringLiteral("#F3F4F6"));
        m_surface = dark ? QColor(QStringLiteral("#2B2D31")) : QColor(QStringLiteral("#FFFFFF"));
        m_surfaceAlt = dark ? QColor(QStringLiteral("#313338")) : QColor(QStringLiteral("#F7F7F7"));
        m_sidebar = dark ? QColor(QStringLiteral("#26272B")) : QColor(QStringLiteral("#ECEEF1"));
        m_sidebarAlt = dark ? QColor(QStringLiteral("#2E3034")) : QColor(QStringLiteral("#E0E2E6"));
        m_textPrimary = dark ? QColor(QStringLiteral("#ECECEC")) : QColor(QStringLiteral("#1F1F1F"));
        m_textBody = dark ? QColor(QStringLiteral("#D6D6D6")) : QColor(QStringLiteral("#333333"));
        m_textSecondary = dark ? QColor(QStringLiteral("#C0C0C0")) : QColor(QStringLiteral("#444444"));
        m_textMuted = dark ? QColor(QStringLiteral("#9A9A9A")) : QColor(QStringLiteral("#666666"));
        m_textFaint = dark ? QColor(QStringLiteral("#7E7E7E")) : QColor(QStringLiteral("#888888"));
        m_border = dark ? QColor(QStringLiteral("#3F4248")) : QColor(QStringLiteral("#E0E0E0"));
        m_danger = dark ? QColor(QStringLiteral("#E0606C")) : QColor(QStringLiteral("#D13438"));
        m_hoverOverlay = dark ? QColor(255, 255, 255, 15) : QColor(0, 0, 0, 10);
        m_star = QColor(QStringLiteral("#F2C811"));
        m_eventChip = dark ? QColor(QStringLiteral("#2E4A68")) : QColor(QStringLiteral("#D6E7F8"));
        m_eventChipText = dark ? QColor(QStringLiteral("#9CC3F0")) : QColor(QStringLiteral("#0F5C9E"));
        m_eventCard = dark ? QColor(QStringLiteral("#33363C")) : QColor(QStringLiteral("#F3F6FA"));
    }

    emit paletteChanged();
}
