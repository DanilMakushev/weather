#include "filemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

FileManager::FileManager(QObject *parent) : QObject(parent) {}


// =========================================================================
// ЗАПИСЬ
// =========================================================================


QJsonArray FileManager::SerializeWeatherList(const QList<WeatherData>& list) {
    QJsonArray array;
    for (const WeatherData& data : list) {
        QJsonObject item;
        item["dt"] = data._dt;
        item["temperature"] = data._temperature;
        item["humidity"] = data._humidity;
        item["pressure"]  = data._pressure;
        item["windSpeed"] = data._windSpeed;
        item["deg"]  = data._deg;
        item["clouds"] = data._clouds;
        item["cloudsStatus"] = data._cloudsStatus;
        item["description"] = data._description;
        item["icon"] = data._icon;
        array.append(item);
    }
    return array;
}


void FileManager::WriteJsonToFile(const QJsonObject& data, const QString& fileName) {
    QFile file(fileName + ".json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Не удалось открыть файл для записи:" << file.errorString();
        return;
    }
    file.write(QJsonDocument(data).toJson());
    file.close();
    qDebug() << "Файл обновлён:" << fileName;
}


void FileManager::WriteWeatherToFile(const QList<WeatherData>& list, const QString& fileName) {
    QJsonObject obj;
    obj["list"] = SerializeWeatherList(list);
    WriteJsonToFile(obj, fileName);
}


void FileManager::WriteMainToFile(const QString& city, const QString& region, const QString& apiKey, const QString& theme) {
    QJsonObject obj;
    obj["city"] = city;
    obj["region"] = region;
    obj["apiKey"] = apiKey;
    obj["theme"] = theme;
    WriteJsonToFile(obj, "main");
}


// =========================================================================
// ЧТЕНИЕ
// =========================================================================


bool FileManager::ReadMainFromFile(QString& city, QString& region, QString& apiKey, QString& theme) {
    QFile file("main.json");
    if (!file.open(QIODevice::ReadOnly)) return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) return false;

    const QJsonObject obj = doc.object();
    city = obj.value("city").toString();
    region = obj.value("region").toString();
    apiKey = obj.value("apiKey").toString();
    theme = obj.value("theme").toString();

    return !city.isEmpty() && !region.isEmpty() && !apiKey.isEmpty();
}


QList<WeatherData> FileManager::ReadWeatherFromFile(const QString& fileName) {
    QList<WeatherData> list;

    QFile file(fileName + ".json");
    if (!file.open(QIODevice::ReadOnly)) return list;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) return list;

    const QJsonArray array = doc.object().value("list").toArray();
    for (const QJsonValue& value : array) {
        if (!value.isObject()) continue;

        const QJsonObject obj = value.toObject();
        WeatherData item;
        item._dt = obj.value("dt").toVariant().toLongLong();
        item._temperature = obj.value("temperature").toDouble();
        item._humidity = obj.value("humidity").toDouble();
        item._pressure = obj.value("pressure").toDouble();
        item._windSpeed = obj.value("windSpeed").toDouble();
        item._deg = obj.value("deg").toDouble();
        item._clouds  = obj.value("clouds").toDouble();
        item._cloudsStatus = obj.value("cloudsStatus").toString();
        item._description = obj.value("description").toString();
        item._icon = obj.value("icon").toString();
        list.append(item);
    }

    return list;
}