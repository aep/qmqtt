#include "qmqtt.hpp"
#include <QSslSocket>

#include "frame.hpp"

class QMqttClientPrivate : public QObject
{
Q_OBJECT
public:
    QMqttClientPrivate(QMqttClient *p)
        : QObject (p)
        , p(p)
        , mExpectedSize(0)
        , nextMessageId(1)
    {
    }

    QMqttClient *p;

    QSslSocket socket;

    int    mExpectedSize;
    quint8 mNextHeader;

    QString clientId;
    QString username;
    QString password;

    quint16 nextMessageId;

public slots:

    void connected();
    void disconnected();
    void readyRead();

private:
    void onCONNACK (Frame &frame);
    void onPUBACK  (Frame &frame);
    void onSUBACK  (Frame &frame);
    void onPUBLISH (Frame &frame);


};
