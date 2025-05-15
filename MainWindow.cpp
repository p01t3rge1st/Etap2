/**
 * @file MainWindow.cpp
 * @brief Implementacja głównego okna aplikacji monitorującej czujniki środowiskowe
 */

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
#include <QLineEdit>

/** Stałe wartości progowe dla pomiarów */
const int HIGH_CO2 = 1200;        ///< Próg alarmowy CO2 (ppm)
const int HIGH_PM1 = 25;          ///< Próg alarmowy PM1.0 (µg/m³)
const int HIGH_PM25 = 25;         ///< Próg alarmowy PM2.5 (µg/m³)
const int HIGH_PM10 = 50;         ///< Próg alarmowy PM10 (µg/m³)
const int HIGH_RADIATION = 30;     ///< Próg alarmowy promieniowania (imp/min)
const float CPS_PER_USV = 0.0037; ///< Współczynnik konwersji CPS na µSv/h

/** Zakresy wykresów */
const int MAX_CO2 = 2000;         ///< Maksymalna wartość CO2 na wykresie
const int MAX_PM1 = 50;           ///< Maksymalna wartość PM1.0 na wykresie
const int MAX_PM25 = 50;          ///< Maksymalna wartość PM2.5 na wykresie
const int MAX_PM10 = 100;         ///< Maksymalna wartość PM10 na wykresie
const int MAX_RADIATION = 100;     ///< Maksymalna wartość promieniowania na wykresie

/**
 * @brief Konstruktor głównego okna
 * @param reader Wskaźnik do obiektu czytającego dane z czujników
 * @param parent Wskaźnik do widgetu nadrzędnego
 */
