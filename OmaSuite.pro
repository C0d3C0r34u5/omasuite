QT += quick quickcontrols2 network sql networkauth core gui

CONFIG += c++17 link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libsecret-1

TARGET = omasuite
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/theme.cpp \
    src/appcore.cpp \
    src/storage/database.cpp \
    src/storage/secretstore.cpp \
    src/account/account.cpp \
    src/account/accountmanager.cpp \
    src/account/provider.cpp \
    src/mail/mailmodel.cpp \
    src/mail/imapclient.cpp \
    src/mail/smtpclient.cpp \
    src/calendar/calendarmodel.cpp \
    src/calendar/caldavclient.cpp \
    src/contacts/contactmodel.cpp \
    src/contacts/carddavclient.cpp \
    src/tasks/taskmodel.cpp \
    src/net/mimeutils.cpp \
    src/net/icalutils.cpp \
    src/net/vcardutils.cpp \
    src/net/synccontroller.cpp \
    src/net/oauth2manager.cpp \
    src/net/ewsclient.cpp

HEADERS += \
    src/appcore.h \
    src/theme.h \
    src/storage/database.h \
    src/storage/secretstore.h \
    src/account/account.h \
    src/account/accountmanager.h \
    src/account/provider.h \
    src/mail/mailmodel.h \
    src/mail/imapclient.h \
    src/mail/smtpclient.h \
    src/calendar/calendarmodel.h \
    src/calendar/caldavclient.h \
    src/contacts/contactmodel.h \
    src/contacts/carddavclient.h \
    src/tasks/taskmodel.h \
    src/net/mimeutils.h \
    src/net/icalutils.h \
    src/net/vcardutils.h \
    src/net/synccontroller.h \
    src/net/oauth2manager.h \
    src/net/ewsclient.h

RESOURCES += resources.qrc

QML_IMPORT_PATH = qml
QML_DESIGNER_IMPORT_PATH = qml

INCLUDEPATH += src

# Install target
target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
