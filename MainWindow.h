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
#include <QButtonGroup>
#include <QRadioButton>
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

    /**
     * @brief Ładuje dane historyczne.
     */
    void loadHistoricalData();

    /**
     * @brief Obsługuje zmianę zakresu Time Machine.
     * @param id Identyfikator wybranego zakresu.
     */
    void onTimeMachineChanged(int id);

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
    QtCharts::QLineSeries* temperatureSeries;
    QtCharts::QLineSeries* humiditySeries;
    QtCharts::QLineSeries* radiationDoseSeries;
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

    // Time Machine controls
    QButtonGroup* timeMachineGroup;
    QRadioButton* hour1Button;
    QRadioButton* hour2Button;
    QRadioButton* hour4Button;
    QRadioButton* hour8Button;
    QRadioButton* hour12Button;
    QRadioButton* hour24Button;
    QRadioButton* hour48Button;
    QRadioButton* hour78Button;

    int timeMachineHours;  // Zastąp stałą TIME_MACHINE_HOURS zmienną

    QDateTime chartStartTime;  // Początek zakresu wykresu
};

#endif
