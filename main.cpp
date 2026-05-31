#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[]){
    setlocale(LC_ALL, "Rus");
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}