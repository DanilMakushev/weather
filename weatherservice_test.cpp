#include <QtTest>
#include <QSignalSpy>
#include "weatherservice.h"
#include "forecastdata.h"

class WeatherServiceTest : public QObject {
    Q_OBJECT

private slots:
    void init() {
        service = new WeatherService(this);
    }

    void cleanup() {
        delete service;
    }

    void testGettersAndSetters() {
        QVERIFY(service->IsApiKeyEmpty());

        service->SetCity("Москва");
        service->SetRegion("Московская область");
        service->SetApiKey("test_api_key_12345");

        QCOMPARE(service->GetCity(), QString("Москва"));
        QCOMPARE(service->GetRegion(), QString("Московская область"));
        QCOMPARE(service->GetApiKey(), QString("test_api_key_12345"));
        QVERIFY(!service->IsApiKeyEmpty());
    }

    void testAggregateForecastByDay() {
        // Метод  принимает список 3-часовых слотов (WeatherData)
        // и должен сгруппировать их по дням (ForecastData), вычислив Мин/Макс температуру.

        QList<WeatherData> mockRawForecast;

        uint secsInDay = 86400;
        uint baseTime = QDateTime(QDate(2026, 6, 15), QTime(0, 0)).toSecsSinceEpoch();


        for (int i = 0; i < 8; ++i) {
            WeatherData data;
            data._dt = baseTime + (i * 3 * 3600); // каждые 3 часа
            data._temperature = 15.0 + i;        // температура от 15 до 22 градусов
            data._icon = "01d";
            mockRawForecast.append(data);
        }

        for (int i = 0; i < 8; ++i) {
            WeatherData data;
            data._dt = baseTime + secsInDay + (i * 3 * 3600);
            data._temperature = 10.0 - i;        // температура от 10 до 3 градусов
            data._icon = "02d";
            mockRawForecast.append(data);
        }

        QList<ForecastData> aggregated = service->AggregateForecastByDay(mockRawForecast);

        // Проверяем результат
        QCOMPARE(aggregated.size(), 2); // Должно получиться ровно 2 дня

        // Проверяем первый день (15 июня)
        // Максимальная должна быть 15+7 = 22, минимальная = 15
        QCOMPARE(aggregated[0]._maxTemp, 22.0);
        QCOMPARE(aggregated[0]._minTemp, 15.0);

        // Проверяем второй день (16 июня)
        // Максимальная должна быть 10, минимальная = 10-7 = 3
        QCOMPARE(aggregated[1]._maxTemp, 10.0);
        QCOMPARE(aggregated[1]._minTemp, 3.0);
    }

private:
    WeatherService* service;
};

QTEST_MAIN(WeatherServiceTest)
#include "weatherservice_test.moc"