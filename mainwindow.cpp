#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include "forecastdata.h"
#include "weatherchart.h"


// ==========================================
// КОНСТРУКТОР И ДЕСТРУКТОР
// ==========================================


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    InitUiCollections();
    SetupConnections();
    InitDatabase(); 
    InitUiSettings();
    InitTimers();
    LoadInitialData();
}


MainWindow::~MainWindow() {
    delete _ui;
}


// ==========================================
// ИНИЦИАЛИЗАЦИЯ
// ==========================================


void MainWindow::InitUiSettings() {
    setFixedSize(size());
    _ui->stackedWidget->setCurrentIndex(0);
    _ui->currentTimeLabel->hide();
    _ui->cityNameButton->hide();

    const auto& icons = _iconLabels;
    for (QLabel* label : icons){
        label->setScaledContents(true);
    }
    _ui->iconLabelMain->setScaledContents(true);

    QIcon locationIcon(":/icons/icons/location.png");
    _ui->cityNameButton->setIcon(locationIcon);
    _ui->cityNameButton->setIconSize(QSize(24, 24));

    // Управление видимостью в зависимости от страницы
    connect(_ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int index) {
        if (index == 0 || index == 1 || index == 4 || index == 5) {
            _ui->currentTimeLabel->hide();
            _ui->cityNameButton->hide();
        }
        else {
            _ui->currentTimeLabel->show();
            _ui->cityNameButton->show();
        }
        if (_weatherService->IsApiKeyEmpty()) {
            _ui->backButton->hide();
            _ui->backButton_2->hide();
        }
        else if (_weatherService->GetCity().isEmpty()) {
            _ui->backButton->hide();
            if (index == 4) {
                _ui->backButton_2->show();
            }
        }
        else {
            _ui->backButton->show();
            _ui->backButton_2->show();
        }
    });
}


