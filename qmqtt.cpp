#include "qmqtt_p.hpp"

#include <QSslCertificate>
#include <QDataStream>

QMqttClient::QMqttClient(QObject *parent)
    : QObject(parent)
    , d(new QMqttClientPrivate(this))
{
    connect(&(d->socket), SIGNAL(error(QAbstractSocket::SocketError)),
           this, SIGNAL(socketError(QAbstractSocket::SocketError)));
    connect(&(d->socket), SIGNAL(sslErrors (const QList<QSslError> &)),
           this, SIGNAL(sslErrors(const QList<QSslError> &)));

    connect(&(d->socket), SIGNAL(disconnected()), d, SLOT(disconnected()));
    connect(&(d->socket), SIGNAL(readyRead()), d, SLOT(readyRead()));
}

QMqttClient::~QMqttClient()
{
}

void QMqttClient::connectToHostEncrypted (const QString & hostName, quint16 port)
{
    disconnect(d, SLOT(connected()));
    connect(&(d->socket), SIGNAL(encrypted()), d, SLOT(connected()));
    d->socket.connectToHostEncrypted(hostName, port);
}

void QMqttClient::connectToHost (const QString & hostName, quint16 port)
{
    disconnect(d, SLOT(connected()));
    connect(&(d->socket), SIGNAL(connected()), d, SLOT(connected()));
    d->socket.connectToHost(hostName, port);
}

void QMqttClient::setCaCertificates (const QList<QSslCertificate> &certs)
{
    d->socket.setCaCertificates(certs);
}

void QMqttClient::setUsername(const QString &username)
{
    d->username = username;
}

void QMqttClient::setPassword(const QString &password)
{
    d->password = password;
}

void QMqttClient::setClientId(const QString &id)
{
    d->clientId = id;
}

void QMqttClientPrivate::connected()
{
    int     keepalive = 300;
    bool    cleansess = false;

    Frame frame(Frame::CONNECT, 1);
    frame.writeString(PROTOCOL_MAGIC);
    frame.writeByte(MQTT_PROTO_MAJOR);

    quint8 flags = 0;
    //flags
    flags = FLAG_CLEANSESS(flags, cleansess ? 1 : 0 );
    /*
    flags = FLAG_WILL(flags, will ? 1 : 0);
    if (!willTopic.isEmpty()) {
        flags = FLAG_WILLQOS(flags, will->qos());
        flags = FLAG_WILLRETAIN(flags, will->retain() ? 1 : 0);
    }
    */
    if(!username.isEmpty()) {
        flags = FLAG_USERNAME(flags, 1);
    }
    if (!password.isEmpty()) {
        flags = FLAG_PASSWD(flags, 1);
    }
    frame.writeByte(flags);
    frame.writeInt(keepalive);
    frame.writeString(clientId);
    /*
    if(will != NULL) {
        frame.writeString(will->topic());
        frame.writeString(will->message());
    }
    */
    if (!username.isEmpty()) {
        frame.writeString(username);
    }
    if (!password.isEmpty()) {
        frame.writeString(password);
    }


    frame.writeTo(&socket);
}

void QMqttClientPrivate::disconnected()
{
    p->disconnected();
}

static int readPayloadSize(QIODevice &in)
{
    qint8 byte;
    int len = 0;
    int mul = 1;
    do {
        //TODO we hope we got the whole header in one tcp package
        in.getChar((char*)&byte);
        len += (byte & 127) * mul;
        mul *= 128  ;
    } while ((byte & 128) != 0);
    return len;
}

void QMqttClientPrivate::readyRead()
{
    while (socket.bytesAvailable()) {
        if (mExpectedSize == 0) {
            if (socket.bytesAvailable() < 2)
                return;

            socket.getChar((char*)&mNextHeader);
            mExpectedSize = readPayloadSize(socket);
        } else {
            if (socket.bytesAvailable() < mExpectedSize)
                return;
            QByteArray data = socket.read(mExpectedSize);
            mExpectedSize = 0;

            Frame frame(mNextHeader, data);
            switch (frame.type) {
                case Frame::CONNACK:
                    onCONNACK(frame);
                    break;
                case Frame::PUBACK:
                    onPUBACK(frame);
                    break;
                case Frame::SUBACK:
                    onSUBACK(frame);
                    break;
                case Frame::PUBLISH:
                    onPUBLISH(frame);
                    break;
                default:
                    qDebug() << "unhandled message type " << frame.type;
            }
        }
    }
}

void QMqttClientPrivate::onCONNACK(Frame &frame)
{
    frame.readByte();
    quint8 status = frame.readByte();
    qDebug() << "connection status: " << status;
    if (status != 0) {
        p->protocolError(QMqttClient::ProtocolError(status));
        socket.disconnectFromHost();
        return;
    }
    p->connected();
}

void QMqttClientPrivate::onPUBACK(Frame &frame)
{
    int id = frame.readInt();
    qDebug() << "ackd publish " << id;
}

void QMqttClientPrivate::onSUBACK(Frame &frame)
{
    int id = frame.readInt();
    qDebug() << "ack sub" << id;
}

void QMqttClientPrivate::onPUBLISH(Frame &frame)
{
    QString topic = frame.readString();

    if (frame.qos == 1) {
        quint16 id = frame.readInt();
        Frame frame(Frame::PUBACK, 0);
        frame.writeInt(id);
        frame.writeTo(&socket);
    } else if (frame.qos == 2) {
        //TODO
    }

    QByteArray m  = frame.data;
    p->published(topic, m);

}

void QMqttClient::publish(const QString &topic, const QByteArray &message, int qos)
{
    Frame frame(Frame::PUBLISH, qos);
    frame.writeString(topic);
    if (++d->nextMessageId == 0) d->nextMessageId = 1;
    frame.writeInt(d->nextMessageId);
    frame.writeRawData(message);

    frame.writeTo(&(d->socket));

}

void QMqttClient::subscribe (const QString &topic, int qos)
{
    Frame frame(Frame::SUBSCRIBE, 1);
    if (++d->nextMessageId == 0) d->nextMessageId = 1;
    frame.writeInt(d->nextMessageId);

    frame.writeString(topic);
    frame.writeByte(qos);

    frame.writeTo(&(d->socket));
}

