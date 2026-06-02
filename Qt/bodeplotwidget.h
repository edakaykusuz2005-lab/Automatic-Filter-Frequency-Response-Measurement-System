#ifndef BODEPLOTWIDGET_H
#define BODEPLOTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class BodePlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BodePlotWidget(QWidget *parent = 0);

    void clear();
    void addPoint(double frequencyHz, double gainDb);
    void setFrequencyRange(double minHz, double maxHz);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVector<QPointF> m_points;

    double m_minFreq;
    double m_maxFreq;
    double m_minDb;
    double m_maxDb;

    double xToPixel(double freqHz, const QRect &plotRect) const;
    double yToPixel(double db, const QRect &plotRect) const;
};

#endif // BODEPLOTWIDGET_H
