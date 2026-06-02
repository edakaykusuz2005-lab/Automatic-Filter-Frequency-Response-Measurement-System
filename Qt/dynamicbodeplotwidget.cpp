#include "dynamicbodeplotwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>
#include <algorithm>

DynamicBodePlotWidget::DynamicBodePlotWidget(QWidget *parent)
    : QWidget(parent),
      m_minFreq(10.0),
      m_maxFreq(100000.0),
      m_minDb(-80.0),
      m_maxDb(20.0),
      m_autoFreqRange(true),
      m_hoverIndex(-1)
{
    setMinimumSize(650, 360);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void DynamicBodePlotWidget::clear()
{
    m_points.clear();
    m_hoverIndex = -1;
    m_minFreq = 10.0;
    m_maxFreq = 100000.0;
    m_minDb = -80.0;
    m_maxDb = 20.0;
    update();
}

void DynamicBodePlotWidget::addPoint(double frequencyHz, double gainDb)
{
    if (frequencyHz <= 0.0)
        return;

    m_points.append(QPointF(frequencyHz, gainDb));

    std::sort(m_points.begin(), m_points.end(),
              [](const QPointF &a, const QPointF &b) {
                  return a.x() < b.x();
              });

    updateRanges();
    update();
}

void DynamicBodePlotWidget::setFrequencyRange(double minHz, double maxHz)
{
    if (minHz <= 0.0)
        minHz = 1.0;

    if (maxHz <= minHz)
        maxHz = minHz * 10.0;

    m_autoFreqRange = false;
    m_minFreq = minHz;
    m_maxFreq = maxHz;

    update();
}

void DynamicBodePlotWidget::setAutoFrequencyRange(bool enabled)
{
    m_autoFreqRange = enabled;
    updateRanges();
    update();
}

QRect DynamicBodePlotWidget::plotRect() const
{
    return rect().adjusted(75, 30, -30, -60);
}

double DynamicBodePlotWidget::log10Value(double value) const
{
    return qLn(value) / qLn(10.0);
}

void DynamicBodePlotWidget::updateRanges()
{
    if (m_points.isEmpty())
        return;

    double minGain = m_points.first().y();
    double maxGain = m_points.first().y();

    double minFreq = m_points.first().x();
    double maxFreq = m_points.first().x();

    for (const QPointF &pt : m_points)
    {
        minGain = qMin(minGain, pt.y());
        maxGain = qMax(maxGain, pt.y());

        minFreq = qMin(minFreq, pt.x());
        maxFreq = qMax(maxFreq, pt.x());
    }

    m_minDb = qFloor((minGain - 5.0) / 10.0) * 10.0;
    m_maxDb = qCeil((maxGain + 5.0) / 10.0) * 10.0;

    if (m_maxDb <= m_minDb)
        m_maxDb = m_minDb + 10.0;

    if (m_autoFreqRange)
    {
        int minDec = static_cast<int>(qFloor(log10Value(minFreq)));
        int maxDec = static_cast<int>(qCeil(log10Value(maxFreq)));

        if (minDec == maxDec)
            maxDec++;

        m_minFreq = qPow(10.0, minDec);
        m_maxFreq = qPow(10.0, maxDec);
    }
}

double DynamicBodePlotWidget::xToPixel(double freqHz, const QRect &rect) const
{
    double logMin = log10Value(m_minFreq);
    double logMax = log10Value(m_maxFreq);
    double logF = log10Value(freqHz);

    double t = (logF - logMin) / (logMax - logMin);
    return rect.left() + t * rect.width();
}

double DynamicBodePlotWidget::yToPixel(double gainDb, const QRect &rect) const
{
    double t = (gainDb - m_minDb) / (m_maxDb - m_minDb);
    return rect.bottom() - t * rect.height();
}

QString DynamicBodePlotWidget::frequencyText(double hz) const
{
    if (hz >= 1000000.0)
        return QString::number(hz / 1000000.0, 'f', 2) + " MHz";

    if (hz >= 1000.0)
        return QString::number(hz / 1000.0, 'f', 2) + " kHz";

    return QString::number(hz, 'f', 2) + " Hz";
}

void DynamicBodePlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), Qt::white);

    QRect r = plotRect();

    p.setPen(QPen(Qt::black, 1));
    p.drawRect(r);

    p.drawText(QRect(0, 5, width(), 20),
               Qt::AlignCenter,
               "Dynamic Bode Magnitude Plot");

    p.drawText(QRect(0, height() - 28, width(), 22),
               Qt::AlignCenter,
               "Frequency [Hz] - logarithmic");

    p.save();
    p.translate(18, height() / 2);
    p.rotate(-90);
    p.drawText(QRect(-100, 0, 200, 20),
               Qt::AlignCenter,
               "Gain [dB]");
    p.restore();

    int minDec = static_cast<int>(qFloor(log10Value(m_minFreq)));
    int maxDec = static_cast<int>(qCeil(log10Value(m_maxFreq)));

    for (int d = minDec; d <= maxDec; ++d)
    {
        double f = qPow(10.0, d);

        if (f < m_minFreq || f > m_maxFreq)
            continue;

        int x = static_cast<int>(xToPixel(f, r));

        p.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
        p.drawLine(x, r.top(), x, r.bottom());

        p.setPen(Qt::black);
        p.drawText(x - 35,
                   r.bottom() + 6,
                   70,
                   20,
                   Qt::AlignCenter,
                   frequencyText(f));
    }

    int yStart = static_cast<int>(qFloor(m_minDb / 10.0) * 10.0);
    int yStop = static_cast<int>(qCeil(m_maxDb / 10.0) * 10.0);

    for (int db = yStart; db <= yStop; db += 10)
    {
        int y = static_cast<int>(yToPixel(db, r));

        p.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
        p.drawLine(r.left(), y, r.right(), y);

        p.setPen(Qt::black);
        p.drawText(20,
                   y - 10,
                   50,
                   20,
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(db));
    }

    if (m_points.isEmpty())
        return;

    QPolygonF poly;

    for (const QPointF &pt : m_points)
    {
        if (pt.x() < m_minFreq || pt.x() > m_maxFreq)
            continue;

        poly << QPointF(xToPixel(pt.x(), r),
                        yToPixel(pt.y(), r));
    }

    p.setPen(QPen(Qt::blue, 2));

    if (poly.size() >= 2)
        p.drawPolyline(poly);

    for (int i = 0; i < poly.size(); ++i)
    {
        p.setBrush(i == m_hoverIndex ? Qt::red : Qt::blue);
        p.setPen(Qt::NoPen);
        p.drawEllipse(poly.at(i), i == m_hoverIndex ? 5 : 3, i == m_hoverIndex ? 5 : 3);
    }
}

int DynamicBodePlotWidget::pointAt(const QPoint &pos) const
{
    QRect r = plotRect();

    for (int i = 0; i < m_points.size(); ++i)
    {
        QPointF pt = m_points.at(i);

        QPointF screenPt(xToPixel(pt.x(), r),
                         yToPixel(pt.y(), r));

        if (QLineF(pos, screenPt).length() <= 8.0)
            return i;
    }

    return -1;
}

void DynamicBodePlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    int index = pointAt(event->pos());

    if (index != m_hoverIndex)
    {
        m_hoverIndex = index;
        update();
    }

    if (index >= 0)
    {
        QPointF pt = m_points.at(index);

        QString text =
            "Frequency: " + frequencyText(pt.x()) +
            "\nGain: " + QString::number(pt.y(), 'f', 2) + " dB";

        QToolTip::showText(event->globalPos(), text, this);
    }
    else
    {
        QToolTip::hideText();
    }
}

void DynamicBodePlotWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    m_hoverIndex = -1;
    QToolTip::hideText();
    update();
}
