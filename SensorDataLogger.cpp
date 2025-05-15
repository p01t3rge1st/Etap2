/**
 * @file SensorDataLogger.cpp
 * @brief Implementacja klasy zapisującej dane z czujników do pliku CSV.
 */

#include "SensorDataLogger.h"

/** @brief Współczynnik konwersji CPS na µSv/h */
const float CPS_PER_USV = 0.0037f;

/**
 * @brief Konstruktor klasy logger'a
 * @param filename Nazwa pliku CSV do zapisu danych
 */
SensorDataLogger::SensorDataLogger(const QString& filename)
    : file(filename)
{
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    stream.setDevice(&file);
    writeHeaderIfNeeded();
}

/**
 * @brief Destruktor zamykający plik
 */
SensorDataLogger::~SensorDataLogger()
{
    file.close();
}

/**
 * @brief Zapisuje nagłówek CSV jeśli plik jest pusty
 */
void SensorDataLogger::writeHeaderIfNeeded()
{
    if (file.size() == 0) {
        stream << "Data i czas,CO2,Temperatura,Wilgotność,PM1.0,PM2.5,PM10,Promieniowanie,Dawka/h,Temperatura_CO2,Promieniowanie_uSv\n";
        stream.flush();
        headerWritten = true;
    }
}

/**
 * @brief Zapisuje jeden rekord pomiarowy do pliku CSV
 * @param data Struktura zawierająca dane z czujników
 */
void SensorDataLogger::log(const SensorData& data)
{
    QString line = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
        .arg(data.co2)
        .arg(data.co2_temp)
        .arg(data.co2_hum)
        .arg(data.pm1)
        .arg(data.pm25)
        .arg(data.pm10)
        .arg(data.radiation)
        .arg(data.radiation_dose_per_hour)
        .arg(data.co2_temp)
        .arg(data.radiation * CPS_PER_USV);
    stream << line;
    stream.flush();
}