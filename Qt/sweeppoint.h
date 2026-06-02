#ifndef SWEEPPOINT_H
#define SWEEPPOINT_H

#include <QtGlobal>

struct SweepPoint
{
    double requestedFreqHz;
    double targetSampleFreqHz;
    double realSampleFreqHz;
    double realSignalFreqHz;

    quint16 psc;
    quint16 arr;
    quint32 ddsFreqHz;

    double timerErrorHz;
    double timerErrorPercent;

    SweepPoint()
        : requestedFreqHz(0.0),
          targetSampleFreqHz(0.0),
          realSampleFreqHz(0.0),
          realSignalFreqHz(0.0),
          psc(0),
          arr(0),
          ddsFreqHz(0),
          timerErrorHz(0.0),
          timerErrorPercent(0.0)
    {
    }
};

#endif // SWEEPPOINT_H
