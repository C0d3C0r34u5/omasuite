#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QColor>
#include <QHash>
#include <QString>

class QFileSystemWatcher;

// Exposes the app's color palette to QML.
//
// When running on Omarchy, the palette is derived from the active theme's
// colors.toml (~/.local/state/omarchy/current/theme/colors.toml) and hot-reloads
// whenever the user changes the theme from the picker, matching the shell. On
// other systems it falls back to the desktop color scheme (light/dark).
class Theme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool dark READ dark NOTIFY paletteChanged)

    Q_PROPERTY(QColor accent READ accent NOTIFY paletteChanged)
    Q_PROPERTY(QColor accentHover READ accentHover NOTIFY paletteChanged)
    Q_PROPERTY(QColor accentDisabled READ accentDisabled NOTIFY paletteChanged)
    Q_PROPERTY(QColor accentLight READ accentLight NOTIFY paletteChanged)

    Q_PROPERTY(QColor bg READ bg NOTIFY paletteChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY paletteChanged)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt NOTIFY paletteChanged)
    Q_PROPERTY(QColor sidebar READ sidebar NOTIFY paletteChanged)
    Q_PROPERTY(QColor sidebarAlt READ sidebarAlt NOTIFY paletteChanged)

    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY paletteChanged)
    Q_PROPERTY(QColor textBody READ textBody NOTIFY paletteChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY paletteChanged)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY paletteChanged)
    Q_PROPERTY(QColor textFaint READ textFaint NOTIFY paletteChanged)

    Q_PROPERTY(QColor border READ border NOTIFY paletteChanged)
    Q_PROPERTY(QColor danger READ danger NOTIFY paletteChanged)
    Q_PROPERTY(QColor hoverOverlay READ hoverOverlay NOTIFY paletteChanged)

    Q_PROPERTY(QColor star READ star NOTIFY paletteChanged)
    Q_PROPERTY(QColor eventChip READ eventChip NOTIFY paletteChanged)
    Q_PROPERTY(QColor eventChipText READ eventChipText NOTIFY paletteChanged)
    Q_PROPERTY(QColor eventCard READ eventCard NOTIFY paletteChanged)

public:
    explicit Theme(QObject *parent = nullptr);

    bool dark() const { return m_dark; }
    QColor accent() const { return m_accent; }
    QColor accentHover() const { return m_accentHover; }
    QColor accentDisabled() const { return m_accentDisabled; }
    QColor accentLight() const { return m_accentLight; }
    QColor bg() const { return m_bg; }
    QColor surface() const { return m_surface; }
    QColor surfaceAlt() const { return m_surfaceAlt; }
    QColor sidebar() const { return m_sidebar; }
    QColor sidebarAlt() const { return m_sidebarAlt; }
    QColor textPrimary() const { return m_textPrimary; }
    QColor textBody() const { return m_textBody; }
    QColor textSecondary() const { return m_textSecondary; }
    QColor textMuted() const { return m_textMuted; }
    QColor textFaint() const { return m_textFaint; }
    QColor border() const { return m_border; }
    QColor danger() const { return m_danger; }
    QColor hoverOverlay() const { return m_hoverOverlay; }
    QColor star() const { return m_star; }
    QColor eventChip() const { return m_eventChip; }
    QColor eventChipText() const { return m_eventChipText; }
    QColor eventCard() const { return m_eventCard; }

signals:
    void paletteChanged();

private slots:
    void reload();

private:
    void setupWatch();
    QString resolve(const QStringList &keys) const;

    QFileSystemWatcher *m_watcher = nullptr;
    QHash<QString, QString> m_colors;

    bool m_dark = false;
    QColor m_accent, m_accentHover, m_accentDisabled, m_accentLight;
    QColor m_bg, m_surface, m_surfaceAlt, m_sidebar, m_sidebarAlt;
    QColor m_textPrimary, m_textBody, m_textSecondary, m_textMuted, m_textFaint;
    QColor m_border, m_danger, m_hoverOverlay;
    QColor m_star, m_eventChip, m_eventChipText, m_eventCard;
};

#endif // THEME_H
