#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = 0);

    QStringList availablePorts() const;
    bool openPort(const QString &portName, qint32 baudRate);
    void closePort();
    bool isOpen() const;

    void sendParameters(quint16 psc, quint16 arr, quint32 ddsFreqHz); // compatibility name
    void sendSetParameters(quint16 psc, quint16 arr, quint32 ddsFreqHz);
    void sendAck();
    void sendNack();

signals:
    void connected();
    void disconnected();
    void ackReceived();
    void nackReceived();
    void measurementReceived(float vinRms, float voutRms);
    void rawLineReceived(const QString &line); // hex text for old MainWindow log
    void errorOccurred(const QString &message);

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_serial;
    QByteArray m_rxBuffer;

    void parseBuffer();
    void processFrame(quint8 type, const QByteArray &payload);
    void sendFrame(quint8 type, const QByteArray &payload);
};

#endif
