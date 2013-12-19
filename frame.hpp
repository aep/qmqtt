
#define MQTT_PROTO_MAJOR 3
#define MQTT_PROTO_MINOR 1
#define MQTT_PROTOCOL_VERSION "MQTT/3.1"

#define PROTOCOL_MAGIC "MQIsdp"

#define LSB(A) (quint8)(A & 0x00FF)
#define MSB(A) (quint8)((A & 0xFF00) >> 8)

#define FLAG_CLEANSESS(F, C)	(F | ((C) << 1))
#define FLAG_WILL(F, W)			(F | ((W) << 2))
#define FLAG_WILLQOS(F, Q)		(F | ((Q) << 3))
#define FLAG_WILLRETAIN(F, R) 	(F | ((R) << 5))
#define FLAG_PASSWD(F, P)		(F | ((P) << 6))
#define FLAG_USERNAME(F, U)		(F | ((U) << 7))


#include <QByteArray>
#include <QIODevice>
class Frame
{
public:
    enum MessageType
    {
        //0,  //Reserved
        CONNECT     = 1,  //Client request to connect to Server
        CONNACK     = 2,  //Connect Acknowledgment
        PUBLISH     = 3,  //Publish message
        PUBACK      = 4,  //Publish Acknowledgment
        PUBREC      = 5,  //Publish Received (assured delivery part 1)
        PUBREL      = 6,  //Publish Release (assured delivery part 2)
        PUBCOMP     = 7,  //Publish Complete (assured delivery part 3)
        SUBSCRIBE   = 8,  //Client Subscribe request
        SUBACK      = 9,  //Subscribe Acknowledgment
        UNSUBSCRIBE = 10, //Client Unsubscribe request
        UNSUBACK11  = 11, //Unsubscribe Acknowledgment
        PINGREQ     = 12, //PING Request
        PINGRESP    = 13, //PING Response
        DISCONNECT  = 14, //Client is Disconnecting
        //15  // Reserved
    };

    Frame(MessageType type = MessageType(0), int qos = 0, bool retain = false, bool dup = false)
        : expectedSize(0)
        , type(type)
        , qos(qos)
        , retain(retain)
        , dup(dup)
    { }
    int expectedSize;
    MessageType type;
    quint8 qos;
    bool retain;
    bool dup;
    QByteArray data;

    Frame(quint8 header, const QByteArray &data);

    int     size() const { return data.size(); }
    quint16 readInt();
    quint8  readByte();
    QString readString();

    void writeInt(quint16 i);
    void writeByte(quint8 c);
    void writeString(const QString &string);
    void writeRawData(const QByteArray &data);


    void writeTo(QIODevice *io) const;
};