MainWindow::MainWindow(SensorReader* reader, QWidget *parent)
    : QMainWindow(parent), sensorReader(reader), logger("dane_czujnikow.csv"), timeMachineHours(1)
{
    // Tworzenie pól tekstowych zamiast etykiet
    co2Label = new QLineEdit("--", this);
    co2TempLabel = new QLineEdit("--", this);
    co2HumLabel = new QLineEdit("--", this);
    pm1Label = new QLineEdit("--", this);
    pm25Label = new QLineEdit("--", this);
    pm10Label = new QLineEdit("--", this);
    radiationLabel = new QLineEdit("--", this);
    radiationDoseLabel = new QLineEdit("--", this);

    // Styl dla wszystkich pól
    QString lineEditStyle = "QLineEdit { "
                          "background-color: white; "
                          "border: 1px solid gray; "
                          "padding: 2px; "
                          "min-width: 60px; "
                          "max-width: 80px; "
                          "}";

    // Zastosuj styl i właściwości do wszystkich pól
    QList<QLineEdit*> allFields = {co2Label, co2TempLabel, co2HumLabel, 
                                  pm1Label, pm25Label, pm10Label,
                                  radiationLabel, radiationDoseLabel};
    
    for (auto field : allFields) {
        field->setStyleSheet(lineEditStyle);
        field->setReadOnly(true);
        field->setAlignment(Qt::AlignRight);
    }

    co2StatusLabel = new QLabel(tr("--"), this);
    pmStatusLabel = new QLabel(tr("--"), this);
    radiationStatusLabel = new QLabel(tr("--"), this);

    QGridLayout* sensorDataLayout = new QGridLayout();

    // Dodaj pola tekstowe (białe prostokąty)
    sensorDataLayout->addWidget(co2Label, 0, 1);
    sensorDataLayout->addWidget(co2TempLabel, 1, 1);
    sensorDataLayout->addWidget(co2HumLabel, 2, 1);
    sensorDataLayout->addWidget(pm1Label, 3, 1);
    sensorDataLayout->addWidget(pm25Label, 4, 1);
    sensorDataLayout->addWidget(pm10Label, 5, 1);
    sensorDataLayout->addWidget(radiationLabel, 6, 1);
    sensorDataLayout->addWidget(radiationDoseLabel, 7, 1);

    // Dodaj etykiety z opisami po lewej
    sensorDataLayout->addWidget(new QLabel(tr("CO2:"), this), 0, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("Temperatura:"), this), 1, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("Wilgotność:"), this), 2, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("PM1.0:"), this), 3, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("PM2.5:"), this), 4, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("PM10:"), this), 5, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("Promieniowanie:"), this), 6, 0, Qt::AlignRight);
    sensorDataLayout->addWidget(new QLabel(tr("Dawka:"), this), 7, 0, Qt::AlignRight);

    // Dodaj jednostki po prawej
    sensorDataLayout->addWidget(new QLabel("ppm", this), 0, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("°C", this), 1, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("%", this), 2, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("µg/m³", this), 3, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("µg/m³", this), 4, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("µg/m³", this), 5, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("imp/min", this), 6, 2, Qt::AlignLeft);
    sensorDataLayout->addWidget(new QLabel("µSv/h", this), 7, 2, Qt::AlignLeft);

    QFrame* sensorDataFrame = new QFrame(this);
    sensorDataFrame->setLayout(sensorDataLayout);
    sensorDataFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    sensorDataFrame->setLineWidth(2);

    QGridLayout* interpretationLayout = new QGridLayout();
    interpretationLayout->addWidget(new QLabel(tr("CO2 Status:"), this), 0, 0, Qt::AlignRight);
    interpretationLayout->addWidget(co2StatusLabel, 0, 1, Qt::AlignLeft);
    interpretationLayout->addWidget(new QLabel(tr("PM Status:"), this), 1, 0, Qt::AlignRight);
    interpretationLayout->addWidget(pmStatusLabel, 1, 1, Qt::AlignLeft);
    interpretationLayout->addWidget(new QLabel(tr("Prom. Status:"), this), 2, 0, Qt::AlignRight);
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
    axisX->setTitleText(tr("Czas"));
    axisY = new QtCharts::QValueAxis;

    chart = new QtCharts::QChart();
    chart->addSeries(co2Series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    co2Series->attachAxis(axisX);
    co2Series->attachAxis(axisY);
    chart->setTitle(tr("CO2 (ppm)"));
    axisY->setTitleText(tr("CO2 [ppm]"));
    axisY->setRange(0, MAX_CO2);

    chartView = new QtCharts::QChartView(chart);
    chartView->setMinimumSize(700, 400);

    chartSelector = new QComboBox(this);
    chartSelector->addItem(tr("CO2"));
    chartSelector->addItem(tr("PM1.0"));
    chartSelector->addItem(tr("PM2.5"));
    chartSelector->addItem(tr("PM10"));
    chartSelector->addItem(tr("Promieniowanie"));
    chartSelector->addItem(tr("Temperatura"));
    chartSelector->addItem(tr("Wilgotność"));
    chartSelector->addItem(tr("Dawka promieniowania"));

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
    QLabel* timeRangeLabel = new QLabel(tr("Zakres:"), this);
    timeRangeLayout->addWidget(timeRangeLabel);
    
    hour1Button = new QRadioButton(tr("1H"), this);
    hour2Button = new QRadioButton(tr("2H"), this);
    hour4Button = new QRadioButton(tr("4H"), this);
    hour8Button = new QRadioButton(tr("8H"), this);
    hour12Button = new QRadioButton(tr("12H"), this);
    hour24Button = new QRadioButton(tr("24H"), this);
    hour48Button = new QRadioButton(tr("48H"), this);
    hour78Button = new QRadioButton(tr("78H"), this);

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
    QLabel* dataSelectLabel = new QLabel(tr("Dane:"), this);
    dataSelectionLayout->addWidget(dataSelectLabel);
    dataSelectionLayout->addWidget(chartSelector);
    dataSelectionLayout->addStretch();

    // Układanie elementów w panelu
    historyMainLayout->addLayout(timeRangeLayout);
    historyMainLayout->addLayout(dataSelectionLayout);
    
    historyFrame->setLayout(historyMainLayout);

    // --- Panel językowy ---
    QHBoxLayout* topLayout = new QHBoxLayout;
    
    // Lewy element (Interpretacja danych)
    QLabel* interpretationLabel = new QLabel(tr("Interpretacja danych:"), this);
    topLayout->addWidget(interpretationLabel);
    
    // Elastyczny odstęp
    topLayout->addStretch();
    
    // Prawy element (selektor języka)
    languageSelector = new QComboBox(this);
    languageSelector->setFixedWidth(60);  // Mały rozmiar
    languageSelector->addItem("PL");
    languageSelector->addItem("EN");
    topLayout->addWidget(languageSelector);

    // --- UKŁAD GŁÓWNY ---
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);  // Dodaj górny layout
    mainLayout->addWidget(interpretationFrame);
    mainLayout->addWidget(new QLabel(tr("Dane z czujników:"), this));
    mainLayout->addWidget(sensorDataFrame);
    mainLayout->addWidget(new QLabel(tr("Analiza historyczna:"), this));
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
            chart->setTitle(tr("CO2 (ppm)"));
            axisY->setTitleText(tr("CO2 [ppm]"));
            axisY->setRange(0, MAX_CO2);
            currentSeries = co2Series;
        } else if (idx == 1) {
            chart->setTitle(tr("PM1.0 (µg/m³)"));
            axisY->setTitleText(tr("PM1.0 [µg/m³]"));
            axisY->setRange(0, MAX_PM1);
            currentSeries = pm1Series;
        } else if (idx == 2) {
            chart->setTitle(tr("PM2.5 (µg/m³)"));
            axisY->setTitleText(tr("PM2.5 [µg/m³]"));
            axisY->setRange(0, MAX_PM25);
            currentSeries = pm25Series;
        } else if (idx == 3) {
            chart->setTitle(tr("PM10 (µg/m³)"));
            axisY->setTitleText(tr("PM10 [µg/m³]"));
            axisY->setRange(0, MAX_PM10);
            currentSeries = pm10Series;
        } else if (idx == 4) {
            chart->setTitle(tr("Promieniowanie (imp/min)"));
            axisY->setTitleText(tr("Promieniowanie [imp/min]"));
            axisY->setRange(0, MAX_RADIATION);
            currentSeries = radiationSeries;
        } else if (idx == 5) {
            chart->setTitle(tr("Temperatura (°C)"));
            axisY->setTitleText(tr("Temperatura [°C]"));
            axisY->setRange(0, 50);  // Adjust range as needed
            currentSeries = temperatureSeries;
        } else if (idx == 6) {
            chart->setTitle(tr("Wilgotność (%)"));
            axisY->setTitleText(tr("Wilgotność [%]"));
            axisY->setRange(0, 100);
            currentSeries = humiditySeries;
        } else if (idx == 7) {
            chart->setTitle(tr("Dawka promieniowania (µSv/h)"));
            axisY->setTitleText(tr("Dawka [µSv/h]"));
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
    connect(timeMachineGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::onTimeMachineChanged);

    // Dodaj połączenie dla selektora języka
    connect(languageSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                changeLanguage(index == 0 ? "pl" : "en");
            });

    // --- Timer do odświeżania danych ---
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSensorData);
    timer->start(1000);
}

