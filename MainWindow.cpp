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
    chartSelector = new QComboBox(this);
    chart = new QtCharts::QChart();
    chartView = new QtCharts::QChartView(chart, this);
    hour1Button = new QRadioButton(this);
    hour2Button = new QRadioButton(this);
    hour4Button = new QRadioButton(this);
    hour8Button = new QRadioButton(this);
    hour12Button = new QRadioButton(this);
    hour24Button = new QRadioButton(this);
    hour48Button = new QRadioButton(this);
    hour78Button = new QRadioButton(this);

    co2Label = new QLineEdit("--", this);
    co2TempLabel = new QLineEdit("--", this);
    co2HumLabel = new QLineEdit("--", this);
    pm1Label = new QLineEdit("--", this);
    pm25Label = new QLineEdit("--", this);
    pm10Label = new QLineEdit("--", this);
    radiationLabel = new QLineEdit("--", this);
    radiationDoseLabel = new QLineEdit("--", this);

    QString lineEditStyle = "QLineEdit { background-color: white; border: 1px solid gray; padding: 2px; min-width: 60px; max-width: 80px; }";
    QList<QLineEdit*> allFields = {co2Label, co2TempLabel, co2HumLabel, pm1Label, pm25Label, pm10Label, radiationLabel, radiationDoseLabel};
    for (auto field : allFields) {
        field->setStyleSheet(lineEditStyle);
        field->setReadOnly(true);
        field->setAlignment(Qt::AlignRight);
    }

    co2StatusLabel = new QLabel(tr("--"), this);
    pmStatusLabel = new QLabel(tr("--"), this);
    radiationStatusLabel = new QLabel(tr("--"), this);

    QHBoxLayout* statusLayout = new QHBoxLayout();

   
    QVBoxLayout* co2Col = new QVBoxLayout();
    co2Col->addWidget(new QLabel(tr("CO2"), this), 0, Qt::AlignHCenter);
    co2Col->addWidget(co2StatusLabel, 0, Qt::AlignHCenter);

    QVBoxLayout* pmCol = new QVBoxLayout();
    pmCol->addWidget(new QLabel(tr("Pyły"), this), 0, Qt::AlignHCenter);
    pmCol->addWidget(pmStatusLabel, 0, Qt::AlignHCenter);

    QVBoxLayout* radCol = new QVBoxLayout();
    radCol->addWidget(new QLabel(tr("Promieniowanie"), this), 0, Qt::AlignHCenter);
    radCol->addWidget(radiationStatusLabel, 0, Qt::AlignHCenter);

    statusLayout->addLayout(co2Col);
    statusLayout->addLayout(pmCol);
    statusLayout->addLayout(radCol);

    QFrame* statusFrame = new QFrame(this);
    statusFrame->setLayout(statusLayout);
    statusFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    statusFrame->setLineWidth(2);

    
    QGridLayout* tempHumLayout = new QGridLayout();
    tempHumLayout->addWidget(new QLabel(tr("Temperatura:"), this), 0, 0, Qt::AlignRight);
    tempHumLayout->addWidget(co2TempLabel, 0, 1);
    tempHumLayout->addWidget(new QLabel("°C", this), 0, 2, Qt::AlignLeft);
    tempHumLayout->addWidget(new QLabel(tr("Wilgotność:"), this), 1, 0, Qt::AlignRight);
    tempHumLayout->addWidget(co2HumLabel, 1, 1);
    tempHumLayout->addWidget(new QLabel("%", this), 1, 2, Qt::AlignLeft);
    QFrame* tempHumFrame = new QFrame(this);
    tempHumFrame->setLayout(tempHumLayout);
    tempHumFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    tempHumFrame->setLineWidth(2);

   
    QGridLayout* co2PmLayout = new QGridLayout();
    co2PmLayout->addWidget(new QLabel(tr("CO2:"), this), 0, 0, Qt::AlignRight);
    co2PmLayout->addWidget(co2Label, 0, 1);
    co2PmLayout->addWidget(new QLabel("ppm", this), 0, 2, Qt::AlignLeft);
    co2PmLayout->addWidget(new QLabel(tr("PM1.0:"), this), 1, 0, Qt::AlignRight);
    co2PmLayout->addWidget(pm1Label, 1, 1);
    co2PmLayout->addWidget(new QLabel("µg/m³", this), 1, 2, Qt::AlignLeft);
    co2PmLayout->addWidget(new QLabel(tr("PM2.5:"), this), 2, 0, Qt::AlignRight);
    co2PmLayout->addWidget(pm25Label, 2, 1);
    co2PmLayout->addWidget(new QLabel("µg/m³", this), 2, 2, Qt::AlignLeft);
    co2PmLayout->addWidget(new QLabel(tr("PM10:"), this), 3, 0, Qt::AlignRight);
    co2PmLayout->addWidget(pm10Label, 3, 1);
    co2PmLayout->addWidget(new QLabel("µg/m³", this), 3, 2, Qt::AlignLeft);
    QFrame* co2PmFrame = new QFrame(this);
    co2PmFrame->setLayout(co2PmLayout);
    co2PmFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    co2PmFrame->setLineWidth(2);

    
    QGridLayout* radLayout = new QGridLayout();
    radLayout->addWidget(new QLabel(tr("Promieniowanie:"), this), 0, 0, Qt::AlignRight);
    radLayout->addWidget(radiationLabel, 0, 1);
    radLayout->addWidget(new QLabel("imp/min", this), 0, 2, Qt::AlignLeft);
    radLayout->addWidget(new QLabel(tr("Dawka:"), this), 1, 0, Qt::AlignRight);
    radLayout->addWidget(radiationDoseLabel, 1, 1);
    radLayout->addWidget(new QLabel("µSv/h", this), 1, 2, Qt::AlignLeft);
    QFrame* radFrame = new QFrame(this);
    radFrame->setLayout(radLayout);
    radFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    radFrame->setLineWidth(2);

    QHBoxLayout* sensorsPanelLayout = new QHBoxLayout();
    sensorsPanelLayout->addWidget(tempHumFrame);
    sensorsPanelLayout->addSpacing(20);
    sensorsPanelLayout->addWidget(co2PmFrame);
    sensorsPanelLayout->addSpacing(20);
    sensorsPanelLayout->addWidget(radFrame);

    QHBoxLayout* topLayout = new QHBoxLayout;
    QLabel* interpretationLabel = new QLabel(tr("Interpretacja danych:"), this);
    topLayout->addWidget(interpretationLabel);
    topLayout->addStretch();
    languageSelector = new QComboBox(this);
    languageSelector->setFixedWidth(60); 
    languageSelector->addItem("PL");
    languageSelector->addItem("EN");
    topLayout->addWidget(languageSelector);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout); 
    mainLayout->addWidget(statusFrame);
    mainLayout->addWidget(new QLabel(tr("Dane z czujników:"), this));
    mainLayout->addLayout(sensorsPanelLayout);
    mainLayout->addWidget(new QLabel(tr("Analiza historyczna:"), this));
    
    
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

    
    QFrame* historyFrame = new QFrame(this);
    historyFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    historyFrame->setLineWidth(2);

    QVBoxLayout* historyMainLayout = new QVBoxLayout();

    
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

    hour1Button->setChecked(true);

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

   
    QHBoxLayout* dataSelectionLayout = new QHBoxLayout();
    QLabel* dataSelectLabel = new QLabel(tr("Dane:"), this);
    dataSelectionLayout->addWidget(dataSelectLabel);
    dataSelectionLayout->addWidget(chartSelector);
    dataSelectionLayout->addStretch();

    historyMainLayout->addLayout(timeRangeLayout);
    historyMainLayout->addLayout(dataSelectionLayout);
    historyFrame->setLayout(historyMainLayout);

    mainLayout->addWidget(historyFrame);
    mainLayout->addWidget(chartView);

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    
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
            axisY->setRange(0, 50);
            currentSeries = temperatureSeries;
        } else if (idx == 6) {
            chart->setTitle(tr("Wilgotność (%)"));
            axisY->setTitleText(tr("Wilgotność [%]"));
            axisY->setRange(0, 100);
            currentSeries = humiditySeries;
        } else if (idx == 7) {
            chart->setTitle(tr("Dawka promieniowania (µSv/h)"));
            axisY->setTitleText(tr("Dawka [µSv/h]"));
            axisY->setRange(0, 10);
            currentSeries = radiationDoseSeries;
        }
        if (currentSeries) {
            chart->addSeries(currentSeries);
            currentSeries->attachAxis(axisX);
            currentSeries->attachAxis(axisY);
        }
    });

    connect(timeMachineGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::onTimeMachineChanged);

    connect(languageSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                changeLanguage(index == 0 ? "pl" : "en");
            });

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSensorData);
    timer->start(1000);

    setMinimumSize(900, 600);
    updateInterfaceTexts();
    chartSelector->setCurrentIndex(0);
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

        
        co2StatusLabel->setText(data.co2 > HIGH_CO2 ? tr("Wysokie") : tr("Normalne"));
        pmStatusLabel->setText((data.pm1 > HIGH_PM1 || data.pm25 > HIGH_PM25 || data.pm10 > HIGH_PM10) ? tr("Wysokie") : tr("Normalne"));
        radiationStatusLabel->setText(data.radiation > HIGH_RADIATION ? tr("Wysokie") : tr("Normalne"));

        
        if (data.co2 > HIGH_CO2) {
            co2StatusLabel->setText(tr("Wysokie"));
            co2StatusLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            co2StatusLabel->setText(tr("Normalne"));
            co2StatusLabel->setStyleSheet("color: green; font-weight: bold;");
        }

        if (data.pm1 > HIGH_PM1 || data.pm25 > HIGH_PM25 || data.pm10 > HIGH_PM10) {
            pmStatusLabel->setText(tr("Wysokie"));
            pmStatusLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            pmStatusLabel->setText(tr("Normalne"));
            pmStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        }

        if (data.radiation > HIGH_RADIATION) {
            radiationStatusLabel->setText(tr("Wysokie"));
            radiationStatusLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            radiationStatusLabel->setText(tr("Normalne"));
            radiationStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        }

        
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

            
            chartStartTime = QDateTime::currentDateTime().addSecs(-timeMachineHours * 3600);
            axisX->setRange(chartStartTime, QDateTime::currentDateTime());
            

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
    
    
    QString header = in.readLine();
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        
        
        if (fields.size() < 2) continue;

        QDateTime timestamp = QDateTime::fromString(fields[0], Qt::ISODate);
        if (!timestamp.isValid() || timestamp < chartStartTime) continue;

        qint64 msecsSinceEpoch = timestamp.toMSecsSinceEpoch();
        bool ok;

        
        if (fields.size() > 1) {
            double co2 = fields[1].toDouble(&ok);
            if (ok && co2 != -1) co2Series->append(msecsSinceEpoch, co2);
        }

        if (fields.size() > 2) {  
            double temp = fields[2].toDouble(&ok);
            if (ok && temp != -1) temperatureSeries->append(msecsSinceEpoch, temp);
        }

        if (fields.size() > 3) {  
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

        if (fields.size() > 7) { 
            double radiation = fields[7].toDouble(&ok);
            if (ok && radiation != -1) {
                radiationSeries->append(msecsSinceEpoch, radiation);
                radiationDoseSeries->append(msecsSinceEpoch, radiation * CPS_PER_USV);
            }
        }

        if (fields.size() > 9) {  
            double temp = fields[9].toDouble(&ok);
            if (ok && temp != -1) temperatureSeries->append(msecsSinceEpoch, temp);
        }

        if (fields.size() > 10) { 
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
    loadHistoricalData();  
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
        
        
        this->setWindowTitle(this->windowTitle());
        
        
        updateInterfaceTexts();
    }
}

