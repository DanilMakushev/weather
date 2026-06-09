#include <QtTest>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QRadioButton>
#include "mainwindow.h"
#include "ui_mainwindow.h"

class MainWindowTest : public QObject {
    Q_OBJECT

private slots:
    void init() {
        window = new MainWindow();
        window->show();
    }

    void cleanup() {
        delete window;
    }

    // 1 Тест знаков и округления температуры
    void TestFormatTemperature_data() {
        QTest::addColumn<qreal>("inputTemp");
        QTest::addColumn<QString>("expectedStr");

        QTest::newRow("25.3 -> +25°") << 25.3 << "+25°";
        QTest::newRow("14.6 -> +15°") << 14.6 << "+15°";
        QTest::newRow("0.0 -> 0°") << 0.0   << "0°";
        QTest::newRow("0.2 -> 0°") << 0.2 << "0°";
        QTest::newRow("-0.3 -> 0°") << -0.3 << "0°";
        QTest::newRow("-10.5 -> -11°") << -10.5 << "-11°";
    }

    void TestFormatTemperature() {
        QFETCH(qreal, inputTemp);
        QFETCH(QString, expectedStr);

        QCOMPARE(window->FormatTemperature(inputTemp), expectedStr);
    }

    // 2 Тест перевода давления
    void TestFormatPressure() {
        QCOMPARE(window->FormatPressure(1013.25), QString("760 мм рт. ст."));
        QCOMPARE(window->FormatPressure(0), QString("0 мм рт. ст."));
    }

    // 3 Тест розы ветров
    void TestFormatWindDirection_data() {
        QTest::addColumn<qreal>("degrees");
        QTest::addColumn<QString>("expectedDirection");

        QTest::newRow("N1") << 0.0 << "С";
        QTest::newRow("N2") << 350.0 << "С";
        QTest::newRow("E") << 90.0 << "В";
        QTest::newRow("S") << 180.0 << "Ю";
        QTest::newRow("W") << 270.0 << "З";
        QTest::newRow("N-W") << 300.0 << "СЗ";
        QTest::newRow("NaN") << 700.0 << "—";
    }

    void TestFormatWindDirection() {
        QFETCH(qreal, degrees);
        QFETCH(QString, expectedDirection);

        QCOMPARE(window->FormatWindDirection(degrees), expectedDirection);
    }


private:
    MainWindow* window;
};

QTEST_MAIN(MainWindowTest)
#include "mainwindow_test.moc"