#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QVariantMap>

class Account : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString provider READ provider WRITE setProvider NOTIFY providerChanged)
    Q_PROPERTY(QString providerName READ providerName NOTIFY providerChanged)
    Q_PROPERTY(QString color READ color NOTIFY providerChanged)
    Q_PROPERTY(QString imapHost READ imapHost WRITE setImapHost NOTIFY imapHostChanged)
    Q_PROPERTY(int imapPort READ imapPort WRITE setImapPort NOTIFY imapPortChanged)
    Q_PROPERTY(QString smtpHost READ smtpHost WRITE setSmtpHost NOTIFY smtpHostChanged)
    Q_PROPERTY(int smtpPort READ smtpPort WRITE setSmtpPort NOTIFY smtpPortChanged)
    Q_PROPERTY(QString caldavUrl READ caldavUrl WRITE setCaldavUrl NOTIFY caldavUrlChanged)
    Q_PROPERTY(QString carddavUrl READ carddavUrl WRITE setCarddavUrl NOTIFY carddavUrlChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool allowUntrusted READ allowUntrusted WRITE setAllowUntrusted NOTIFY allowUntrustedChanged)
    Q_PROPERTY(QString authMethod READ authMethod WRITE setAuthMethod NOTIFY authMethodChanged)
    Q_PROPERTY(QString oauthClientId READ oauthClientId WRITE setOauthClientId NOTIFY oauthClientIdChanged)
    Q_PROPERTY(QString oauthClientSecret READ oauthClientSecret WRITE setOauthClientSecret NOTIFY oauthClientSecretChanged)
    Q_PROPERTY(QString ewsUrl READ ewsUrl WRITE setEwsUrl NOTIFY ewsUrlChanged)
    Q_PROPERTY(bool oauthReady READ oauthReady NOTIFY oauthReadyChanged)
    Q_PROPERTY(QString initials READ initials NOTIFY nameChanged)

public:
    explicit Account(QObject *parent = nullptr);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &v);

    QString email() const { return m_email; }
    void setEmail(const QString &v);

    QString provider() const { return m_provider; }
    void setProvider(const QString &v);

    QString providerName() const { return m_providerName; }
    QString color() const { return m_color; }

    QString imapHost() const { return m_imapHost; }
    void setImapHost(const QString &v);

    int imapPort() const { return m_imapPort; }
    void setImapPort(int v);

    QString smtpHost() const { return m_smtpHost; }
    void setSmtpHost(const QString &v);

    int smtpPort() const { return m_smtpPort; }
    void setSmtpPort(int v);

    QString caldavUrl() const { return m_caldavUrl; }
    void setCaldavUrl(const QString &v);

    QString carddavUrl() const { return m_carddavUrl; }
    void setCarddavUrl(const QString &v);

    QString password() const;
    void setPassword(const QString &v);

    bool allowUntrusted() const { return m_allowUntrusted; }
    void setAllowUntrusted(bool v);

    QString authMethod() const { return m_authMethod; }
    void setAuthMethod(const QString &v);

    QString oauthClientId() const { return m_oauthClientId; }
    void setOauthClientId(const QString &v);

    QString oauthClientSecret() const { return m_oauthClientSecret; }
    void setOauthClientSecret(const QString &v);

    QString ewsUrl() const { return m_ewsUrl; }
    void setEwsUrl(const QString &v);

    bool oauthReady() const;

    QString initials() const;

    QVariantMap toMap() const;

signals:
    void nameChanged();
    void emailChanged();
    void providerChanged();
    void imapHostChanged();
    void imapPortChanged();
    void smtpHostChanged();
    void smtpPortChanged();
    void caldavUrlChanged();
    void carddavUrlChanged();
    void passwordChanged();
    void allowUntrustedChanged();
    void authMethodChanged();
    void oauthClientIdChanged();
    void oauthClientSecretChanged();
    void ewsUrlChanged();
    void oauthReadyChanged();

private:
    int m_id = 0;
    QString m_name;
    QString m_email;
    QString m_provider;
    QString m_providerName;
    QString m_color;
    QString m_imapHost;
    int m_imapPort = 993;
    QString m_smtpHost;
    int m_smtpPort = 465;
    QString m_caldavUrl;
    QString m_carddavUrl;
    bool m_allowUntrusted = false;
    QString m_authMethod = QStringLiteral("password");
    QString m_oauthClientId;
    QString m_oauthClientSecret;
    QString m_ewsUrl;
};

#endif // ACCOUNT_H
