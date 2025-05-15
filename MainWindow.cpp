#include "MainWindow.h"
#include "SensorDataLogger.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include "SensorReader.h"
#include <QApplication>
#include <QTimer>
#include <QFrame>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QComboBox>

using namespace std;

const int HIGH_CO2 = 1200;
const int HIGH_PM1 = 25;
const int HIGH_PM25 = 25;
const int HIGH_PM10 = 50;
const int HIGH_RADIATION = 30;
const float CPS_PER_USV = 0.0037;

const int MAX_CO2 = 2000;
const int MAX_PM1 = 50;
const int MAX_PM25 = 50;
const int MAX_PM10 = 100;
const int MAX_RADIATION = 100;

MainWindow::MainWindow(SensorReader* reader, QWidget *parent)
    : QMainWindow(parent), sensorReader(reader), logger("dane_czujnikow.csv")
{
    // --- PANEL WYSEPEK ---
    co2Label = new QLabel("--", this);
    co2TempLabel = new QLabel("--", this);
    co2HumLabel = new QLabel("--", this);
    pm1Label = new QLabel("--", this);
    pm25Label = new QLabel("--", this);
    pm10Label = new QLabel("--", this);
    radiationLabel = new QLabel("--", this);
    radiationDoseLabel = new QLabel("--", this);

    co2StatusLabel = new QLabel("--", this);
    pmStatusLabel = new QLabel("--", this);
    radiationStatusLabel = new QLabel("--", this);

    QGridLayout* sensorDataLayout = new QGridLayout();
    sensorDataLayout->addWidget(new QLabel("CO2 (Dwutlenek Węgla):", this), 0, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(co2Label, 0, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Temperatura:", this), 1, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(co2TempLabel, 1, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Wilgotność:", this), 2, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(co2HumLabel, 2, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Pył zawieszony PM1.0:", this), 3, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(pm1Label, 3, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Pył zawieszony PM2.5:", this), 4, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(pm25Label, 4, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Pył zawieszony PM10:", this), 5, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(pm10Label, 5, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Zliczenia det. Geiger Mullera:", this), 6, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(radiationLabel, 6, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("Dawka przyjęta na godzinę:", this), 7, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(radiationDoseLabel, 7, 1, Qt::AlignLeft);

    QFrame* sensorDataFrame = new QFrame(this);
    sensorDataFrame->setLayout(sensorDataLayout);
    sensorDataFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    sensorDataFrame->setLineWidth(2);

    QGridLayout* interpretationLayout = new QGridLayout();
    interpretationLayout->addWidget(new QLabel("CO2 Status:", this), 0, 0, Qt::AlignRight);
    interpretationLayout->addWidget(co2StatusLabel, 0, 1, Qt::AlignLeft);
    interpretationLayout->addWidget(new QLabel("PM Status:", this), 1, 0, Qt::AlignRight);
    interpretationLayout->addWidget(pmStatusLabel, 1, 1, Qt::AlignLeft);
    interpretationLayout->addWidget(new QLabel("Prom. Status:", this), 2, 0, Qt::AlignRight);
    interpretationLayout->addWidget(radiationStatusLabel, 2, 1, Qt::AlignLeft);

    QFrame* interpretationFrame = new QFrame(this);
    interpretationFrame->setLayout(interpretationLayout);
    interpretationFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    interpretationFrame->setLineWidth(2);

    // --- WYKRESY ---
    // Serie
    co2Series = new QtCharts::QLineSeries();
    pm1Series = new QtCharts::QLineSeries();
    pm25Series = new QtCharts::QLineSeries();
    pm10Series = new QtCharts::QLineSeries();
    radiationSeries = new QtCharts::QLineSeries();

    axisX = new QtCharts::QDateTimeAxis;
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText("Czas");
    axisY = new QtCharts::QValueAxis;

    chart = new QtCharts::QChart();
    chart->addSeries(co2Series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    co2Series->attachAxis(axisX);
    co2Series->attachAxis(axisY);
    chart->setTitle("CO2 (ppm)");
    axisY->setTitleText("CO2 [ppm]");
    axisY->setRange(0, MAX_CO2);

    chartView = new QtCharts::QChartView(chart);
    chartView->setMinimumSize(700, 400);

    chartSelector = new QComboBox(this);
    chartSelector->addItem("CO2");
    chartSelector->addItem("PM1.0");
    chartSelector->addItem("PM2.5");
    chartSelector->addItem("PM10");
    chartSelector->addItem("Promieniowanie");

    // --- UKŁAD GŁÓWNY ---
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel("Interpretacja danych:", this));
    mainLayout->addWidget(interpretationFrame);
    mainLayout->addWidget(new QLabel("Dane z czujników:", this));
    mainLayout->addWidget(sensorDataFrame);
    mainLayout->addWidget(chartSelector);
    mainLayout->addWidget(chartView);

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // --- Przełączanie wykresów ---
    connect(chartSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        QList<QtCharts::QLineSeries*> allSeries = {co2Series, pm1Series, pm25Series, pm10Series, radiationSeries};
        for (auto series : allSeries) {
            series->detachAxis(axisX);
            series->detachAxis(axisY);
            chart->removeSeries(series);
        }
        QtCharts::QLineSeries* currentSeries = nullptr;
        int idx = chartSelector->currentIndex();
        if (idx == 0) {
            chart->setTitle("CO2 (ppm)");
            axisY->setTitleText("CO2 [ppm]");
            axisY->setRange(0, MAX_CO2);
            currentSeries = co2Series;
        } else if (idx == 1) {
            chart->setTitle("PM1.0 (µg/m³)");
            axisY->setTitleText("PM1.0 [µg/m³]");
            axisY->setRange(0, MAX_PM1);
            currentSeries = pm1Series;
        } else if (idx == 2) {
            chart->setTitle("PM2.5 (µg/m³)");
            axisY->setTitleText("PM2.5 [µg/m³]");
            axisY->setRange(0, MAX_PM25);
            currentSeries = pm25Series;
        } else if (idx == 3) {
            chart->setTitle("PM10 (µg/m³)");
            axisY->setTitleText("PM10 [µg/m³]");
            axisY->setRange(0, MAX_PM10);
            currentSeries = pm10Series;
        } else if (idx == 4) {
            chart->setTitle("Promieniowanie (imp/min)");
            axisY->setTitleText("Promieniowanie [imp/min]");
            axisY->setRange(0, MAX_RADIATION);
            currentSeries = radiationSeries;
        }
        if (currentSeries) {
            chart->addSeries(currentSeries);
            currentSeries->attachAxis(axisX);
            currentSeries->attachAxis(axisY);
        }
    });

    // --- Timer do odświeżania danych ---
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSensorData);
    timer->start(1000);
}

