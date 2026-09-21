#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QColor>

// Exposes the app's color palette to QML and follows the system color scheme
// (light / dark) automatically.
class Theme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool dark READ dark NOTIFY darkChanged)

    Q_PROPERTY(QColor accent READ accent CONSTANT)
    Q_PROPERTY(QColor accentHover READ accentHover CONSTANT)
    Q_PROPERTY(QColor accentDisabled READ accentDisabled NOTIFY darkChanged)
    Q_PROPERTY(QColor accentLight READ accentLight NOTIFY darkChanged)

    Q_PROPERTY(QColor bg READ bg NOTIFY darkChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY darkChanged)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt NOTIFY darkChanged)
    Q_PROPERTY(QColor sidebar READ sidebar NOTIFY darkChanged)
    Q_PROPERTY(QColor sidebarAlt READ sidebarAlt NOTIFY darkChanged)

    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY darkChanged)
    Q_PROPERTY(QColor textBody READ textBody NOTIFY darkChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY darkChanged)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY darkChanged)
    Q_PROPERTY(QColor textFaint READ textFaint NOTIFY darkChanged)

    Q_PROPERTY(QColor border READ border NOTIFY darkChanged)
    Q_PROPERTY(QColor danger READ danger NOTIFY darkChanged)
    Q_PROPERTY(QColor hoverOverlay READ hoverOverlay NOTIFY darkChanged)

    Q_PROPERTY(QColor star READ star CONSTANT)
    Q_PROPERTY(QColor eventChip READ eventChip NOTIFY darkChanged)
    Q_PROPERTY(QColor eventChipText READ eventChipText NOTIFY darkChanged)
    Q_PROPERTY(QColor eventCard READ eventCard NOTIFY darkChanged)

public:
    explicit Theme(QObject *parent = nullptr);

    bool dark() const;

    QColor accent() const { return QColor(QStringLiteral("#0F6CBD")); }
    QColor accentHover() const { return QColor(QStringLiteral("#0A5CAD")); }
    QColor accentDisabled() const { return dark() ? QColor(QStringLiteral("#5A5A5A")) : QColor(QStringLiteral("#B0B0B0")); }
    QColor accentLight() const { return dark() ? QColor(QStringLiteral("#1E3A5F")) : QColor(QStringLiteral("#E8F1FB")); }

    QColor bg() const { return dark() ? QColor(QStringLiteral("#1E1F22")) : QColor(QStringLiteral("#F3F4F6")); }
    QColor surface() const { return dark() ? QColor(QStringLiteral("#2B2D31")) : QColor(QStringLiteral("#FFFFFF")); }
    QColor surfaceAlt() const { return dark() ? QColor(QStringLiteral("#313338")) : QColor(QStringLiteral("#F7F7F7")); }
    QColor sidebar() const { return dark() ? QColor(QStringLiteral("#26272B")) : QColor(QStringLiteral("#ECEEF1")); }
    QColor sidebarAlt() const { return dark() ? QColor(QStringLiteral("#2E3034")) : QColor(QStringLiteral("#E0E2E6")); }

    QColor textPrimary() const { return dark() ? QColor(QStringLiteral("#ECECEC")) : QColor(QStringLiteral("#1F1F1F")); }
    QColor textBody() const { return dark() ? QColor(QStringLiteral("#D6D6D6")) : QColor(QStringLiteral("#333333")); }
    QColor textSecondary() const { return dark() ? QColor(QStringLiteral("#C0C0C0")) : QColor(QStringLiteral("#444444")); }
    QColor textMuted() const { return dark() ? QColor(QStringLiteral("#9A9A9A")) : QColor(QStringLiteral("#666666")); }
    QColor textFaint() const { return dark() ? QColor(QStringLiteral("#7E7E7E")) : QColor(QStringLiteral("#888888")); }

    QColor border() const { return dark() ? QColor(QStringLiteral("#3F4248")) : QColor(QStringLiteral("#E0E0E0")); }
    QColor danger() const { return dark() ? QColor(QStringLiteral("#E0606C")) : QColor(QStringLiteral("#D13438")); }
    QColor hoverOverlay() const { return dark() ? QColor(255, 255, 255, 15) : QColor(0, 0, 0, 10); }

    QColor star() const { return QColor(QStringLiteral("#F2C811")); }
    QColor eventChip() const { return dark() ? QColor(QStringLiteral("#2E4A68")) : QColor(QStringLiteral("#D6E7F8")); }
    QColor eventChipText() const { return dark() ? QColor(QStringLiteral("#9CC3F0")) : QColor(QStringLiteral("#0F5C9E")); }
    QColor eventCard() const { return dark() ? QColor(QStringLiteral("#33363C")) : QColor(QStringLiteral("#F3F6FA")); }

signals:
    void darkChanged();

private slots:
    void refresh();

private:
    Q_DISABLE_COPY(Theme)
};

#endif // THEME_H
