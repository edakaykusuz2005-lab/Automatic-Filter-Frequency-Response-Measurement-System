#ifndef SWEEPCONTROLLER_H
#define SWEEPCONTROLLER_H

#include <QObject>
#include <QVector>
#include <QTimer>

#include "sweeppoint.h"
#include "serialmanager.h"

struct MeasurementPoint
{
    double frequencyHz;
    double vinRms;
    double voutRms;
    double gain;
    double gainDb;

    MeasurementPoint()
        : frequencyHz(0.0), vinRms(0.0), voutRms(0.0), gain(0.0), gainDb(0.0)
    {
    }
};

class SweepController : public QObject
{
    Q_OBJECT

public:
    explicit SweepController(SerialManager *serial, QObject *parent = 0);

    void start(double fStartHz, double fStopHz, double fStepHz,
               double timerClockHz, int samplesPerPeriod);
    void stop();
    bool isRunning() const;

signals:
    void sweepStarted(int totalPoints);
    void sweepStopped();
    void sweepFinished();
    void progressChanged(int current, int total);
    void measurementReady(const MeasurementPoint &measurement);
    void message(const QString &text);
    void errorOccurred(const QString &text);

private slots:
    void onAckReceived();
    void onNackReceived();
    void onMeasurementReceived(float vinRms, float voutRms);
    void onTimeout();

private:
    enum State { Idle, WaitingSetAck, WaitingMeasurement };

    SerialManager *m_serial;
    QVector<SweepPoint> m_points;
    int m_index;
    bool m_running;
    State m_state;
    QTimer m_timeoutTimer;

    void sendCurrentPoint();
    void finishSweep();
};

#endif
