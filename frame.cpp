#include "frame.hpp"

#include <QDebug>

Frame::Frame(quint8 header, const QByteArray &data)
    : data(data)
{
    type   = Frame::MessageType(header >> 4);
    dup    = header & (1 << 3);
    qos    = (header >> 1) & 3;
    retain = header & 1;
}

quint8 Frame::readByte()
{
    char c = data.at(0);
    data.remove(0, 1);
    return c;
}

quint16 Frame::readInt()
{
    char msb = data.at(0);
    char lsb = data.at(1);
    data.remove(0, 2);
    //TODO: this doesnt look right. will this convert to a wider type before shift?
    return (msb << 8) + lsb;
}

QString Frame::readString()
{
    int len = readInt();
    QString s = QString::fromUtf8(data.left(len));
    data.remove(0, len);
    return s;
}

//TODO this is BS. just write the 16bit int in big endian
void Frame::writeInt(quint16 i)
{
    data.append(MSB(i));
    data.append(LSB(i));
}

void Frame::writeString(const QString &string)
{
    writeInt(string.size());
    data.append(string.toUtf8());
}

void Frame::writeByte(quint8 c)
{
    data.append(char(c));
}

void Frame::writeRawData(const QByteArray &d)
{
    data.append(d);
}

void Frame::writeTo(QIODevice *io) const
{
    quint8 header = type << 4;
    if (dup)
        header |= 1 << 3;
    header |= qos << 1;
    if (retain)
        header  |= 1;
    io->putChar(header);

    if (data.size() == 0) {
        io->putChar(0);
        return;
    }

    //length
    int length = data.size();
    quint8 d;
    do {
        d = length % 128;
        length /= 128;
        if (length > 0) {
            d |= 0x80;
        }
        io->putChar(d);
    } while (length > 0);

    io->write(data);
}
