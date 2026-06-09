cmake_minimum_required(VERSION 3.19)
project(weather_app LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql Charts Test)

qt_standard_project_setup()

qt_add_executable(weather_app
    WIN32
    mainwindow.ui
    main.cpp
    mainwindow.cpp
    weatherservice.cpp
    filemanager.cpp
    mainwindow.h
    weatherdata.h
    weatherservice.h
    filemanager.h
    resources/resources.qrc
    forecastdata.h
    weatherchart.h
    weatherchart.cpp
    databasemanager.h
    databasemanager.cpp
)

target_link_libraries(weather_app
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
        Qt6::Charts
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data/cities.db")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/cities.db"
         DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif()


enable_testing()


add_executable(mainwindow_test
    mainwindow_test.cpp
    mainwindow.cpp
    weatherservice.cpp
    filemanager.cpp
    weatherchart.cpp
    databasemanager.cpp
)

target_link_libraries(mainwindow_test
    PRIVATE
        Qt6::Test
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
        Qt6::Charts
)
add_test(NAME MainWindowTest COMMAND mainwindow_test)


# Тест для WeatherService
add_executable(weatherservice_test
    weatherservice_test.cpp
    weatherservice.cpp
)
target_link_libraries(weatherservice_test
    PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Network
)
add_test(NAME WeatherServiceTest COMMAND weatherservice_test)