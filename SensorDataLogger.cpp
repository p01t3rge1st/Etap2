/**
 * @file SensorDataLogger.cpp
 * @brief Implementacja klasy SensorDataLogger do zapisu danych z czujników do pliku CSV.
 */

#include "SensorDataLogger.h"

SensorDataLogger::SensorDataLogger(const QString& filename)
    : file(filename)
{
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        // Obsłuż błąd otwarcia pliku
        return;
    }
    stream.setDevice(&file);
    writeHeaderIfNeeded();
}

SensorDataLogger::~SensorDataLogger()
{
    file.close();
}

void SensorDataLogger::writeHeaderIfNeeded()
{
    /**
     * @brief Zapisuje nagłówek do pliku CSV, jeśli plik jest pusty.
     */
    if (file.size() == 0) {
        stream << "Data i czas,CO2,Temperatura,Wilgotność,PM1.0,PM2.5,PM10,Promieniowanie,Dawka/h\n";
        stream.flush();
        headerWritten = true;
    }
}

void SensorDataLogger::log(const SensorData& data)
{
    /**
     * @brief Zapisuje pojedynczy rekord danych z czujników do pliku CSV.
     * @param data Struktura SensorData z danymi do zapisania.
     */
    QString line = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
        .arg(data.co2)
        .arg(data.co2_temp)
        .arg(data.co2_hum)
        .arg(data.pm1)
        .arg(data.pm25)
        .arg(data.pm10)
        .arg(data.radiation)
        .arg(data.radiation_dose_per_hour);
    stream << line;
    stream.flush();
}