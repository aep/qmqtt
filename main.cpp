#include <QCoreApplication>
#include <QStringList>

#include "qmqtt.hpp"


static const char *unixbuscert =
"-----BEGIN CERTIFICATE-----\n"
"MIIChjCCAe+gAwIBAgIJAIGtADB0sZzYMA0GCSqGSIb3DQEBBQUAMFwxCzAJBgNV\n"
"BAYTAkRFMQ4wDAYDVQQIDAVXb3JsZDEPMA0GA1UEBwwGQmVybGluMQ0wCwYDVQQK\n"
"DARleHlzMQswCQYDVQQLDAJDQTEQMA4GA1UEAwwHUm9vdCBDQTAeFw0xMzEyMTkx\n"
"ODI5NDlaFw0yMzEyMTcxODI5NDlaMFwxCzAJBgNVBAYTAkRFMQ4wDAYDVQQIDAVX\n"
"b3JsZDEPMA0GA1UEBwwGQmVybGluMQ0wCwYDVQQKDARleHlzMQswCQYDVQQLDAJD\n"
"QTEQMA4GA1UEAwwHUm9vdCBDQTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA\n"
"tScHZHl8ygrEMGvzdmCQHX7Aby4FuBz+fnwk80XZI5J+mhtNpq5MLY7iup3KV8CU\n"
"6tncbiWlshf6ka1jTUBCfQX4baa0NsJM/DQuAlFoI2Y143S7seffL/mDmkmEpZmm\n"
"yLYsYXgYiMhLkRWtSmo2nhfFlvH1Vqbs+bTtIct/HB0CAwEAAaNQME4wHQYDVR0O\n"
"BBYEFDnJcMO0LhqXLzWD3tsHEa8sk5r6MB8GA1UdIwQYMBaAFDnJcMO0LhqXLzWD\n"
"3tsHEa8sk5r6MAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQADgYEAdAtv2HLy\n"
"yLxhqtmk1WZOSvKWAVKAstHbMJ9bQ4F0XmKKKlrcS2ZUOn9RX+k6tdDgOEMJX/je\n"
"03m8dpzbfJACVBujSaTBSrujb7IHhWlfBGhecxMncLcJ26JSDDndnIALlaIhzff8\n"
"Zh3NCV8Ny4xusXWQZbTjzqu5IR/rRhJ4htY=\n"
"-----END CERTIFICATE-----\n"
;


class Main : public QObject
{
    Q_OBJECT;
public:
    QMqttClient client;
    int main(int argc, char **argv) {
        QCoreApplication app(argc, argv);

        connect(&client,  SIGNAL (socketError(QAbstractSocket::SocketError)),
                this,     SLOT   (socketError(QAbstractSocket::SocketError)));
        connect (&client, SIGNAL (sslErrors (const QList<QSslError> &)),
                this,     SLOT   (sslErrors(const QList<QSslError> &)));
        connect(&client,  SIGNAL (disconnected()), this, SLOT(disconnected()));
        connect(&client,  SIGNAL (connected()), this, SLOT(connected()));
        connect(&client,  SIGNAL (published(const QString &, const QByteArray &)),
                this,     SLOT   (published(const QString &, const QByteArray &)));

        client.setCaCertificates(QSslCertificate::fromData(unixbuscert));

        client.setUsername(app.arguments().at(1));
        client.setPassword(app.arguments().at(2));
        client.setClientId("QMqttClient");
        client.connectToHostEncrypted("unixbus.org", 8883);


        return app.exec();
    }

private slots:

    void socketError(QAbstractSocket::SocketError e) {
        qDebug() << "error bla bla" << e;

    }
    void sslErrors(const QList<QSslError> &) {
        qDebug() << "ssl shiat";;
    }
    void connected() {
        qDebug("connected");
        client.subscribe("wurst");
        client.publish("wurst", "lol");
        client.publish("wurst", "warf");
    }
    void disconnected() {
        qFatal("disconnected");
    }

    void published(const QString &t, const QByteArray &m) {
        qDebug(qPrintable("[" + t + "] " + m));
    }
};

int main(int argc, char **argv)
{
    return Main().main(argc, argv);
}

#include "main.moc"
