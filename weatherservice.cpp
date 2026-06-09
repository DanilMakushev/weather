#include "weatherservice.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QUrl>
#include <QLocale>
#include <QUrlQuery>
#include <QMap>

WeatherService::WeatherService(QObject *parent)
    : QObject(parent)
    , _networkManager(new QNetworkAccessManager(this))
{}


// =========================================================================
// АВТООПРЕДЕЛЕНИЕ ГОРОДА
// =========================================================================


void WeatherService::DetectCityRegionAuto() {
    QNetworkReply* reply = _networkManager->get(QNetworkRequest(QUrl(_detectCityUrl)));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        OnDetectCityReplyFinished(reply);
    });
}


void WeatherService::OnDetectCityReplyFinished(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Ошибка при автоопределении города:" << reply->errorString();
        emit ErrorOccurred();
        return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();

    QString countryCode = obj["countryCode"].toString();
    if (countryCode != "RU") {
        QString countryName = obj["country"].toString();
        qDebug() << "Автоматически был определен город не из России: " << countryName;
        return;
    }

    emit CityRegionAutoReady(obj["city"].toString(), obj["regionName"].toString());
}


// =========================================================================
// ЗАПРОСЫ ПОГОДЫ
// =========================================================================


void WeatherService::RequestData() {
    FetchWeather(_city, _region);
}


void WeatherService::FetchWeather(const QString& city, const QString& region) {
    QNetworkReply* reply = _networkManager->get(QNetworkRequest(BuildRequestUrl(city, region, "weather")));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        OnWeatherReplyFinished(reply);
    });
}


void WeatherService::OnWeatherReplyFinished(QNetworkReply* reply) {
    reply->deleteLater();

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "weather HTTP" << statusCode << "для" << _city;

    // Повтор запроса без региона при 404
    if (statusCode == 404 && !_region.isEmpty()) {
        FetchWeather(_city, "");
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Ошибка сети (weather):" << reply->errorString();
        emit ErrorOccurred();
        return;
    }

    const QJsonObject currentWeatherObj = QJsonDocument::fromJson(reply->readAll()).object();
    FetchForecast(currentWeatherObj, _city, _region);
}


void WeatherService::FetchForecast(const QJsonObject& currentWeatherObj, const QString& city, const QString& region) {
    QNetworkReply* reply = _networkManager->get(QNetworkRequest(BuildRequestUrl(city, region, "forecast")));
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentWeatherObj]() {
        OnForecastReplyFinished(currentWeatherObj, reply);
    });
}


void WeatherService::OnForecastReplyFinished(const QJsonObject& currentWeatherObj, QNetworkReply* reply) {
    reply->deleteLater();

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "forecast HTTP" << statusCode << "для" << _city;

    // Повтор запроса без региона при 404
    if (statusCode == 404 && !_region.isEmpty()) {
        FetchForecast(currentWeatherObj, _city, "");
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Ошибка сети (forecast):" << reply->errorString();
        emit ErrorOccurred();
        return;
    }

    const QJsonObject forecastObj   = QJsonDocument::fromJson(reply->readAll()).object();
    const QJsonArray  forecastArray = forecastObj["list"].toArray();

    // Список для главного экрана: текущая погода + 7 ближайших слотов
    QList<WeatherData> weatherTodayList;
    weatherTodayList.append(ParseWeatherObject(currentWeatherObj));
    const int todayLimit = qMin(7, forecastArray.size());
    for (int i = 0; i < todayLimit; ++i)
        weatherTodayList.append(ParseWeatherObject(forecastArray[i].toObject()));
    emit WeatherTodayListReady(weatherTodayList);

    // Полный список слотов для 5-дневного прогноза
    QList<WeatherData> forecastList;
    forecastList.reserve(forecastArray.size());
    for (const QJsonValue& value : forecastArray)
        forecastList.append(ParseWeatherObject(value.toObject()));
    emit ForecastListReady(forecastList);
}


// =========================================================================
// ВАЛИДАЦИЯ API-КЛЮЧА
// =========================================================================


void WeatherService::ValidateApiKey(const QString& key) {
    const QString testUrl = QString("http://ru.api.openweathermap.org/data/2.5/weather?q=Moscow&appid=%1").arg(key);
    QNetworkReply* reply  = _networkManager->get(QNetworkRequest(QUrl(testUrl)));

    QTimer* timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Успешный запрос
            emit ApiKeyValidationResult(true, "");
        }
        else if (reply->error() == QNetworkReply::OperationCanceledError) {
            // Сработал таймаут
            emit ApiKeyValidationResult(false, "Время ответа истекло, повторите позже");
        }
        else {
            // Любая другая ошибка сервера
            emit ApiKeyValidationResult(false, "Введен некорректный API ключ");
        }

        reply->deleteLater();
    });

    // Логика работы таймера таймаута
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });

    timeoutTimer->start(5000);
}


