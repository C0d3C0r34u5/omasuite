#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QQuickStyle>
#include <QDebug>
#include <cstdio>

#include "appcore.h"
#include "account/accountmanager.h"
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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("OmaSuite"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("Omarchy"));
    app.setOrganizationDomain(QStringLiteral("omarchy.org"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    if (!Database::instance()->initialize()) {
        return 1;
    }

    AppCore core;
    core.accountManager()->load();
    core.refreshAll();

    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "AppCore", &core);
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "Accounts", core.accountManager());
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "Sync", core.syncController());
    qmlRegisterSingletonInstance("OmaSuite", 1, 0, "OAuth", core.oauthManager());

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

    return app.exec();
}