MainWindow::~MainWindow() {}

/**
 * @brief Aktualizuje wyświetlane dane z czujników
 * 
 * Odczytuje nowe dane, waliduje je i aktualizuje interfejs użytkownika
 */
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

        co2Label->setText(QString::number(data.co2));
        co2TempLabel->setText(QString::number(data.co2_temp));
        co2HumLabel->setText(QString::number(data.co2_hum));
        pm1Label->setText(QString::number(data.pm1));
        pm25Label->setText(QString::number(data.pm25));
        pm10Label->setText(QString::number(data.pm10));
        radiationLabel->setText(QString::number(data.radiation));
        radiationDoseLabel->setText(QString::number(data.radiation * CPS_PER_USV));

        // Interpretacja danych
        co2StatusLabel->setText(data.co2 > HIGH_CO2 ? tr("Wysokie") : tr("Normalne"));
        pmStatusLabel->setText((data.pm1 > HIGH_PM1 || data.pm25 > HIGH_PM25 || data.pm10 > HIGH_PM10) ? tr("Wysokie") : tr("Normalne"));
        radiationStatusLabel->setText(data.radiation > HIGH_RADIATION ? tr("Wysokie") : tr("Normalne"));

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

/**
 * @brief Wczytuje dane historyczne z pliku CSV
 * 
 * Odczytuje zapisane dane z czujników i aktualizuje serie danych na wykresach
 */
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

/**
 * @brief Obsługuje zmianę zakresu czasu w widoku historycznym
 * @param id Nowy zakres czasu w godzinach
 */
void MainWindow::onTimeMachineChanged(int id)
{
    timeMachineHours = id;
    loadHistoricalData();  // Przeładuj dane historyczne
    chartStartTime = QDateTime::currentDateTime().addSecs(-timeMachineHours * 3600);
    axisX->setRange(chartStartTime, QDateTime::currentDateTime());
}

/**
 * @brief Zmienia język interfejsu aplikacji
 * @param language Kod języka ("pl" lub "en")
 */
void MainWindow::changeLanguage(const QString &language)
{
    qApp->removeTranslator(&translator);
    if (translator.load(":/translations/wds_" + language)) {
        qApp->installTranslator(&translator);
        
        // Wymuś odświeżenie wszystkich tekstów
        this->setWindowTitle(this->windowTitle());
        
        // Odśwież wszystkie etykiety
        updateInterfaceTexts();
    }
}

/**
 * @brief Aktualizuje wszystkie teksty w interfejsie po zmianie języka
 */
