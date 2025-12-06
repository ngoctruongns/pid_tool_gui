#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QMap>

QT_CHARTS_USE_NAMESPACE

class PlotViewer : public QWidget {
    Q_OBJECT
public:
    explicit PlotViewer(QWidget *parent = nullptr);
    void appendSeriesPoint(int seriesIndex, qreal x, qreal y);
    void appendSeriesPointByName(const QString &varName, qreal y);
    void clear();
    void setVariableNames(const QStringList &names);
    void incrementSampleCounter();

private:
    QChart *m_chart;
    QChartView *m_view;
    QVector<QLineSeries*> m_series;
    QMap<QString, int> m_varNameToIndex;
    qreal m_timeCounter;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

    QLineSeries* ensureSeriesForVariable(const QString &varName);
    void updateAxes();
};
