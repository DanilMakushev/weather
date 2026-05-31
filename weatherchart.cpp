#include "weatherchart.h"
#include <cmath>

WeatherChart::WeatherChart(QWidget* parent): QChartView(new QChart(), parent) {
    // убираем эффект "ступенек"
    this->setRenderHint(QPainter::Antialiasing);

    _maxSeries = new QSplineSeries(this);
    _minSeries = new QSplineSeries(this);

    QPen maxPen(QColor(0xF5, 0xB4, 0x11));
    maxPen.setWidth(3);
    _maxSeries->setPen(maxPen);

    QPen minPen(QColor(0x0A, 0x69, 0xF7));
    minPen.setWidth(3);
    _minSeries->setPen(minPen);

    _maxSeries->setPointsVisible(true);
    _minSeries->setPointsVisible(true);

    _maxSeries->setMarkerSize(4);
    _minSeries->setMarkerSize(4);

    chart()->addSeries(_maxSeries);
    chart()->addSeries(_minSeries);
    chart()->setAnimationOptions(QChart::SeriesAnimations);

    chart()->setBackgroundVisible(false); // Убираем фон графика
    chart()->legend()->hide();            // Прячем легенду

    // Убираем внутренние отступы
    chart()->setMargins(QMargins(0, 10, 0, 10));

    _axisX = new QBarCategoryAxis(this);
    _axisY = new QValueAxis(this);

    _axisX->setVisible(false);
    _axisY->setVisible(false);
    _axisX->setGridLineVisible(false);
    _axisY->setGridLineVisible(false);

    chart()->addAxis(_axisX, Qt::AlignBottom);
    chart()->addAxis(_axisY, Qt::AlignLeft);

    _maxSeries->attachAxis(_axisX);
    _maxSeries->attachAxis(_axisY);
    _minSeries->attachAxis(_axisX);
    _minSeries->attachAxis(_axisY);

    setStyleSheet("background: transparent; border: none;");
}

void WeatherChart::UpdateData(const QList<ForecastData>& forecastList) {
    // Очищаем старые данные
    _maxSeries->clear();
    _minSeries->clear();
    _axisX->clear();

    QStringList categories;
    qreal globalMin = 100;
    qreal globalMax = -100;

    for (int i = 0; i < forecastList.size(); ++i) {
        const auto& day = forecastList[i];

        _maxSeries->append(i, day._maxTemp);
        _minSeries->append(i, day._minTemp);
        categories << day._date;

        if (day._minTemp < globalMin) globalMin = day._minTemp;
        if (day._maxTemp > globalMax) globalMax = day._maxTemp;
    }

    // Обновляем подписи на оси X и диапазон оси Y
    _axisX->append(categories);
    _axisY->setRange(std::floor(globalMin) - 2, std::ceil(globalMax) + 2);
}