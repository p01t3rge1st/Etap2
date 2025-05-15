#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include "SensorReader.h"
#include "SensorDataLogger.h"

// Zakresy Y
/**
 * @class MainWindow
 * @brief Klasa reprezentująca główne okno aplikacji.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @param reader Wskaźnik do obiektu SensorReader.
     * @param parent Wskaźnik do obiektu nadrzędnego (domyślnie nullptr).
     */
    explicit MainWindow(SensorReader* reader, QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Aktualizuje dane z czujników i wyświetla je w interfejsie.
     */
    void updateSensorData();

private:
    // Widgety i logika wykresu
    QComboBox* chartSelector;
    QtCharts::QChart* chart;
    QtCharts::QChartView* chartView;
    QtCharts::QDateTimeAxis* axisX;
    QtCharts::QValueAxis* axisY;
    QtCharts::QLineSeries* co2Series;
    QtCharts::QLineSeries* pm1Series;
    QtCharts::QLineSeries* pm25Series;
    QtCharts::QLineSeries* pm10Series;
    QtCharts::QLineSeries* radiationSeries;
    QTimer* timer;
    SensorReader* sensorReader;
    SensorDataLogger logger;

    // Etykiety do panelu wysepek
    QLabel* co2Label;
    QLabel* co2TempLabel;
    QLabel* co2HumLabel;
    QLabel* pm1Label;
    QLabel* pm25Label;
    QLabel* pm10Label;
    QLabel* radiationLabel;
    QLabel* radiationDoseLabel;

    // Etykiety do interpretacji
    QLabel* co2StatusLabel;
    QLabel* pmStatusLabel;
    QLabel* radiationStatusLabel;

    // Ramki (opcjonalnie, jeśli chcesz mieć do nich dostęp)
    QFrame* sensorDataFrame;
    QFrame* interpretationFrame;
};

#endif
