#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQueryModel>

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(const QString& dbName, QObject *parent = nullptr);
    ~DatabaseManager();

    bool IsOpen() const;
    QSqlQueryModel* GetCitiesModel();

private:
    QSqlDatabase _db;
    QSqlQueryModel* _citiesModel = nullptr;
    void InitDatabase(const QString& dbName);
};