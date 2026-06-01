#include "databasemanager.h"
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(const QString& dbName, QObject *parent) : QObject(parent) {
    InitDatabase(dbName);
}

DatabaseManager::~DatabaseManager() {
    if (_db.isOpen()) {
        _db.close();
    }
}

void DatabaseManager::InitDatabase(const QString& dbName) {
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName(dbName);
    if (!_db.open()) {
        qWarning() << "Не удалось открыть базу данных городов:" << _db.lastError().text();
    }
}

bool DatabaseManager::IsOpen() const {
    return _db.isOpen();
}

QSqlQueryModel* DatabaseManager::GetCitiesModel() {
    if (!_citiesModel && _db.isOpen()) {
        _citiesModel = new QSqlQueryModel(this);
        _citiesModel->setQuery("SELECT name || ', ' || region FROM cities ORDER BY name");
    }
    return _citiesModel;
}