MainWindow::~MainWindow() {}

void MainWindow::updateSensorData()
{
    if (sensorReader->readData()) {
        SensorData data = sensorReader->getData();

        // Aktualizacja etykiet
        co2Label->setText(QString("CO2: %1 ppm").arg(data.co2));
        co2TempLabel->setText(QString("Temperatura: %1 °C").arg(data.co2_temp));
        co2HumLabel->setText(QString("Wilgotność: %1 %").arg(data.co2_hum));
        pm1Label->setText(QString("PM1: %1").arg(data.pm1));
        pm25Label->setText(QString("PM2.5: %1").arg(data.pm25));
        pm10Label->setText(QString("PM10: %1").arg(data.pm10));
        radiationLabel->setText(QString("Promieniowanie: %1").arg(data.radiation));
        radiationDoseLabel->setText(QString("Dawka: %1 µSv/h").arg(data.radiation * CPS_PER_USV));

        // Interpretacja danych
        co2StatusLabel->setText(data.co2 > HIGH_CO2 ? "Wysokie" : "Normalne");
        pmStatusLabel->setText((data.pm1 > HIGH_PM1 || data.pm25 > HIGH_PM25 || data.pm10 > HIGH_PM10) ? "Wysokie" : "Normalne");
        radiationStatusLabel->setText(data.radiation > HIGH_RADIATION ? "Wysokie" : "Normalne");

        // Dodawanie punktów do serii (czas = teraz)
        qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
        co2Series->append(now, data.co2);
        pm1Series->append(now, data.pm1);
        pm25Series->append(now, data.pm25);
        pm10Series->append(now, data.pm10);
        radiationSeries->append(now, data.radiation);

        // Oś X: ostatnia godzina
        QDateTime hourAgo = QDateTime::currentDateTime().addSecs(-3600);
        axisX->setRange(hourAgo, QDateTime::currentDateTime());
    }
}
