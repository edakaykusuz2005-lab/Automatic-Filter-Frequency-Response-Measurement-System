#include "sweepcontroller.h"
#include "timercalculator.h"
#include <cmath>

SweepController::SweepController(SerialManager *serial, QObject *parent)
    : QObject(parent), m_serial(serial), m_index(0), m_running(false), m_state(Idle)
{
    connect(m_serial, SIGNAL(ackReceived()), this, SLOT(onAckReceived()));
    connect(m_serial, SIGNAL(nackReceived()), this, SLOT(onNackReceived()));
    connect(m_serial, SIGNAL(measurementReceived(float,float)), this, SLOT(onMeasurementReceived(float,float)));

    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.setInterval(180000);
    connect(&m_timeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void SweepController::start(double fStartHz, double fStopHz, double fStepHz,
                            double timerClockHz, int samplesPerPeriod)
{
    if (!m_serial || !m_serial->isOpen())
    {
        emit errorOccurred("Sweep cannot start: serial port is not open.");
        return;
    }

    if (fStartHz <= 0.0 || fStopHz <= 0.0 || fStepHz <= 0.0 || fStopHz < fStartHz)
    {
        emit errorOccurred("Frequency range is invalid.");
        return;
    }

    m_points.clear();
    for (double f = fStartHz; f <= fStopHz + 1e-9; f += fStepHz)
        m_points.append(TimerCalculator::calculate(f, timerClockHz, samplesPerPeriod));

    if (m_points.isEmpty())
    {
        emit errorOccurred("No sweep point generated.");
        return;
    }

    m_index = 0;
    m_running = true;
    m_state = Idle;
    emit sweepStarted(m_points.size());
    emit progressChanged(0, m_points.size());
    emit message("Sweep started.");
    sendCurrentPoint();
}

void SweepController::stop()
{
    if (!m_running)
        return;
    m_timeoutTimer.stop();
    m_running = false;
    m_state = Idle;
    emit sweepStopped();
    emit message("Sweep stopped.");
}

bool SweepController::isRunning() const
{
    return m_running;
}

void SweepController::sendCurrentPoint()
{
    if (!m_running)
        return;

    if (m_index >= m_points.size())
    {
        finishSweep();
        return;
    }

    const SweepPoint &p = m_points.at(m_index);
    emit message(QString("SET_PARAMETERS: f=%1 Hz, PSC=%2, ARR=%3, DDS=%4")
                 .arg(p.realSignalFreqHz, 0, 'f', 3)
                 .arg(p.psc).arg(p.arr).arg(p.ddsFreqHz));

    m_serial->sendSetParameters(p.psc, p.arr, p.ddsFreqHz);
    m_state = WaitingSetAck;
    m_timeoutTimer.start();
}

void SweepController::onAckReceived()
{
    if (!m_running)
        return;

    if (m_state == WaitingSetAck)
    {
        emit message("ACK received for SET_PARAMETERS. Waiting GET_PARAMETERS...");
        m_state = WaitingMeasurement;
        m_timeoutTimer.start();
        return;
    }

    emit message("ACK received.");
}

void SweepController::onNackReceived()
{
    if (!m_running)
        return;
    emit errorOccurred("NACK received from STM32.");
    stop();
}

void SweepController::onMeasurementReceived(float vinRms, float voutRms)
{
    if (!m_running || m_state != WaitingMeasurement || m_index >= m_points.size())
        return;

    m_timeoutTimer.stop();

    const SweepPoint &p = m_points.at(m_index);
    MeasurementPoint m;
    m.frequencyHz = p.ddsFreqHz;
    m.vinRms = static_cast<double>(vinRms);
    m.voutRms = static_cast<double>(voutRms);

    if (m.vinRms > 0.0 && m.voutRms > 0.0)
    {
        m.gain = m.voutRms / m.vinRms;
        m.gainDb = 20.0 * std::log10(m.gain);
    }
    else
    {
        m.gain = 0.0;
        m.gainDb = -120.0;
    }

    emit message(QString("GET_PARAMETERS: Vin=%1, Vout=%2")
                 .arg(m.vinRms, 0, 'f', 6)
                 .arg(m.voutRms, 0, 'f', 6));
    emit measurementReady(m);

    m_serial->sendAck();
    emit message("ACK sent for GET_PARAMETERS.");

    ++m_index;
    emit progressChanged(m_index, m_points.size());
    m_state = Idle;
    sendCurrentPoint();
}

void SweepController::onTimeout()
{
    if (!m_running)
        return;
    if (m_state == WaitingSetAck)
        emit errorOccurred("Timeout while waiting ACK for SET_PARAMETERS.");
    else if (m_state == WaitingMeasurement)
        emit errorOccurred("Timeout while waiting GET_PARAMETERS.");
    else
        emit errorOccurred("Timeout.");
    stop();
}

void SweepController::finishSweep()
{
    m_timeoutTimer.stop();
    m_running = false;
    m_state = Idle;
    emit sweepFinished();
    emit message("Sweep finished.");
}
