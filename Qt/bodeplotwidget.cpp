#include "bodeplotwidget.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <algorithm>

BodePlotWidget::BodePlotWidget(QWidget *parent)
    : QWidget(parent),
      m_minFreq(10.0),
      m_maxFreq(100000.0),
      m_minDb(-80.0),
      m_maxDb(20.0)
{
    setMinimumSize(600,320);
    setSizePolicy(QSizePolicy::Expanding,
                      QSizePolicy::Expanding);
    setAutoFillBackground(true);
}

void BodePlotWidget::clear()
{
    m_points.clear();
    update();
}

void BodePlotWidget::addPoint(double frequencyHz, double gainDb)
{
    if (frequencyHz <= 0.0)
        return;

    m_points.append(QPointF(frequencyHz, gainDb));

    if (gainDb < m_minDb)
        m_minDb = qFloor(gainDb / 10.0) * 10.0;

    if (gainDb > m_maxDb)
        m_maxDb = qCeil(gainDb / 10.0) * 10.0;

    if (m_maxDb <= m_minDb)
        m_maxDb = m_minDb + 10.0;

    update();
}

void BodePlotWidget::setFrequencyRange(double minHz, double maxHz)
{
    if (minHz <= 0.0)
        minHz = 1.0;

    if (maxHz <= minHz)
        maxHz = minHz * 10.0;

    m_minFreq = minHz;
    m_maxFreq = maxHz;
    m_minDb = -80.0;
    m_maxDb = 20.0;
    update();
}

double BodePlotWidget::xToPixel(double freqHz, const QRect &plotRect) const
{
    double logMin = qLn(m_minFreq) / qLn(10.0);
    double logMax = qLn(m_maxFreq) / qLn(10.0);
    double logF = qLn(freqHz) / qLn(10.0);

    double t = (logF - logMin) / (logMax - logMin);
    return plotRect.left() + t * plotRect.width();
}

double BodePlotWidget::yToPixel(double db, const QRect &plotRect) const
{
    double t = (db - m_minDb) / (m_maxDb - m_minDb);
    return plotRect.bottom() - t * plotRect.height();
}

void BodePlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), Qt::white);

    QRect plotRect = rect().adjusted(70, 30, -25, -55);

    p.setPen(QPen(Qt::black, 1));
    p.drawRect(plotRect);

    p.drawText(QRect(0, 5, width(), 20), Qt::AlignCenter, "Bode Magnitude Plot");
    p.drawText(QRect(0, height() - 25, width(), 20), Qt::AlignCenter, "Frequency [Hz] - logarithmic");

    p.save();
    p.translate(15, height() / 2);
    p.rotate(-90);
    p.drawText(QRect(-100, 0, 200, 20), Qt::AlignCenter, "Gain [dB]");
    p.restore();

    p.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));

    int minDec = static_cast<int>(qFloor(qLn(m_minFreq) / qLn(10.0)));
    int maxDec = static_cast<int>(qCeil(qLn(m_maxFreq) / qLn(10.0)));

    for (int d = minDec; d <= maxDec; ++d)
    {
        double f = qPow(10.0, d);
        if (f < m_minFreq || f > m_maxFreq)
            continue;

        int x = static_cast<int>(xToPixel(f, plotRect));
        p.drawLine(x, plotRect.top(), x, plotRect.bottom());

        p.setPen(Qt::black);
        p.drawText(x - 30, plotRect.bottom() + 5, 60, 20,
                   Qt::AlignCenter, QString::number(f, 'g', 4));
        p.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
    }

    int yStart = static_cast<int>(qFloor(m_minDb / 10.0) * 10.0);
    int yStop = static_cast<int>(qCeil(m_maxDb / 10.0) * 10.0);

    for (int db = yStart; db <= yStop; db += 10)
    {
        if (db < m_minDb || db > m_maxDb)
            continue;

        int y = static_cast<int>(yToPixel(db, plotRect));
        p.drawLine(plotRect.left(), y, plotRect.right(), y);

        p.setPen(Qt::black);
        p.drawText(20, y - 10, 45, 20, Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(db));
        p.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
    }

    if (m_points.isEmpty())
        return;

    p.setPen(QPen(Qt::blue, 2));

    QPolygonF poly;
    for (int i = 0; i < m_points.size(); ++i)
    {
        QPointF pt = m_points.at(i);
        if (pt.x() <= 0.0)
            continue;

        double x = xToPixel(pt.x(), plotRect);
        double y = yToPixel(pt.y(), plotRect);
        poly << QPointF(x, y);
    }

    if (poly.size() >= 2)
        p.drawPolyline(poly);

    p.setBrush(Qt::blue);
    for (int i = 0; i < poly.size(); ++i)
        p.drawEllipse(poly.at(i), 3, 3);
}
