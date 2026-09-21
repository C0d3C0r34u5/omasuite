#ifndef IMAPCLIENT_H
#define IMAPCLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QVariantList>
#include <QHash>
#include <functional>

class ImapClient : public QObject
{
    Q_OBJECT
public:
    explicit ImapClient(QObject *parent = nullptr);

    void connectToServer(const QString &host, int port);
    void login(const QString &user, const QString &pass);
    void loginXoauth2(const QString &user, const QString &accessToken);
    void listFolders();
    void selectFolder(const QString &folder);
    void fetchFolder(const QString &folder, int limit = 100);
    void fetchBody(const QString &folder, int uid);
    void logout();

    bool isConnected() const { return m_state >= Connected; }
    QString errorString() const { return m_error; }
    void setAllowUntrusted(bool allow) { m_allowUntrusted = allow; }

signals:
    void connected();
    void authenticated();
    void failed(const QString &reason);
    void authenticationFailed();
    void foldersFetched(const QVariantList &folders);
    void messagesFetched(const QVariantList &messages);
    void bodyFetched(int uid, const QString &body);

private slots:
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onDisconnected();

private:
    enum State { Disconnected, Connecting, Connected, Authenticated };
    State m_state = Disconnected;

    void sendCommand(const QString &cmd, std::function<void(const QString &tag, const QString &status, const QString &text)> handler);
    void handleLine(const QByteArray &line);
    void handleTagged(const QByteArray &line);
    void handleUntagged(const QByteArray &line);

    void parseList(const QByteArray &line);
    void parseFetch(const QByteArray &line);
    void completeFetch(int limit);

    QSslSocket *m_socket = nullptr;
    QByteArray m_buffer;
    QByteArray m_line;
    int m_literalRemaining = -1;
    int m_nextTag = 0;
    bool m_allowUntrusted = false;
    QString m_user;
    QString m_pass;
    QString m_error;
    QString m_selectedFolder;

    struct Command {
        std::function<void(const QString &tag, const QString &status, const QString &text)> handler;
    };
    QHash<QString, Command> m_commands;

    // Folder listing accumulation
    QVariantList m_folderList;

    // Fetch accumulation
    struct MsgPart {
        int seq = 0;
        int uid = 0;
        bool seen = false;
        bool flagged = false;
        QString from;
        QString to;
        QString subject;
        QString date;
        QString preview;
        QString internalDate;
        qint64 timestampMs = 0;
    };
    QList<MsgPart> m_msgParts;
    QHash<int, MsgPart> m_msgs; // keyed by seq
    QString m_pendingBody;
    int m_pendingBodyUid = 0;
    bool m_fetchingBody = false;
};

#endif // IMAPCLIENT_H
