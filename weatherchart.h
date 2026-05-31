#pragma once
#include <QChartView>
#include <QChart>
#include <QSplineSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include "forecastdata.h"

class WeatherChart : public QChartView {
    Q_OBJECT
public:
    explicit WeatherChart(QWidget* parent = nullptr);
    ~WeatherChart() = default;

    void UpdateData(const QList<ForecastData>& forecastList);

private:
    QSplineSeries* _maxSeries;
    QSplineSeries* _minSeries;
    QBarCategoryAxis* _axisX;
    QValueAxis* _axisY;
};