/**
 * @brief Aktualizuje wszystkie teksty w interfejsie po zmianie języka
 */
void MainWindow::updateInterfaceTexts()
{
    
    co2StatusLabel->setText(tr("--"));
    pmStatusLabel->setText(tr("--"));
    radiationStatusLabel->setText(tr("--"));

    
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

    hour1Button->setText(tr("1H"));
    hour2Button->setText(tr("2H"));
    hour4Button->setText(tr("4H"));
    hour8Button->setText(tr("8H"));
    hour12Button->setText(tr("12H"));
    hour24Button->setText(tr("24H"));
    hour48Button->setText(tr("48H"));
    hour78Button->setText(tr("78H"));

    chartSelector->setItemText(0, tr("CO2"));
    chartSelector->setItemText(1, tr("PM1.0"));
    chartSelector->setItemText(2, tr("PM2.5"));
    chartSelector->setItemText(3, tr("PM10"));
    chartSelector->setItemText(4, tr("Promieniowanie"));
    chartSelector->setItemText(5, tr("Temperatura"));
    chartSelector->setItemText(6, tr("Wilgotność"));
    chartSelector->setItemText(7, tr("Dawka promieniowania"));
    
    updateChartTitles();
    
    axisX->setTitleText(tr("Czas"));
    
    updateSensorData();

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

