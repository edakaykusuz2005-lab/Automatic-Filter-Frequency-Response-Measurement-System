#include "timercalculator.h"

#include <cmath>
#include <limits>

SweepPoint TimerCalculator::calculate(double signalFreqHz,
                                      double timerClockHz,
                                      int samplesPerPeriod)
{
    SweepPoint best;
    best.requestedFreqHz = signalFreqHz;
    best.targetSampleFreqHz = signalFreqHz * samplesPerPeriod;
    best.ddsFreqHz = static_cast<quint32>(std::llround(signalFreqHz));

    double bestError = std::numeric_limits<double>::max();
    const double targetFs = best.targetSampleFreqHz;

    for (quint32 psc = 0; psc <= 65535; ++psc)
    {
        double arrFloat = timerClockHz / (targetFs * static_cast<double>(psc + 1)) - 1.0;
        long arrLong = std::lround(arrFloat);

        if (arrLong < 0 || arrLong > 65535)
            continue;

        double realFs = timerClockHz /
                (static_cast<double>(psc + 1) * static_cast<double>(arrLong + 1));

        double error = std::fabs(realFs - targetFs);

        if (error < bestError)
        {
            bestError = error;
            best.psc = static_cast<quint16>(psc);
            best.arr = static_cast<quint16>(arrLong);
            best.realSampleFreqHz = realFs;
            best.realSignalFreqHz = realFs / static_cast<double>(samplesPerPeriod);
            best.timerErrorHz = error;
            best.timerErrorPercent = targetFs > 0.0 ? 100.0 * error / targetFs : 0.0;
        }

        if (bestError == 0.0)
            break;
    }

    return best;
}
