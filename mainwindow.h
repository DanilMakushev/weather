#pragma once
#include <QMainWindow>
#include <QList>
#include <QString>
#include <QCompleter>
#include <QLabel>
#include <QTimer>
#include "weatherdata.h"
#include "weatherservice.h"
#include "filemanager.h"
#include "weatherchart.h"
#include "databasemanager.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    QString GetTheme() { return _theme; }

private slots:
    // Слоты сервисных сигналов
    void OnWeatherTodayListReady(const QList<WeatherData>& list);
    void OnForecastListReady(const QList<WeatherData>& list);
    void OnCityRegionAutoReady(const QString& city, const QString& region);
    void OnApiKeyValidationResult(bool success);

    // Слоты кнопок UI
    void OnConfirmApiKeyClicked();
    void OnConfirmManuallyButtonClicked();

    // Слоты таймеров
    void OnTimerDateTimeTick();
    void OnTimerWeatherTick();
    void OnTimerCheckInternetTick();
    void InternetIsWorking();

    void OnDayFrameClicked(int dayIndex);

private:
    // Инициализация
    void InitUiSettings();
    void InitUiCollections();
    void InitDatabase();
    void InitTimers();
    void SetupConnections();
    void LoadInitialData();

    // Навигация и управление состоянием
    void GoToCitySelection();
    void GoToApiKeyInput();

    //  Отрисовка данных
    void ShowTodayWeather(bool isNewData);
    void ShowForecast(bool isNewData);
    void ShowChart(const QList<ForecastData>& fiveDaysList);
    void NoInternet();

    // Форматирование значений
    QString FormatTemperature(qreal temp) const;
    QString FormatHumidity(qreal hum) const;
    QString FormatPressure(qreal press) const;
    QString FormatWindSpeed(qreal speed) const;
    QString FormatWindDirection(qreal deg) const;
    QString GetTemperatureSign(qreal temp) const;

    // Дизайн
    void SetStyle(const QString& theme);
    QString _theme;

    // Данные
    Ui::MainWindow*  _ui;
    WeatherService* _weatherService = nullptr;
    FileManager* _fileManager = nullptr;
    WeatherChart* _weatherChart = nullptr;
    DatabaseManager* _dbManager = nullptr;
    QTimer*  _timerDateTime;
    QTimer* _timerWeather;
    QTimer* _timerCheckInternet;

    QList<WeatherData> _weatherTodayList;
    QList<WeatherData> _forecastList;

    QString _detectedCity;
    QString _detectedRegion;

    // Коллекции виджетов
    QList<QLabel*> _timeLabels;
    QList<QLabel*> _tempLabels;
    QList<QLabel*> _iconLabels;
    QList<QLabel*> _dateForecastLabels;
    QList<QLabel*> _iconForecastLabels;
    QList<QLabel*> _tempMaxForecastLabels;
    QList<QLabel*> _tempMinForecastLabels;
    QList<QLabel*> _detailHourTimeLabels;
    QList<QLabel*> _detailHourIconLabels;
    QList<QLabel*> _detailHourTemperatureLabels;

    // Константы
    static constexpr int TIMER_DATETIME_MS = 1000;
    static constexpr int TIMER_CHECK_INTERNET_MS = 10000;
    static constexpr int TIMER_WEATHER_MS  = 600000;
    const QString DB_NAME = "cities.db";
    const QString FILE_WEATHER_TODAY = "weathertoday";
    const QString FILE_FORECAST = "forecast";

    bool eventFilter(QObject *obj, QEvent *event) override;

    friend class MainWindowTest;
};