# STM32 Bode Sweep - Qt Creator 4.6.2 / Qt 5.11.1 Compatible

This version avoids the Qt Charts module and uses a custom QWidget for plotting.
Required Qt modules:

- core
- gui
- widgets
- serialport

Open `STM32_BodeSweep_Qt5_11_Compatible.pro` in Qt Creator.

## Serial protocol

PC to STM32:

```text
SET_PARAMS <psc> <arr> <dds_frequency_hz>\n
```

Example:

```text
SET_PARAMS 71 49999 1000
```

STM32 to PC:

```text
OK <vin_rms> <vout_rms>\n
```

Example:

```text
OK 1.002 0.707
```

## Notes

- For each signal frequency, ADC trigger frequency is calculated as:

```text
Fs_adc = samples_per_period * f_signal
```

- Timer frequency approximation:

```text
Fs_real = TimerClock / ((PSC + 1) * (ARR + 1))
```

- Plot X value uses the real signal frequency:

```text
realSignalFreq = Fs_real / samples_per_period
```

- Gain is:

```text
Gain_dB = 20 * log10(Vout_RMS / Vin_RMS)
```
