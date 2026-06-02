#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QtGlobal>

namespace Protocol
{
    static const quint8 HEADER_1 = 0xAA;
    static const quint8 HEADER_2 = 0xBB;

    static const quint8 MSG_ACK            = 0x10;
    static const quint8 MSG_NACK           = 0x20;
    static const quint8 MSG_SET_PARAMETERS = 0x11;
    static const quint8 MSG_GET_PARAMETERS = 0x12;

    QByteArray buildFrame(quint8 type, const QByteArray &payload);
    quint8 checksum(quint8 type, quint16 length, const QByteArray &payload);

    void appendUInt16LE(QByteArray &data, quint16 value);
    void appendUInt32LE(QByteArray &data, quint32 value);
    quint16 readUInt16LE(const QByteArray &data, int offset);
    quint32 readUInt32LE(const QByteArray &data, int offset);
    float readFloatLE(const QByteArray &data, int offset);
}

#endif