// =========================================================================
// АГРЕГАЦИЯ ПРОГНОЗА ПО ДНЯМ
// =========================================================================


QList<ForecastData> WeatherService::AggregateForecastByDay(const QList<WeatherData>& list) {
    const QLocale ruLocale(QLocale::Russian);
    QMap<QString, QList<WeatherData>> groupedByDate;

    for (const WeatherData& item : list) {
        // Пропускаем слоты с невалидным временем
        if (item._dt <= 0) continue;
        const QString dateKey = QDateTime::fromSecsSinceEpoch(item._dt)
                                    .date().toString("yyyy-MM-dd");
        groupedByDate[dateKey].append(item);
    }

    QList<ForecastData> result;
    for (auto it = groupedByDate.cbegin(); it != groupedByDate.cend(); ++it) {
        const QList<WeatherData>& daySlots = it.value();

        ForecastData day;
        day._maxTemp = -999.0;
        day._minTemp = 999.0;

        const QDateTime firstDt = QDateTime::fromSecsSinceEpoch(daySlots.first()._dt);
        day._date = ruLocale.toString(firstDt.date(), "ddd") + "\n" + firstDt.date().toString("dd-MM");

        // Ищем слот, ближайший к 12:00 — он будет «представителем» дня
        int bestDistance = 24;
        WeatherData representative = daySlots.first();
        for (const WeatherData& slot : daySlots) {
            if (slot._temperature > day._maxTemp) day._maxTemp = slot._temperature;
            if (slot._temperature < day._minTemp) day._minTemp = slot._temperature;

            const int distance = qAbs(QDateTime::fromSecsSinceEpoch(slot._dt).time().hour() - 13);
            if (distance < bestDistance) {
                bestDistance  = distance;
                representative = slot;
            }
        }

        day._description = representative._description;
        day._icon = representative._icon;
        result.append(day);
    }

    // API возвращает до 6 дней (включая текущий) — обрезаем до 5
    while (result.size() > 5)
        result.removeLast();

    return result;
}


// =========================================================================
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// =========================================================================


QUrl WeatherService::BuildRequestUrl(const QString& city, const QString& region, const QString& endpoint) const {
    QUrl url(_openWeatherMapUrl + endpoint);
    QUrlQuery query;

    QString location = city;
    if (!region.isEmpty()) location += "," + region;
    location += ",RU";

    query.addQueryItem("q", location);
    query.addQueryItem("appid", _apiKey);
    query.addQueryItem("units", "metric");
    query.addQueryItem("lang", "ru");
    url.setQuery(query);

    qDebug() << "URL запроса:" << url.toString();
    return url;
}


WeatherData WeatherService::ParseWeatherObject(const QJsonObject& obj) const {
    WeatherData weather;

    const QJsonObject mainObj = obj["main"].toObject();
    const QJsonObject windObj = obj["wind"].toObject();
    const QJsonObject cloudsObj = obj["clouds"].toObject();
    const QJsonArray  weatherArr = obj["weather"].toArray();

    weather._dt = obj["dt"].toVariant().toLongLong();
    weather._temperature = mainObj["temp"].toDouble();
    weather._humidity = mainObj["humidity"].toInt();
    weather._pressure = mainObj["pressure"].toInt();
    weather._windSpeed = windObj["speed"].toDouble();
    weather._deg  = windObj["deg"].toDouble();
    weather._clouds = cloudsObj["all"].toDouble();

    if (!weatherArr.isEmpty()) {
        const QJsonObject weatherObj = weatherArr[0].toObject();
        weather._description = weatherObj["description"].toString();
        weather._icon  = weatherObj["icon"].toString();
    } else {
        weather._description = "нет описания";
        weather._icon = "01d";
    }

    weather._cloudsStatus = ResolveCloudsStatus(weather._clouds);
    return weather;
}


QString WeatherService::ResolveCloudsStatus(double cloudPercent) const {
    if (cloudPercent <= 10) return "ясно";
    if (cloudPercent <= 50) return "рассеянные облака";
    if (cloudPercent <= 85) return "облачно";
    return "сплошная облачность";
}


bool WeatherService::IsInternetAvailable() {
    QTcpSocket socket;
    socket.connectToHost("ya.ru", 80);
    return socket.waitForConnected(1000);
}