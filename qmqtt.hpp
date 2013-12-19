#include <QObject>
#include <QSslCertificate>
#include <QAbstractSocket>

class QMqttClientPrivate;
class QMqttClient : public QObject
{
Q_OBJECT
public:

    enum ProtocolError {
        RefusedProtocolVersion    = 0x1,
        RefusedIdentifier         = 0x2,
        RefusedServerUnavailable  = 0x3,
        RefusedBadLogin           = 0x4,
        RefusedNotAuthorized      = 0x5
    };


    QMqttClient(QObject *parent = 0);
    ~QMqttClient();

    void connectToHostEncrypted (const QString & hostName, quint16 port);
    void connectToHost          (const QString & hostName, quint16 port);

    void setCaCertificates(const QList<QSslCertificate> &certs);
    void setClientId(const QString &id);
    void setUsername(const QString &username);
    void setPassword(const QString &password);

public slots:

    void subscribe (const QString &topic, int qos = 1);
    void publish   (const QString &topic, const QByteArray &message, int qos = 1);

signals:

    void published (const QString &topic, const QByteArray &message);

    void protocolError(ProtocolError e);
    void socketError(QAbstractSocket::SocketError);
    void sslErrors (const QList<QSslError> &);
    void connected();
    void disconnected();

private:
    friend class QMqttClientPrivate;
    QMqttClientPrivate *d;
};
