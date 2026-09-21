#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QQuickStyle>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QQuickWindow>
#include <QDateTime>
#include <QDir>
#include <cstdio>

#include "appcore.h"
#include "theme.h"
#include "account/accountmanager.h"
#include "account/account.h"
#include "mail/mailmodel.h"
#include "calendar/calendarmodel.h"
#include "contacts/contactmodel.h"
#include "tasks/taskmodel.h"
#include "storage/database.h"

static void messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Q_UNUSED(ctx);
    const QByteArray local = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:    fprintf(stderr, "[debug] %s\n", local.constData()); break;
    case QtInfoMsg:     fprintf(stderr, "[info ] %s\n", local.constData()); break;
    case QtWarningMsg:  fprintf(stderr, "[warn ] %s\n", local.constData()); break;
    case QtCriticalMsg: fprintf(stderr, "[crit ] %s\n", local.constData()); break;
    case QtFatalMsg:    fprintf(stderr, "[fatal] %s\n", local.constData()); break;
    }
    fflush(stderr);
}

static void seedSampleData(AppCore &core)
{
    QVariantMap a;
    a[QStringLiteral("name")] = QStringLiteral("Brett Example");
    a[QStringLiteral("email")] = QStringLiteral("brett@example.com");
    a[QStringLiteral("provider")] = QStringLiteral("gmail");
    core.accountManager()->addAccount(a);

    const int acct = core.accountManager()->firstAccount()->id();
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 day = 24LL * 3600 * 1000;

    core.mailModel()->addSynced(acct, QStringLiteral("m1"), QStringLiteral("Weekly team sync notes"),
                                QStringLiteral("Alice Cooper"), QStringLiteral("Me"),
                                QStringLiteral("Hi Brett,\n\nHere are the notes from today's team sync.\n- Ship v0.2\n- Review OAuth flow\n- Fix calendar drag\n\nThanks!"),
                                now - 2 * 3600 * 1000, true, true);
    core.mailModel()->addSynced(acct, QStringLiteral("m2"), QStringLiteral("Your receipt from Omarchy Store"),
                                QStringLiteral("Omarchy Store"), QStringLiteral("Me"),
                                QStringLiteral("Thank you for your purchase. Total: $12.99"),
                                now - 1 * day, false, false);
    core.mailModel()->addSynced(acct, QStringLiteral("m3"), QStringLiteral("Re: Project OmaSuite kickoff"),
                                QStringLiteral("Dana Whitfield"), QStringLiteral("Me"),
                                QStringLiteral("Sounds good! Let's aim for a public beta next month."),
                                now - 2 * day, false, false);
    core.mailModel()->addSynced(acct, QStringLiteral("m4"), QStringLiteral("Flight confirmation to SYD"),
                                QStringLiteral("Qantas Airways"), QStringLiteral("Me"),
                                QStringLiteral("Your flight QF123 is confirmed."),
                                now - 4 * day, true, false);

    core.calendarModel()->addSynced(acct, QStringLiteral("e1"), QStringLiteral("Design review"),
                                    QStringLiteral("Review the new sidebar mockups."), QStringLiteral("Meeting room 2"),
                                    now + 3600 * 1000, now + 2 * 3600 * 1000, false);
    core.calendarModel()->addSynced(acct, QStringLiteral("e2"), QStringLiteral("Lunch with Sam"),
                                    QString(), QStringLiteral("Cafe Lune"),
                                    now + 5 * 3600 * 1000, now + 6 * 3600 * 1000, false);
    core.calendarModel()->addSynced(acct, QStringLiteral("e3"), QStringLiteral("Omarchy release day"),
                                    QString(), QString(),
                                    now + 2 * day, now + 3 * day, true);

    core.contactModel()->addSynced(acct, QStringLiteral("c1"), QStringLiteral("Alice"), QStringLiteral("Cooper"),
                                    QStringLiteral("alice@example.com"), QStringLiteral("+61 400 000 000"), QStringLiteral("Omarchy"));
    core.contactModel()->addSynced(acct, QStringLiteral("c2"), QStringLiteral("Dana"), QStringLiteral("Whitfield"),
                                    QStringLiteral("dana@example.com"), QString(), QStringLiteral("Design"));
    core.contactModel()->addSynced(acct, QStringLiteral("c3"), QStringLiteral("Sam"), QStringLiteral("Rodriguez"),
                                    QStringLiteral("sam@example.com"), QStringLiteral("+1 555 0100"), QString());

    core.taskModel()->addSynced(acct, QStringLiteral("t1"), QStringLiteral("Ship OmaSuite v0.2"),
                                  QStringLiteral("Package and tag the release."), now + 3 * day, false);
    core.taskModel()->addSynced(acct, QStringLiteral("t2"), QStringLiteral("Write OAuth setup docs"),
                                  QString(), now + 1 * day, false);
    core.taskModel()->addSynced(acct, QStringLiteral("t3"), QStringLiteral("Fix calendar drag"),
                                  QString(), now, true);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("OmaSuite"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("Omarchy"));
    app.setOrganizationDomain(QStringLiteral("omarchy.org"));

    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    const QStringList args = app.arguments();
    const int shotIdx = args.indexOf(QStringLiteral("--screenshot"));
    QString shotPath;
    bool screenshotMode = false;
    if (shotIdx >= 0 && shotIdx + 1 < args.size()) {
        shotPath = args.at(shotIdx + 1);
        screenshotMode = true;
        const QString tmpData = QStringLiteral("/tmp/omasuite-screenshot");
        QDir(tmpData).removeRecursively();
        qputenv("XDG_DATA_HOME", tmpData.toUtf8());
    }

    if (!Database::instance()->initialize()) {
        return 1;
    }

    AppCore core;
    core.accountManager()->load();

    Theme theme;

    if (screenshotMode)
        seedSampleData(core);

    core.refreshAll();

    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "AppCore", &core);
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "Accounts", core.accountManager());
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "Sync", core.syncController());
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "OAuth", core.oauthManager());
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "Theme", &theme);

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/"));

    QObject::connect(&engine, &QQmlEngine::warnings, &app,
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &e : warnings)
            qWarning().noquote() << e.toString();
    });

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    if (screenshotMode) {
        QTimer::singleShot(2500, [&engine, shotPath]() {
            QObject *root = engine.rootObjects().value(0);
            auto *win = qobject_cast<QQuickWindow *>(root);
            if (win) {
                const QImage img = win->grabWindow();
                if (img.save(shotPath))
                    fprintf(stderr, "screenshot saved to %s\n", qPrintable(shotPath));
                else
                    fprintf(stderr, "failed to save screenshot\n");
            } else {
                fprintf(stderr, "no window to grab\n");
            }
            QCoreApplication::exit(0);
        });
    }

    return app.exec();
}
