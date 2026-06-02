#ifndef TIMERCALCULATOR_H
#define TIMERCALCULATOR_H

#include "sweeppoint.h"

class TimerCalculator
{
public:
    static SweepPoint calculate(double signalFreqHz,
                                double timerClockHz,
                                int samplesPerPeriod);
};

#endif // TIMERCALCULATOR_H
