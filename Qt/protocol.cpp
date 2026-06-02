#include "protocol.h"
#include <cstring>

namespace Protocol
{
    quint8 checksum(quint8 type, quint16 length, const QByteArray &payload)
    {
        quint32 sum = 0;
        sum += type;
        sum += static_cast<quint8>(length & 0xFF);
        sum += static_cast<quint8>((length >> 8) & 0xFF);
        for (int i = 0; i < payload.size(); ++i)
            sum += static_cast<quint8>(payload.at(i));
        return static_cast<quint8>(sum & 0xFF);
    }

    QByteArray buildFrame(quint8 type, const QByteArray &payload)
    {
        QByteArray frame;
        quint16 length = static_cast<quint16>(payload.size());
        frame.append(static_cast<char>(HEADER_1));
        frame.append(static_cast<char>(HEADER_2));
        frame.append(static_cast<char>(type));
        frame.append(static_cast<char>(length & 0xFF));
        frame.append(static_cast<char>((length >> 8) & 0xFF));
        frame.append(payload);
        frame.append(static_cast<char>(checksum(type, length, payload)));
        return frame;
    }

    void appendUInt16LE(QByteArray &data, quint16 value)
    {
        data.append(static_cast<char>(value & 0xFF));
        data.append(static_cast<char>((value >> 8) & 0xFF));
    }

    void appendUInt32LE(QByteArray &data, quint32 value)
    {
        data.append(static_cast<char>(value & 0xFF));
        data.append(static_cast<char>((value >> 8) & 0xFF));
        data.append(static_cast<char>((value >> 16) & 0xFF));
        data.append(static_cast<char>((value >> 24) & 0xFF));
    }

    quint16 readUInt16LE(const QByteArray &data, int offset)
    {
        quint16 b0 = static_cast<quint8>(data.at(offset));
        quint16 b1 = static_cast<quint8>(data.at(offset + 1));
        return static_cast<quint16>(b0 | (b1 << 8));
    }

    quint32 readUInt32LE(const QByteArray &data, int offset)
    {
        quint32 b0 = static_cast<quint8>(data.at(offset));
        quint32 b1 = static_cast<quint8>(data.at(offset + 1));
        quint32 b2 = static_cast<quint8>(data.at(offset + 2));
        quint32 b3 = static_cast<quint8>(data.at(offset + 3));
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    float readFloatLE(const QByteArray &data, int offset)
    {
        quint8 bytes[4];
        bytes[0] = static_cast<quint8>(data.at(offset));
        bytes[1] = static_cast<quint8>(data.at(offset + 1));
        bytes[2] = static_cast<quint8>(data.at(offset + 2));
        bytes[3] = static_cast<quint8>(data.at(offset + 3));
        float value;
        std::memcpy(&value, bytes, 4);
        return value;
    }
}