void MainWindow::InitUiCollections() {
    _timeLabels << _ui->timeLabel_1 << _ui->timeLabel_2 << _ui->timeLabel_3 << _ui->timeLabel_4
                << _ui->timeLabel_5 << _ui->timeLabel_6 << _ui->timeLabel_7 << _ui->timeLabel_8;

    _tempLabels << _ui->tempLabel_1 << _ui->tempLabel_2 << _ui->tempLabel_3 << _ui->tempLabel_4
                << _ui->tempLabel_5 << _ui->tempLabel_6 << _ui->tempLabel_7 << _ui->tempLabel_8;

    _iconLabels << _ui->iconLabel_1 << _ui->iconLabel_2 << _ui->iconLabel_3 << _ui->iconLabel_4
                << _ui->iconLabel_5 << _ui->iconLabel_6 << _ui->iconLabel_7 << _ui->iconLabel_8;

    _dateForecastLabels << _ui->dateForecastLabel_1 << _ui->dateForecastLabel_2 << _ui->dateForecastLabel_3
                        << _ui->dateForecastLabel_4 << _ui->dateForecastLabel_5;

    _tempMaxForecastLabels << _ui->tempMaxForecastLabel_1 << _ui->tempMaxForecastLabel_2 << _ui->tempMaxForecastLabel_3
                           << _ui->tempMaxForecastLabel_4 << _ui->tempMaxForecastLabel_5;

    _tempMinForecastLabels << _ui->tempMinForecastLabel_1 << _ui->tempMinForecastLabel_2 << _ui->tempMinForecastLabel_3
                           << _ui->tempMinForecastLabel_4 << _ui->tempMinForecastLabel_5;

    _iconForecastLabels << _ui->iconForecastLabel_1 << _ui->iconForecastLabel_2 << _ui->iconForecastLabel_3
                        << _ui->iconForecastLabel_4 << _ui->iconForecastLabel_5;

    _detailHourTimeLabels << _ui->detailHourTime_1 << _ui->detailHourTime_2 << _ui->detailHourTime_3
                          << _ui->detailHourTime_4 << _ui->detailHourTime_5 << _ui->detailHourTime_6
                          << _ui->detailHourTime_7 << _ui->detailHourTime_8;

    _detailHourIconLabels << _ui->detailHourIcon_1 << _ui->detailHourIcon_2 << _ui->detailHourIcon_3
                          << _ui->detailHourIcon_4 << _ui->detailHourIcon_5 << _ui->detailHourIcon_6
                          << _ui->detailHourIcon_7 << _ui->detailHourIcon_8;

    _detailHourTemperatureLabels << _ui->detailHourTemperature_1 << _ui->detailHourTemperature_2 << _ui->detailHourTemperature_3
                                 << _ui->detailHourTemperature_4 << _ui->detailHourTemperature_5 << _ui->detailHourTemperature_6
                                 << _ui->detailHourTemperature_7 << _ui->detailHourTemperature_8;

    // Регистрируем фреймы дней в фильтре событий
    _ui->frameForecast_1->installEventFilter(this);
    _ui->frameForecast_2->installEventFilter(this);
    _ui->frameForecast_3->installEventFilter(this);
    _ui->frameForecast_4->installEventFilter(this);
    _ui->frameForecast_5->installEventFilter(this);

    // делаем все внутренние лейблы карточек "прозрачными" для кликов
    for (int i = 0; i < 5; ++i) {
        _dateForecastLabels[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
        _tempMaxForecastLabels[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
        _tempMinForecastLabels[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
        _iconForecastLabels[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}


void MainWindow::InitDatabase() {
    _dbManager = new DatabaseManager(DB_NAME, this);

    QSqlQueryModel* cityModel = _dbManager->GetCitiesModel();

    auto* completer = new QCompleter(cityModel, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchStartsWith);
    _ui->lineEditCityName->setCompleter(completer);

    connect(completer, qOverload<const QString&>(&QCompleter::activated),
            this, [this](const QString& fullText) {
                const QStringList parts = fullText.split(", ");
                if (parts.size() >= 2) {
                    _weatherService->SetCity(parts[0]);
                    _weatherService->SetRegion(parts[1]);
                    _ui->confirmManuallyButton->show();
                }
            });

    connect(_ui->lineEditCityName, &QLineEdit::textEdited, this, [this]() {
        if (_ui->confirmManuallyButton->isVisible()) {
            _weatherService->SetCity("");
            _weatherService->SetRegion("");
            _ui->confirmManuallyButton->hide();
        }
    });
}


void MainWindow::InitTimers() {
    _timerDateTime = new QTimer(this);
    connect(_timerDateTime, &QTimer::timeout, this, &MainWindow::OnTimerDateTimeTick);
    _timerDateTime->start(TIMER_DATETIME_MS);
    OnTimerDateTimeTick(); // сразу отображаем время

    _timerWeather = new QTimer(this);
    connect(_timerWeather, &QTimer::timeout, this, &MainWindow::OnTimerWeatherTick);
    _timerWeather->start(TIMER_WEATHER_MS);

    _timerCheckInternet = new QTimer(this);
    _timerCheckInternet->start(TIMER_CHECK_INTERNET_MS);
    connect(_timerCheckInternet, &QTimer::timeout, this, &MainWindow::OnTimerCheckInternetTick);
}


void MainWindow::SetupConnections() {
    _fileManager = new FileManager(this);
    _weatherService = new WeatherService(this);

    // Сигналы от сервиса
    connect(_weatherService, &WeatherService::ErrorOccurred, this, [this]() {_ui->errorMessage->show(); NoInternet(); });
    connect(_weatherService, &WeatherService::ApiKeyValidationResult, this, &MainWindow::OnApiKeyValidationResult);
    connect(_weatherService, &WeatherService::WeatherTodayListReady,  this, &MainWindow::OnWeatherTodayListReady);
    connect(_weatherService, &WeatherService::ForecastListReady,      this, &MainWindow::OnForecastListReady);
    connect(_weatherService, &WeatherService::CityRegionAutoReady,    this, &MainWindow::OnCityRegionAutoReady);

    // Кнопки подтверждения
    connect(_ui->confirmManuallyButton, &QPushButton::clicked, this, &MainWindow::OnConfirmManuallyButtonClicked);
    connect(_ui->confirmApiKeyButton,   &QPushButton::clicked, this, &MainWindow::OnConfirmApiKeyClicked);

    // Навигация
    connect(_ui->cityNameButton,    &QPushButton::clicked, this, &MainWindow::GoToCitySelection);
    connect(_ui->settingsButton,    &QPushButton::clicked, this, [this]() { _ui->stackedWidget->setCurrentIndex(5); });
    connect(_ui->editApiKeyButton,  &QPushButton::clicked, this, &MainWindow::GoToApiKeyInput);
    connect(_ui->backButton,        &QPushButton::clicked, this, [this]() { _ui->stackedWidget->setCurrentIndex(2); });
    connect(_ui->backButton_2,      &QPushButton::clicked, this, [this]() { _ui->stackedWidget->setCurrentIndex(5); });
    connect(_ui->backButton_3,      &QPushButton::clicked, this, [this]() { _ui->stackedWidget->setCurrentIndex(2); });
    connect(_ui->toForecastButton,  &QPushButton::clicked, this, [this]() { _ui->stackedWidget->setCurrentIndex(3); });
    connect(_ui->BackToMainButton,  &QPushButton::clicked, this, [this]() { _ui->stackedWidget->setCurrentIndex(2); });

    // Автоопределение города
    connect(_ui->confirmAutoButton, &QPushButton::clicked, this, [this]() {
        _weatherService->SetCity(_detectedCity);
        _weatherService->SetRegion(_detectedRegion);
        _ui->stackedWidget->setCurrentIndex(0);
        _weatherService->RequestData();
    });
    connect(_ui->cancelAutoButton, &QPushButton::clicked, this, [this]() {
        _ui->suggestFrame->hide();
        _ui->lineEditCityName->clearFocus();
        setFocus();
    });

    // Дизайн
    connect(_ui->darkRadioButton, &QRadioButton::toggled, this, [this](bool checked){
        if(checked) SetStyle("dark");
        _fileManager->WriteMainToFile(_weatherService->GetCity(), _weatherService->GetRegion(), _weatherService->GetApiKey(), "dark");
        _theme = "dark";
    });
    connect(_ui->lightRadioButton, &QRadioButton::toggled, this, [this](bool checked){
        if(checked) SetStyle("light");
        _fileManager->WriteMainToFile(_weatherService->GetCity(), _weatherService->GetRegion(), _weatherService->GetApiKey(), "light");
        _theme = "light";
    });
}


void MainWindow::LoadInitialData() {
    QString city, region, apiKey, theme;

    if (_fileManager->ReadMainFromFile(city, region, apiKey, theme)) {
        _weatherService->SetCity(city);
        _weatherService->SetRegion(region);
        _weatherService->SetApiKey(apiKey);

        if (theme == "light") {
            _ui->lightRadioButton->setChecked(true);
            SetStyle("light");
            _theme = "light";
        } else {
            _ui->darkRadioButton->setChecked(true);
            SetStyle("dark");
            _theme = "dark";
        }

        if (_weatherService->IsInternetAvailable()) {
            _weatherService->RequestData();
        } else {
            _weatherTodayList = _fileManager->ReadWeatherFromFile(FILE_WEATHER_TODAY);
            _forecastList = _fileManager->ReadWeatherFromFile(FILE_FORECAST);
            ShowTodayWeather(false);
            ShowForecast(false);
        }
    } else {
        _ui->darkRadioButton->setChecked(true); 
        _theme = "dark"; 
        
        _ui->backButton->hide();
        GoToApiKeyInput(); 
    }
}


// ==========================================
// НАВИГАЦИЯ
// ==========================================


void MainWindow::GoToCitySelection() {
    _ui->stackedWidget->setCurrentIndex(1);
    setFocus();
    _ui->lineEditCityName->setPlaceholderText("Начните вводить название города");
    _ui->suggestFrame->hide();
    _ui->confirmManuallyButton->hide();
    _ui->errorMessage->hide();
    _weatherService->DetectCityRegionAuto();
}


void MainWindow::GoToApiKeyInput() {
    _ui->stackedWidget->setCurrentIndex(4);
    setFocus();
    _ui->apiKeyLineEdit->setPlaceholderText("Введите ваш API ключ");
}



bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    // Проверяем, что событие — это именно нажатие кнопки мыши
    if (event->type() == QEvent::MouseButtonPress) {

        // Проверяем, какой именно фрейм нажал пользователь
        if (obj == _ui->frameForecast_1) {
            OnDayFrameClicked(0);
            return true;
        }
        else if (obj == _ui->frameForecast_2) {
            OnDayFrameClicked(1);
            return true;
        }
        else if (obj == _ui->frameForecast_3) {
            OnDayFrameClicked(2);
            return true;
        }
        else if (obj == _ui->frameForecast_4) {
            OnDayFrameClicked(3);
            return true;
        }
        else if (obj == _ui->frameForecast_5) {
            OnDayFrameClicked(4);
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}


// ==========================================
// СЛОТЫ
// ==========================================


void MainWindow::OnWeatherTodayListReady(const QList<WeatherData>& list) {
    _weatherTodayList = list;
    _fileManager->WriteMainToFile(_weatherService->GetCity(), _weatherService->GetRegion(), _weatherService->GetApiKey(), GetTheme());
    _fileManager->WriteWeatherToFile(_weatherTodayList, FILE_WEATHER_TODAY);
    ShowTodayWeather(true);
}


void MainWindow::OnForecastListReady(const QList<WeatherData>& list) {
    _forecastList = list;
    _fileManager->WriteWeatherToFile(_forecastList, FILE_FORECAST);
    ShowForecast(true);
    OnDayFrameClicked(0);
}


void MainWindow::OnCityRegionAutoReady(const QString& city, const QString& region) {
    _detectedCity = city;
    _detectedRegion = region;
    _ui->CityRegionNameAutoLabel->setText("Это ваш город?\n" + city + ", " + region);
    _ui->suggestFrame->show();
}


void MainWindow::OnConfirmApiKeyClicked() {
    const QString key = _ui->apiKeyLineEdit->text().trimmed();
    if (key.isEmpty()) {
        _ui->errorMessage->setText("Поле не может быть пустым!");
        _ui->errorMessage->show();
        return;
    }
    _ui->confirmApiKeyButton->setEnabled(false);
    _ui->errorMessage->hide();
    _weatherService->ValidateApiKey(key);
}


void MainWindow::OnApiKeyValidationResult(bool success) {
    _ui->confirmApiKeyButton->setEnabled(true);

    if (!success) {
        _ui->apiKeyLineEdit->clear();
        _ui->apiKeyLineEdit->setPlaceholderText("Ошибка: некорректный ключ");
        _ui->apiKeyLineEdit->clearFocus();
        setFocus();
        qDebug() << "Введен некорректный API ключ";
        return;
    }

    const QString validKey = _ui->apiKeyLineEdit->text().trimmed();
    _weatherService->SetApiKey(validKey);
    qDebug() << "API ключ сохранен";
    _ui->apiKeyLineEdit->clear();

    if (_weatherService->GetCity().isEmpty()) {
        GoToCitySelection(); // первый запуск — переходим к выбору города
    } else {
        _fileManager->WriteMainToFile(_weatherService->GetCity(), _weatherService->GetRegion(), validKey, _theme);
        _weatherService->RequestData();
        _ui->stackedWidget->setCurrentIndex(5);
    }
}


void MainWindow::OnConfirmManuallyButtonClicked() {
    _ui->stackedWidget->setCurrentIndex(0);
    _weatherService->RequestData();
}


void MainWindow::OnDayFrameClicked(int dayIndex){
    if (_forecastList.isEmpty()) return;

    // Очищаем labels перед новым отображением
    _ui->detailDate->setText("--:--");
    for (int i = 0; i < 8; ++i) {
        _detailHourTimeLabels[i]->setText("--:--");
        _detailHourIconLabels[i]->clear();
        _detailHourTemperatureLabels[i]->setText("--");
    }

    // Собираем список уникальных дат, которые есть в общем прогнозе
    const auto& forecast = _forecastList;
    QStringList uniqueDates;
    for (const WeatherData& item : forecast) { // Цикл стал чистым
        QString dateKey = QDateTime::fromSecsSinceEpoch(item._dt).date().toString("yyyy-MM-dd");
        if (!uniqueDates.contains(dateKey)) {
            uniqueDates.append(dateKey);
        }
    }

    // Защита от выхода за границы списка дней
    if (dayIndex >= uniqueDates.size()) return;
    QString targetDateStr = uniqueDates[dayIndex]; // Наша целевая дата (например, "2026-05-30")

    // Вытаскиваем из общего списка только те 3-часовые слоты, которые принадлежат этой дате
    QList<WeatherData> daySlots;
    for (const WeatherData& item : _forecastList) {
        QString itemDate = QDateTime::fromSecsSinceEpoch(item._dt).date().toString("yyyy-MM-dd");
        if (itemDate == targetDateStr) {
            daySlots.append(item);
        }
    }

    if (daySlots.isEmpty()) return;

    // Выводим текст даты
    QDateTime firstSlotTime = QDateTime::fromSecsSinceEpoch(daySlots.first()._dt);
    QLocale ruLocale(QLocale::Russian);

    // Формат "dddd, d MMMM" сделает строку вида "суббота, 30 мая"
    QString dateTitle = ruLocale.toString(firstSlotTime.date(), "dddd, d MMMM");
    if (!dateTitle.isEmpty()) {
        dateTitle[0] = dateTitle[0].toUpper(); // Делаем первую букву дня недели заглавной
    }

    if (dayIndex == 0) {
        dateTitle = "Сегодня, " + ruLocale.toString(firstSlotTime.date(), "d MMMM");
    }
    _ui->detailDate->setText(dateTitle);

    // Распределяем данные
    for (int i = 0; i < 8; ++i) {
        if (i < daySlots.size()) {
            // Данные для этого шага времени есть — выводим их
            const WeatherData& slot = daySlots[i];
            QDateTime slotTime = QDateTime::fromSecsSinceEpoch(slot._dt);

            _detailHourTimeLabels[i]->setText(slotTime.time().toString("hh:mm"));

            _detailHourIconLabels[i]->setPixmap(QPixmap(":/icons/icons/" + slot._icon + ".png"));

            _detailHourTemperatureLabels[i]->setText(FormatTemperature(slot._temperature));
        }
        else {
            // Если слотов в сутках оказалось меньше 9
            // заполняем оставшиеся ячейки прочерками, чтобы сохранить верстку.
            _detailHourTimeLabels[i]->setText("--:--");
            _detailHourIconLabels[i]->setPixmap(QPixmap()); // Очищаем иконку
            _detailHourTemperatureLabels[i]->setText("--");
        }
    }
}


// ==========================================
// ОТРИСОВКА ДАННЫХ
// ==========================================


void MainWindow::ShowTodayWeather(bool isNewData) {
    if (_weatherTodayList.isEmpty()) return;

    _ui->cityNameButton->setText(_weatherService->GetCity());

    // Текущая погода (первый элемент — ответ /weather)
    const WeatherData& current = _weatherTodayList[0];
    _ui->tempLabel->setText(FormatTemperature(current._temperature));
    _ui->cloudsStatusLabel->setText(current._cloudsStatus);
    _ui->humidityLabel->setText(FormatHumidity(current._humidity));
    _ui->windSpeedLabel->setText(FormatWindSpeed(current._windSpeed));
    _ui->pressureLabel->setText(FormatPressure(current._pressure));
    _ui->degLabel->setText(FormatWindDirection(current._deg));
    _ui->iconLabelMain->setPixmap(QPixmap(":/icons/icons/" + current._icon + ".png"));

    // Почасовой прогноз
    const int count = qMin(8, _weatherTodayList.size());
    for (int i = 0; i < count; ++i) {
        const WeatherData& w = _weatherTodayList[i];
        _timeLabels[i]->setText(QDateTime::fromSecsSinceEpoch(w._dt).toString("HH:mm"));
        _tempLabels[i]->setText(FormatTemperature(w._temperature));
        _iconLabels[i]->setPixmap(QPixmap(":/icons/icons/" + w._icon + ".png"));
    }

    if (isNewData) {
        _timeLabels[0]->setText("Сейчас");
        _ui->infoLabel->clear();
        _ui->cityNameButton->setEnabled(true);
    } else {
        NoInternet();
    }

    _ui->stackedWidget->setCurrentIndex(2);
    _ui->backButton->show();
    _ui->lineEditCityName->clear();
}


void MainWindow::ShowForecast(bool isNewData) {
    if (_forecastList.isEmpty()) return;

    _ui->cityNameButton->setText(_weatherService->GetCity());

    const QList<ForecastData> fiveDaysList = _weatherService->AggregateForecastByDay(_forecastList);
    const int count = qMin(5, fiveDaysList.size());
    for (int i = 0; i < count; ++i) {
        const ForecastData& day = fiveDaysList[i];
        _dateForecastLabels[i]->setText(day._date);
        _tempMaxForecastLabels[i]->setText(FormatTemperature(day._maxTemp));
        _tempMinForecastLabels[i]->setText(FormatTemperature(day._minTemp));
        _iconForecastLabels[i]->setPixmap(QPixmap(":/icons/icons/" + day._icon + ".png"));
    }

    if (isNewData) {
        _dateForecastLabels[0]->setText("Сегодня\n" + fiveDaysList[0]._date.split('\n').last());
        _ui->infoLabel_2->clear();
        _ui->cityNameButton->setEnabled(true);
    } else {
        NoInternet();
    }

    ShowChart(fiveDaysList);
}


void MainWindow::ShowChart(const QList<ForecastData>& fiveDaysList) {
    if (!_weatherChart) {
        _weatherChart = new WeatherChart(_ui->forecastFrame);
    }
    // Игнорируем мышь для обработки нажатий на frame
    _weatherChart->setAttribute(Qt::WA_TransparentForMouseEvents);

    _weatherChart->UpdateData(fiveDaysList);

    _weatherChart->setGeometry(30, 160, 820, 160);
    _weatherChart->raise();
    _weatherChart->show();
}


void MainWindow::NoInternet(){
    _ui->infoLabel->setText("Не удалось обновить информацию. Данные могут быть неактуальными.");
    _ui->infoLabel_2->setText("Не удалось обновить информацию. Данные могут быть неактуальными.");
    _ui->cityNameButton->setEnabled(false);
    _ui->editApiKeyButton->setEnabled(false);
    int currentPage = _ui->stackedWidget->currentIndex();
    if(currentPage != 2 && currentPage != 3 && currentPage != 5){
        _ui->stackedWidget->setCurrentIndex(2);
    }
    qDebug() << "Нет Интернет соединения";
}


void MainWindow::InternetIsWorking(){
    _ui->infoLabel->clear();
    _ui->infoLabel_2->clear();
    _ui->cityNameButton->setEnabled(true);
    _ui->editApiKeyButton->setEnabled(true);
    qDebug() << "Интернет соединение есть";
}


// ==========================================
// ФОРМАТИРОВАНИЕ ЗНАЧЕНИЙ
// ==========================================


QString MainWindow::GetTemperatureSign(qreal temp) const {
    return (temp >= 0.5) ? "+" : "";
}


QString MainWindow::FormatTemperature(qreal temp) const {
    return GetTemperatureSign(temp) + QString::number(static_cast<int>(std::round(temp))) + "°";
}


QString MainWindow::FormatHumidity(qreal hum) const {
    return QString::number(hum) + "%";
}


QString MainWindow::FormatPressure(qreal press) const {
    return QString::number(static_cast<int>(std::round(press * 0.750062))) + " мм рт. ст.";
}


QString MainWindow::FormatWindSpeed(qreal speed) const {
    return QString::number(speed) + " м/с";
}


QString MainWindow::FormatWindDirection(qreal deg) const {
    if (deg >= 337.5 || deg <  22.5)  return "С";
    if (deg >=  22.5 && deg <  67.5)  return "СВ";
    if (deg >=  67.5 && deg < 112.5)  return "В";
    if (deg >= 112.5 && deg < 157.5)  return "ЮВ";
    if (deg >= 157.5 && deg < 202.5)  return "Ю";
    if (deg >= 202.5 && deg < 247.5)  return "ЮЗ";
    if (deg >= 247.5 && deg < 292.5)  return "З";
    if (deg >= 292.5 && deg < 337.5)  return "СЗ";
    return "—";
}


// ==========================================
// ТАЙМЕРЫ
// ==========================================


void MainWindow::OnTimerDateTimeTick() {
    _ui->currentTimeLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy  HH:mm"));
}


void MainWindow::OnTimerWeatherTick() {
    if (_weatherService->IsInternetAvailable()){
        _weatherService->RequestData();
    }
}


void MainWindow::OnTimerCheckInternetTick(){
    if (_weatherService->IsInternetAvailable()){
        InternetIsWorking();
    }
    else{
        NoInternet();
    }
}


// ==========================================
// ДИЗАЙН
// ==========================================


void MainWindow::SetStyle(const QString& theme) {
    if (theme == "dark") {
        this->setStyleSheet(R"(
            /* основной фон */
            QMainWindow, #centralwidget {
                background-color: rgb(44, 44, 44);
            }

            /* текст */
            QLabel {
                background-color: transparent;
                color: #e0e0e0;
            }

            #tempLabel {
                color: #ffffff;
                font-weight: bold;
            }

            /* поля ввода */
            QLineEdit#lineEditCityName, QLineEdit#apiKeyLineEdit {
                background-color: #3a3a3a;
                color: #ffffff;
                placeholder-text-color: #adadad;
                border: 1px solid #555555;
                border-radius: 6px;
                padding: 0px 10px;
            }

            QLineEdit#lineEditCityName, QLineEdit#apiKeyLineEdit:focus {
                border: 1px solid #00a8ff;
            }

            /* кнопки */
            QPushButton {
                background-color: #3a3a3a;
                color: #ffffff;
                border: 1px solid #555555;
                border-radius: 6px;
            }

            QPushButton:hover {
                background-color: #4a4a4a;
                border: 1px solid #00a8ff;
            }

            QPushButton:pressed {
                background-color: #2b2b2b;
            }

            QRadioButton {
                color: #fff;
            }

            /* frame */
            #suggestFrame, #weatherTodayFrame, #forecastFrame, #detailFrame, #detailDateFrame, #frame {
                background-color: #323232;
                border: 1px solid #444444;
                border-radius: 8px;
            }

            QFrame[frameShape="NoFrame"] {
                background-color: transparent;
                border: none;
            }
        )");
    }
    else {
        this->setStyleSheet(R"(
            /* основной фон */
            QMainWindow, #stackedWidget {
                background-color: #a8a8a8;
            }

            /* текст */
            QLabel {
                background-color: transparent;
                color: #000;
            }

            #tempLabel {
                color: #000;
                font-weight: bold;
            }

            /* поля ввода */
            QLineEdit#lineEditCityName, QLineEdit#apiKeyLineEdit {
                background-color: #979797;
                color: #000;
                placeholder-text-color: #1a1a1a;
                border: 1px solid #555555;
                border-radius: 8px;
                padding: 0px 10px;
            }

            /* кнопки */
            QPushButton {
                background-color: #979797;
                color: #000000;
                border: 1px solid #555555;
                border-radius: 6px;
            }

            QToolButton {
                background-color: #979797;
                color: #000000;
                border: 1px solid #555555;
                border-radius: 6px;
            }

            QPushButton:hover {
                background-color: #757575;
                border: 1px solid #00a8ff;
            }

            QToolButton:hover {
                background-color: #757575;
                border: 1px solid #00a8ff;
            }

            QPushButton:pressed {
                background-color: #595959;
            }

            QToolButton:pressed {
                background-color: #595959;
            }

            QRadioButton {
                color: #000000;
            }

            /* frame */
            #suggestFrame, #weatherTodayFrame, #forecastFrame, #detailFrame, #detailDateFrame, #frame {
                background-color: #979797;
                border: 1px solid #444444;
                border-radius: 8px;
            }

            QFrame[frameShape="NoFrame"] {
                background-color: transparent;
                border: none;
            }
        )");
    }
}