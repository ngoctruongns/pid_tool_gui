#include "plotviewer.h"
#include <QVBoxLayout>
#include <QDebug>

PlotViewer::PlotViewer(QWidget *parent) : QWidget(parent), m_timeCounter(0.0) {
    m_chart = new QChart();
    m_view = new QChartView(m_chart);
    m_view->setRenderHint(QPainter::Antialiasing);

    m_axisX = new QValueAxis();
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("Sample");
    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Value");

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chart->legend()->setVisible(true);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_view);
}

QLineSeries* PlotViewer::ensureSeriesForVariable(const QString &varName) {
    if (m_varNameToIndex.contains(varName)) {
        int idx = m_varNameToIndex[varName];
        return m_series[idx];
    }

    // Create new series for this variable
    QLineSeries *s = new QLineSeries();
    s->setName(varName);
    m_series.append(s);
    m_chart->addSeries(s);

    // Attach axes
    s->attachAxis(m_axisX);
    s->attachAxis(m_axisY);

    int idx = m_series.size() - 1;
    m_varNameToIndex[varName] = idx;

    return s;
}

void PlotViewer::appendSeriesPoint(int seriesIndex, qreal x, qreal y) {
    if (seriesIndex < 0 || seriesIndex >= m_series.size()) return;
    m_series[seriesIndex]->append(x, y);
}

void PlotViewer::appendSeriesPointByName(const QString &varName, qreal y) {
    QLineSeries *s = ensureSeriesForVariable(varName);
    s->append(m_timeCounter, y);
}

void PlotViewer::incrementSampleCounter() {
    m_timeCounter += 1.0;
    updateAxes();
}

void PlotViewer::updateAxes() {
    if (m_series.isEmpty()) return;

    qreal minX = 0, maxX = 1;
    qreal minY = 0, maxY = 1;
    bool hasData = false;

    for (QLineSeries *s : m_series) {
        if (s->count() > 0) {
            hasData = true;
            for (const QPointF &p : s->points()) {
                if (p.x() < minX) minX = p.x();
                if (p.x() > maxX) maxX = p.x();
                if (p.y() < minY) minY = p.y();
                if (p.y() > maxY) maxY = p.y();
            }
        }
    }

    if (hasData) {
        // Add some margin
        qreal xMargin = (maxX - minX) * 0.1;
        qreal yMargin = (maxY - minY) * 0.1;
        if (xMargin == 0) xMargin = 1;
        if (yMargin == 0) yMargin = 1;

        m_axisX->setRange(minX - xMargin, maxX + xMargin);
        m_axisY->setRange(minY - yMargin, maxY + yMargin);
    }
}

void PlotViewer::setVariableNames(const QStringList &names) {
    clear();
    m_varNameToIndex.clear();

    for (const QString &name : names) {
        ensureSeriesForVariable(name);
    }
    m_timeCounter = 0.0;
}

void PlotViewer::clear() {
    for (QLineSeries *s : m_series) s->clear();
    m_timeCounter = 0.0;
}
