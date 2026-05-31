#pragma once
#include <QString>

struct WeatherData {
    qint64 _dt{};
    qreal _temperature{};
    qreal _humidity{};
    qreal _pressure{};
    qreal _windSpeed{};
    qreal _deg{};
    qreal _clouds{};
    QString _cloudsStatus;
    QString _description;
    QString _icon;
};