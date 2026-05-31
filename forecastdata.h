#pragma once
#include <QString>

struct ForecastData {
    QString _date;
    QString _description;
    QString _icon;
    double  _maxTemp{};
    double  _minTemp{};
};