#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QComboBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTranslator>
#include "SensorReader.h"
#include "SensorDataLogger.h"

/**
 * @file MainWindow.h
 * @brief Plik nagłówkowy głównego okna aplikacji monitorującej czujniki.
 */

/**
 * @class MainWindow
 * @brief Główne okno aplikacji wyświetlające dane z czujników i wykresy.
 * 
 * Klasa odpowiada za prezentację danych w czasie rzeczywistym oraz
 * umożliwia przeglądanie historii pomiarów.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor głównego okna
     * @param reader Wskaźnik do czytnika danych z czujników
     * @param parent Wskaźnik do widgetu nadrzędnego
     */
    explicit MainWindow(SensorReader* reader, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * @brief Aktualizuje interfejs danymi z czujników
     */
    void updateSensorData();

    /**
     * @brief Wczytuje i wyświetla dane historyczne
     */
    void loadHistoricalData();

    /**
     * @brief Obsługuje zmianę zakresu czasu na wykresie
     * @param id Nowy zakres czasu w godzinach
     */
    void onTimeMachineChanged(int id);

private:
    /** @brief Komponenty wykresu */
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

    /** @brief Podstawowe komponenty */
    QTimer* timer;
    SensorReader* sensorReader;
    SensorDataLogger logger;

    /** @brief Pola wyświetlające pomiary */
    QLineEdit* co2Label;
    QLineEdit* co2TempLabel;
    QLineEdit* co2HumLabel;
    QLineEdit* pm1Label;
    QLineEdit* pm25Label;
    QLineEdit* pm10Label;
    QLineEdit* radiationLabel;
    QLineEdit* radiationDoseLabel;

    /** @brief Etykiety statusu */
    QLabel* co2StatusLabel;
    QLabel* pmStatusLabel;
    QLabel* radiationStatusLabel;

    /** @brief Kontenery interfejsu */
    QFrame* sensorDataFrame;
    QFrame* interpretationFrame;

    /** @brief Kontrolki wyboru zakresu czasu */
    QButtonGroup* timeMachineGroup;
    QRadioButton* hour1Button;
    QRadioButton* hour2Button;
    QRadioButton* hour4Button;
    QRadioButton* hour8Button;
    QRadioButton* hour12Button;
    QRadioButton* hour24Button;
    QRadioButton* hour48Button;
    QRadioButton* hour78Button;

    int timeMachineHours;      ///< Aktualny zakres czasu w godzinach
    QDateTime chartStartTime;   ///< Początek zakresu wykresu

    /** @brief Komponenty obsługi języków */
    QComboBox* languageSelector;
    QTranslator translator;

    /**
     * @brief Aktualizuje teksty interfejsu po zmianie języka
     */
    void updateInterfaceTexts();

    /**
     * @brief Aktualizuje tytuły wykresów
     */
    void updateChartTitles();

    /**
     * @brief Zmienia język interfejsu
     * @param language Kod języka ("pl" lub "en")
     */
    void changeLanguage(const QString &language);
};

#endif