void MainWindow::updateInterfaceTexts()
{
    // Aktualizacja etykiet statusu i pomiarów
    co2StatusLabel->setText(tr("--"));
    pmStatusLabel->setText(tr("--"));
    radiationStatusLabel->setText(tr("--"));

    // Aktualizacja etykiet opisowych
    foreach(QLabel* label, findChildren<QLabel*>()) {
        if (label->text() == "CO2:") label->setText(tr("CO2:"));
        else if (label->text() == "Temperatura:") label->setText(tr("Temperatura:"));
        else if (label->text() == "Wilgotność:") label->setText(tr("Wilgotność:"));
        else if (label->text() == "PM1.0:") label->setText(tr("PM1.0:"));
        else if (label->text() == "PM2.5:") label->setText(tr("PM2.5:"));
        else if (label->text() == "PM10:") label->setText(tr("PM10:"));
        else if (label->text() == "Promieniowanie:") label->setText(tr("Promieniowanie:"));
        else if (label->text() == "Dawka:") label->setText(tr("Dawka:"));
        else if (label->text() == "CO2 Status:") label->setText(tr("CO2 Status:"));
        else if (label->text() == "PM Status:") label->setText(tr("PM Status:"));
        else if (label->text() == "Prom. Status:") label->setText(tr("Prom. Status:"));
        else if (label->text() == "Interpretacja danych:") label->setText(tr("Interpretacja danych:"));
        else if (label->text() == "Dane z czujników:") label->setText(tr("Dane z czujników:"));
        else if (label->text() == "Analiza historyczna:") label->setText(tr("Analiza historyczna:"));
        else if (label->text() == "Zakres:") label->setText(tr("Zakres:"));
        else if (label->text() == "Dane:") label->setText(tr("Dane:"));
    }

    // Aktualizacja przycisków zakresu czasu
    hour1Button->setText(tr("1H"));
    hour2Button->setText(tr("2H"));
    hour4Button->setText(tr("4H"));
    hour8Button->setText(tr("8H"));
    hour12Button->setText(tr("12H"));
    hour24Button->setText(tr("24H"));
    hour48Button->setText(tr("48H"));
    hour78Button->setText(tr("78H"));

    // Aktualizacja selektora wykresu
    chartSelector->setItemText(0, tr("CO2"));
    chartSelector->setItemText(1, tr("PM1.0"));
    chartSelector->setItemText(2, tr("PM2.5"));
    chartSelector->setItemText(3, tr("PM10"));
    chartSelector->setItemText(4, tr("Promieniowanie"));
    chartSelector->setItemText(5, tr("Temperatura"));
    chartSelector->setItemText(6, tr("Wilgotność"));
    chartSelector->setItemText(7, tr("Dawka promieniowania"));
    
    // Aktualizacja tytułów wykresów
    updateChartTitles();
    
    // Aktualizacja osi wykresu
    axisX->setTitleText(tr("Czas"));
    
    // Wymuś odświeżenie danych
    updateSensorData();

    // Wymuś przerysowanie całego widgetu
    update();
}

/**
 * @brief Aktualizuje tytuły wykresów na podstawie wybranej serii danych
 */
void MainWindow::updateChartTitles()
{
    int idx = chartSelector->currentIndex();
    if (idx == 0) {
        chart->setTitle(tr("CO2 (ppm)"));
        axisY->setTitleText(tr("CO2 [ppm]"));
    } else if (idx == 1) {
        chart->setTitle(tr("PM1.0 (µg/m³)"));
        axisY->setTitleText(tr("PM1.0 [µg/m³]"));
    } else if (idx == 2) {
        chart->setTitle(tr("PM2.5 (µg/m³)"));
        axisY->setTitleText(tr("PM2.5 [µg/m³]"));
    } else if (idx == 3) {
        chart->setTitle(tr("PM10 (µg/m³)"));
        axisY->setTitleText(tr("PM10 [µg/m³]"));
    } else if (idx == 4) {
        chart->setTitle(tr("Promieniowanie (imp/min)"));
        axisY->setTitleText(tr("Promieniowanie [imp/min]"));
    } else if (idx == 5) {
        chart->setTitle(tr("Temperatura (°C)"));
        axisY->setTitleText(tr("Temperatura [°C]"));
    } else if (idx == 6) {
        chart->setTitle(tr("Wilgotność (%)"));
        axisY->setTitleText(tr("Wilgotność [%]"));
    } else if (idx == 7) {
        chart->setTitle(tr("Dawka promieniowania (µSv/h)"));
        axisY->setTitleText(tr("Dawka [µSv/h]"));
    }
}
