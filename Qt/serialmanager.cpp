#include "serialmanager.h"
#include "protocol.h"

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&m_serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(onSerialError(QSerialPort::SerialPortError)));
}

QStringList SerialManager::availablePorts() const
{
    QStringList ports;
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    for (int i = 0; i < infos.size(); ++i)
        ports << infos.at(i).portName();
    return ports;
}

bool SerialManager::openPort(const QString &portName, qint32 baudRate)
{
    if (m_serial.isOpen())
        m_serial.close();

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite))
    {
        emit errorOccurred(m_serial.errorString());
        return false;
    }

    m_rxBuffer.clear();
    emit connected();
    return true;
}

void SerialManager::closePort()
{
    if (m_serial.isOpen())
        m_serial.close();
    emit disconnected();
}

bool SerialManager::isOpen() const
{
    return m_serial.isOpen();
}

void SerialManager::sendParameters(quint16 psc, quint16 arr, quint32 ddsFreqHz)
{
    sendSetParameters(psc, arr, ddsFreqHz);
}

void SerialManager::sendSetParameters(quint16 psc, quint16 arr, quint32 ddsFreqHz)
{
    QByteArray payload;
    Protocol::appendUInt16LE(payload, psc);
    Protocol::appendUInt16LE(payload, arr);
    Protocol::appendUInt32LE(payload, ddsFreqHz);
    sendFrame(Protocol::MSG_SET_PARAMETERS, payload);
}

void SerialManager::sendAck()
{
    sendFrame(Protocol::MSG_ACK, QByteArray());
}

void SerialManager::sendNack()
{
    sendFrame(Protocol::MSG_NACK, QByteArray());
}

void SerialManager::sendFrame(quint8 type, const QByteArray &payload)
{
    if (!m_serial.isOpen())
    {
        emit errorOccurred("Serial port is not open.");
        return;
    }

    QByteArray frame = Protocol::buildFrame(type, payload);
    emit rawLineReceived(QString("TX HEX: ") + QString::fromLatin1(frame.toHex(' ').toUpper()));
    m_serial.write(frame);
    m_serial.flush();
}

void SerialManager::onReadyRead()
{
    QByteArray incoming = m_serial.readAll();
    if (!incoming.isEmpty())
        emit rawLineReceived(QString("RX HEX: ") + QString::fromLatin1(incoming.toHex(' ').toUpper()));
    m_rxBuffer.append(incoming);
    parseBuffer();
}

void SerialManager::parseBuffer()
{
    while (m_rxBuffer.size() >= 6)
    {
        int headerIndex = -1;
        for (int i = 0; i < m_rxBuffer.size() - 1; ++i)
        {
            quint8 b0 = static_cast<quint8>(m_rxBuffer.at(i));
            quint8 b1 = static_cast<quint8>(m_rxBuffer.at(i + 1));
            if (b0 == Protocol::HEADER_1 && b1 == Protocol::HEADER_2)
            {
                headerIndex = i;
                break;
            }
        }

        if (headerIndex < 0)
        {
            m_rxBuffer.clear();
            return;
        }

        if (headerIndex > 0)
            m_rxBuffer.remove(0, headerIndex);

        if (m_rxBuffer.size() < 6)
            return;

        quint8 type = static_cast<quint8>(m_rxBuffer.at(2));
        quint16 length = static_cast<quint8>(m_rxBuffer.at(3)) |
                         (static_cast<quint16>(static_cast<quint8>(m_rxBuffer.at(4))) << 8);
        int frameSize = 2 + 1 + 2 + length + 1;

        if (m_rxBuffer.size() < frameSize)
            return;

        QByteArray payload = m_rxBuffer.mid(5, length);
        quint8 rxChecksum = static_cast<quint8>(m_rxBuffer.at(frameSize - 1));
        quint8 calcChecksum = Protocol::checksum(type, length, payload);
        m_rxBuffer.remove(0, frameSize);

        if (rxChecksum != calcChecksum)
        {
            emit errorOccurred("Checksum error. NACK sent.");
            sendNack();
            continue;
        }

        processFrame(type, payload);
    }
}

void SerialManager::processFrame(quint8 type, const QByteArray &payload)
{
    if (type == Protocol::MSG_ACK)
    {
        if (payload.size() == 0)
            emit ackReceived();
        else
        {
            emit errorOccurred("ACK payload size must be 0.");
            sendNack();
        }
        return;
    }

    if (type == Protocol::MSG_NACK)
    {
        emit nackReceived();
        return;
    }

    if (type == Protocol::MSG_GET_PARAMETERS)
    {
        if (payload.size() != 8)
        {
            emit errorOccurred("GET_PARAMETERS payload size must be 8.");
            sendNack();
            return;
        }

        float vin = Protocol::readFloatLE(payload, 0);
        float vout = Protocol::readFloatLE(payload, 4);
        emit measurementReceived(vin, vout);
        return;
    }

    emit errorOccurred(QString("Unknown message type: 0x%1").arg(type, 2, 16, QLatin1Char('0')).toUpper());
    sendNack();
}

void SerialManager::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError)
        return;
    emit errorOccurred(m_serial.errorString());
}
