/**
 * @file SensorDataLogger.h
 * @brief Deklaracja klasy SensorDataLogger do zapisu danych z czujników do pliku CSV.
 */

#ifndef SENSORDATALOGGER_H
#define SENSORDATALOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "SensorReader.h"

/**
 * @class SensorDataLogger
 * @brief Klasa umożliwiająca logowanie danych z czujników do pliku CSV.
 */
class SensorDataLogger
{
public:
    /**
     * @brief Konstruktor otwierający plik do zapisu.
     * @param filename Nazwa pliku CSV.
     */
    explicit SensorDataLogger(const QString& filename);

    /**
     * @brief Destruktor zamykający plik.
     */
    ~SensorDataLogger();

    /**
     * @brief Zapisuje pojedynczy rekord danych z czujników do pliku CSV.
     * @param data Struktura SensorData z danymi do zapisania.
     */
    void log(const SensorData& data);

private:
    QFile file; ///< Obsługa pliku CSV.
    QTextStream stream; ///< Strumień do zapisu tekstu.
    void writeHeaderIfNeeded(); ///< Zapisuje nagłówek, jeśli plik jest pusty.
    bool headerWritten = false; ///< Flaga informująca o zapisaniu nagłówka.
};

#endif