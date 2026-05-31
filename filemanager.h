#pragma once
#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include "weatherdata.h"

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);

    void WriteWeatherToFile(const QList<WeatherData>& list, const QString& fileName);
    void WriteMainToFile(const QString& city, const QString& region, const QString& apiKey, const QString& theme);

    bool ReadMainFromFile(QString& city, QString& region, QString& apiKey, QString& theme);
    QList<WeatherData> ReadWeatherFromFile(const QString& fileName);

private:
    QJsonArray SerializeWeatherList(const QList<WeatherData>& list);
    void WriteJsonToFile(const QJsonObject& data, const QString& fileName);
};