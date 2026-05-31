#pragma once
#include <QObject>
#include <QtNetwork>
#include <QUrl>
#include <QJsonObject>
#include <QTcpSocket>
#include "weatherdata.h"
#include "forecastdata.h"



class WeatherService : public QObject {
    Q_OBJECT

public:
    explicit WeatherService(QObject *parent = nullptr);

    // Запросы данных
    void RequestData();
    void DetectCityRegionAuto();
    void ValidateApiKey(const QString& apiKey);

    // Обработка данных
    QList<ForecastData> AggregateForecastByDay(const QList<WeatherData>& list);

    // Утилиты
    bool IsInternetAvailable();

    // Геттеры / сеттеры
    void SetCity(const QString& city) { _city   = city; }
    void SetRegion(const QString& region) { _region = region; }
    void SetApiKey(const QString& apiKey) { _apiKey = apiKey; }
    QString GetCity()   const { return _city;   }
    QString GetRegion() const { return _region; }
    QString GetApiKey() const { return _apiKey; }
    bool IsApiKeyEmpty() const { return _apiKey.isEmpty(); }

signals:
    void WeatherTodayListReady(const QList<WeatherData>& weatherTodayList);
    void ForecastListReady(const QList<WeatherData>& forecastList);
    void CityRegionAutoReady(const QString& city, const QString& region);
    void ApiKeyValidationResult(bool success);
    void ErrorOccurred();

private:
    // Сетевые запросы
    void FetchWeather(const QString& city, const QString& region);
    void FetchForecast(const QJsonObject& currentWeatherObj, const QString& city, const QString& region);

    // Слоты для обработки ответов
    void OnWeatherReplyFinished(QNetworkReply* reply);
    void OnForecastReplyFinished(const QJsonObject& currentWeatherObj, QNetworkReply* reply);
    void OnDetectCityReplyFinished(QNetworkReply* reply);

    // Вспомогательные методы
    QUrl BuildRequestUrl(const QString& city, const QString& region, const QString& endpoint) const;
    WeatherData ParseWeatherObject(const QJsonObject& obj) const;
    QString ResolveCloudsStatus(double cloudPercent) const;

    QNetworkAccessManager* _networkManager;
    QString _city;
    QString _region;
    QString _apiKey;

    const QString _detectCityUrl = "http://ip-api.com/json/?lang=ru";
    const QString _openWeatherMapUrl = "http://ru.api.openweathermap.org/data/2.5/";
};