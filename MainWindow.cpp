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
#include <QButtonGroup>
#include <QRadioButton>
#include <QHBoxLayout>

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
    : QMainWindow(parent), sensorReader(reader), logger("dane_czujnikow.csv"), timeMachineHours(1)
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
    sensorDataLayout->addWidget(co2Label, 0, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(co2TempLabel, 1, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(co2HumLabel, 2, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(pm1Label, 3, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(pm25Label, 4, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(pm10Label, 5, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(radiationLabel, 6, 1, Qt::AlignLeft);
    sensorDataLayout->addWidget(radiationDoseLabel, 7, 1, Qt::AlignLeft);

    // Stałe etykiety opisowe
    sensorDataLayout->addWidget(new QLabel("CO2:", this), 0, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("Temperatura:", this), 1, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("Wilgotność:", this), 2, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("PM1.0:", this), 3, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("PM2.5:", this), 4, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("PM10:", this), 5, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("Promieniowanie:", this), 6, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel("Dawka:", this), 7, 0, Qt::AlignRight);

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
    temperatureSeries = new QtCharts::QLineSeries();
    humiditySeries = new QtCharts::QLineSeries();
    radiationDoseSeries = new QtCharts::QLineSeries();

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
    chartSelector->addItem("Temperatura");
    chartSelector->addItem("Wilgotność");
    chartSelector->addItem("Dawka promieniowania");

    // Po utworzeniu serii i przed timerem
    loadHistoricalData();  // Wczytaj dane historyczne
    
    // Ustaw początkowy zakres osi X
    axisX->setRange(chartStartTime, QDateTime::currentDateTime());

    // --- Panel Analizy Historycznej ---
    QFrame* historyFrame = new QFrame(this);
    historyFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    historyFrame->setLineWidth(2);

    QVBoxLayout* historyMainLayout = new QVBoxLayout();
    
    // Zakres czasu
    QHBoxLayout* timeRangeLayout = new QHBoxLayout();
    QLabel* timeRangeLabel = new QLabel("Zakres:", this);
    timeRangeLayout->addWidget(timeRangeLabel);
    
    hour1Button = new QRadioButton("1H", this);
    hour2Button = new QRadioButton("2H", this);
    hour4Button = new QRadioButton("4H", this);
    hour8Button = new QRadioButton("8H", this);
    hour12Button = new QRadioButton("12H", this);
    hour24Button = new QRadioButton("24H", this);
    hour48Button = new QRadioButton("48H", this);
    hour78Button = new QRadioButton("78H", this);

    hour1Button->setChecked(true);  // Domyślnie 1H

    timeMachineGroup = new QButtonGroup(this);
    timeMachineGroup->addButton(hour1Button, 1);
    timeMachineGroup->addButton(hour2Button, 2);
    timeMachineGroup->addButton(hour4Button, 4);
    timeMachineGroup->addButton(hour8Button, 8);
    timeMachineGroup->addButton(hour12Button, 12);
    timeMachineGroup->addButton(hour24Button, 24);
    timeMachineGroup->addButton(hour48Button, 48);
    timeMachineGroup->addButton(hour78Button, 78);

    timeRangeLayout->addWidget(hour1Button);
    timeRangeLayout->addWidget(hour2Button);
    timeRangeLayout->addWidget(hour4Button);
    timeRangeLayout->addWidget(hour8Button);
    timeRangeLayout->addWidget(hour12Button);
    timeRangeLayout->addWidget(hour24Button);
    timeRangeLayout->addWidget(hour48Button);
    timeRangeLayout->addWidget(hour78Button);
    timeRangeLayout->addStretch();

    // Wybór danych
    QHBoxLayout* dataSelectionLayout = new QHBoxLayout();
    QLabel* dataSelectLabel = new QLabel("Dane:", this);
    dataSelectionLayout->addWidget(dataSelectLabel);
    dataSelectionLayout->addWidget(chartSelector);
    dataSelectionLayout->addStretch();

    // Układanie elementów w panelu
    historyMainLayout->addLayout(timeRangeLayout);
    historyMainLayout->addLayout(dataSelectionLayout);
    
    historyFrame->setLayout(historyMainLayout);

    // --- UKŁAD GŁÓWNY ---
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel("Interpretacja danych:", this));
    mainLayout->addWidget(interpretationFrame);
    mainLayout->addWidget(new QLabel("Dane z czujników:", this));
    mainLayout->addWidget(sensorDataFrame);
    mainLayout->addWidget(new QLabel("Analiza historyczna:", this));  // Changed to match style
    mainLayout->addWidget(historyFrame);
    mainLayout->addWidget(chartView);

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // --- Przełączanie wykresów ---
    connect(chartSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        QList<QtCharts::QLineSeries*> allSeries = {
            co2Series, pm1Series, pm25Series, pm10Series, 
            radiationSeries, temperatureSeries, humiditySeries, 
            radiationDoseSeries
        };
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
        } else if (idx == 5) {
            chart->setTitle("Temperatura (°C)");
            axisY->setTitleText("Temperatura [°C]");
            axisY->setRange(0, 50);  // Adjust range as needed
            currentSeries = temperatureSeries;
        } else if (idx == 6) {
            chart->setTitle("Wilgotność (%)");
            axisY->setTitleText("Wilgotność [%]");
            axisY->setRange(0, 100);
            currentSeries = humiditySeries;
        } else if (idx == 7) {
            chart->setTitle("Dawka promieniowania (µSv/h)");
            axisY->setTitleText("Dawka [µSv/h]");
            axisY->setRange(0, 10);  // Adjust range as needed
            currentSeries = radiationDoseSeries;
        }
        if (currentSeries) {
            chart->addSeries(currentSeries);
            currentSeries->attachAxis(axisX);
            currentSeries->attachAxis(axisY);
        }
    });

    // Połącz sygnał zmiany zakresu czasu
    connect(timeMachineGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::onTimeMachineChanged);

    // --- Timer do odświeżania danych ---
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSensorData);
    timer->start(1000);
}

MainWindow::~MainWindow() {}

void MainWindow::updateSensorData()
{
    static SensorData lastValidData = {0}; 

    if (sensorReader->readData()) {
        SensorData data = sensorReader->getData();
        
        // Add CRC check
        if (!data.crcValid) {
            qDebug() << "CRC check failed, using last valid data";
            data = lastValidData;
        }
        else if (data.co2 == -1 || data.co2_temp == -1 || data.co2_hum == -1 || 
            data.pm1 == -1 || data.pm25 == -1 || data.pm10 == -1 || 
            data.radiation == -1) {
            data = lastValidData;
        } else {
            lastValidData = data;
        }

        // Aktualizacja etykiet bez duplikacji tekstu
        co2Label->setText(QString("%1 ppm").arg(data.co2));
        co2TempLabel->setText(QString("%1 °C").arg(data.co2_temp));
        co2HumLabel->setText(QString("%1 %").arg(data.co2_hum));
        pm1Label->setText(QString("%1 µg/m³").arg(data.pm1));
        pm25Label->setText(QString("%1 µg/m³").arg(data.pm25));
        pm10Label->setText(QString("%1 µg/m³").arg(data.pm10));
        radiationLabel->setText(QString("%1 imp/min").arg(data.radiation));
        radiationDoseLabel->setText(QString("%1 µSv/h").arg(data.radiation * CPS_PER_USV));

        // Interpretacja danych
        co2StatusLabel->setText(data.co2 > HIGH_CO2 ? "Wysokie" : "Normalne");
        pmStatusLabel->setText((data.pm1 > HIGH_PM1 || data.pm25 > HIGH_PM25 || data.pm10 > HIGH_PM10) ? "Wysokie" : "Normalne");
        radiationStatusLabel->setText(data.radiation > HIGH_RADIATION ? "Wysokie" : "Normalne");

        // Dodawanie punktów do serii tylko jeśli dane są prawidłowe
        if (data.co2 != -1 && data.pm1 != -1 && data.pm25 != -1 && 
            data.pm10 != -1 && data.radiation != -1) {
            qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
            co2Series->append(now, data.co2);
            pm1Series->append(now, data.pm1);
            pm25Series->append(now, data.pm25);
            pm10Series->append(now, data.pm10);
            radiationSeries->append(now, data.radiation);
            temperatureSeries->append(now, data.co2_temp);
            humiditySeries->append(now, data.co2_hum);
            radiationDoseSeries->append(now, data.radiation * CPS_PER_USV);

            // Aktualizuj zakres wykresu
            chartStartTime = QDateTime::currentDateTime().addSecs(-timeMachineHours * 3600);
            axisX->setRange(chartStartTime, QDateTime::currentDateTime());
            
            // Zapisz do CSV tylko prawidłowe dane
            logger.log(data);
        }
    }
}

void MainWindow::loadHistoricalData() 
{
    // Clear all series before loading
    co2Series->clear();
    pm1Series->clear();
    pm25Series->clear();
    pm10Series->clear();
    radiationSeries->clear();
    temperatureSeries->clear();
    humiditySeries->clear();
    radiationDoseSeries->clear();

    QFile file("dane_czujnikow.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Nie można otworzyć pliku CSV";
        return;
    }

    QTextStream in(&file);
    chartStartTime = QDateTime::currentDateTime().addSecs(-timeMachineHours * 3600);
    
    // Skip header
    QString header = in.readLine();
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        
        // Basic validation - need at least timestamp and one value
        if (fields.size() < 2) continue;

        QDateTime timestamp = QDateTime::fromString(fields[0], Qt::ISODate);
        if (!timestamp.isValid() || timestamp < chartStartTime) continue;

        qint64 msecsSinceEpoch = timestamp.toMSecsSinceEpoch();
        bool ok;

        // Try to read each value, skip if invalid
        if (fields.size() > 1) {
            double co2 = fields[1].toDouble(&ok);
            if (ok && co2 != -1) co2Series->append(msecsSinceEpoch, co2);
        }

        if (fields.size() > 2) {  // Temperature field
            double temp = fields[2].toDouble(&ok);
            if (ok && temp != -1) temperatureSeries->append(msecsSinceEpoch, temp);
        }

        if (fields.size() > 3) {  // Humidity field
            double hum = fields[3].toDouble(&ok);
            if (ok && hum != -1) humiditySeries->append(msecsSinceEpoch, hum);
        }

        if (fields.size() > 4) {
            double pm1 = fields[4].toDouble(&ok);
            if (ok && pm1 != -1) pm1Series->append(msecsSinceEpoch, pm1);
        }

        if (fields.size() > 5) {
            double pm25 = fields[5].toDouble(&ok);
            if (ok && pm25 != -1) pm25Series->append(msecsSinceEpoch, pm25);
        }

        if (fields.size() > 6) {
            double pm10 = fields[6].toDouble(&ok);
            if (ok && pm10 != -1) pm10Series->append(msecsSinceEpoch, pm10);
        }

        if (fields.size() > 7) {  // Radiation for dose calculation
            double radiation = fields[7].toDouble(&ok);
            if (ok && radiation != -1) {
                radiationSeries->append(msecsSinceEpoch, radiation);
                radiationDoseSeries->append(msecsSinceEpoch, radiation * CPS_PER_USV);
            }
        }

        if (fields.size() > 9) {  // Temperature column
            double temp = fields[9].toDouble(&ok);
            if (ok && temp != -1) temperatureSeries->append(msecsSinceEpoch, temp);
        }

        if (fields.size() > 10) {  // Radiation uSv column
            double radiationUsv = fields[10].toDouble(&ok);
            if (ok && radiationUsv != -1) radiationDoseSeries->append(msecsSinceEpoch, radiationUsv);
        }
    }
    
    file.close();
}

void MainWindow::onTimeMachineChanged(int id)
{
    timeMachineHours = id;
    loadHistoricalData();  // Przeładuj dane historyczne
    chartStartTime = QDateTime::currentDateTime().addSecs(-timeMachineHours * 3600);
    axisX->setRange(chartStartTime, QDateTime::currentDateTime());
}
