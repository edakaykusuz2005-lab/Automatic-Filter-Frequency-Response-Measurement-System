#ifndef DYNAMICBODEPLOTWIDGET_H
#define DYNAMICBODEPLOTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class DynamicBodePlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DynamicBodePlotWidget(QWidget *parent = 0);

    void clear();
    void addPoint(double frequencyHz, double gainDb);

    void setFrequencyRange(double minHz, double maxHz);
    void setAutoFrequencyRange(bool enabled);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:
    QVector<QPointF> m_points;

    double m_minFreq;
    double m_maxFreq;
    double m_minDb;
    double m_maxDb;

    bool m_autoFreqRange;
    int m_hoverIndex;

    QRect plotRect() const;

    void updateRanges();

    double log10Value(double value) const;
    double xToPixel(double freqHz, const QRect &rect) const;
    double yToPixel(double gainDb, const QRect &rect) const;

    int pointAt(const QPoint &pos) const;
    QString frequencyText(double hz) const;
};
#endif // DYNAMICBODEPLOTWIDGET_H
