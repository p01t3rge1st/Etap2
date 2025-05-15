#ifndef SENSORREADER_H
#define SENSORREADER_H

#include <string>
#include <termios.h>

/**
 * @struct SensorData
 * @brief Struktura przechowująca dane odczytane z czujników.
 */
struct SensorData {
    int co2 = -1; ///< Poziom CO2 w ppm.
    int co2_temp = -1; ///< Temperatura CO2 w stopniach Celsjusza.
    int co2_hum = -1; ///< Wilgotność CO2 w procentach.
    int pm1 = -1; ///< Poziom pyłu PM1.0 w µg/m³.
    int pm25 = -1; ///< Poziom pyłu PM2.5 w µg/m³.
    int pm10 = -1; ///< Poziom pyłu PM10 w µg/m³.
    int radiation = -1; ///< Liczba zliczeń promieniowania (CPM).
    float radiation_dose_per_hour = -1.0; ///< Dawka promieniowania na godzinę w µSv/h.
};

/**
 * @class SensorReader
 * @brief Klasa do obsługi czujników przez port szeregowy.
 */
class SensorReader {
public:
    /**
     * @brief Konstruktor klasy SensorReader.
     * @param portname Nazwa portu szeregowego.
     * @param baudrate Prędkość transmisji (domyślnie B9600).
     */
    SensorReader(const std::string& portname, int baudrate = B9600);

    /**
     * @brief Destruktor klasy SensorReader.
     */
    ~SensorReader();

    /**
     * @brief Otwiera port szeregowy.
     * @return true, jeśli port został otwarty pomyślnie, false w przeciwnym razie.
     */
    bool openPort();

    /**
     * @brief Zamyka port szeregowy.
     */
    void closePort();

    /**
     * @brief Odczytuje dane z portu szeregowego.
     * @return true, jeśli dane zostały odczytane pomyślnie, false w przeciwnym razie.
     */
    bool readData();

    /**
     * @brief Zwraca ostatnio odczytane dane z czujników.
     * @return Struktura SensorData zawierająca dane z czujników.
     */
    SensorData getData() const;

private:
    std::string portname; ///< Nazwa portu szeregowego.
    int baudrate; ///< Prędkość transmisji.
    int serial_port; ///< Uchwyt do portu szeregowego.
    char buffer[256]; ///< Bufor do przechowywania danych z portu szeregowego.
    std::string serial_data; ///< Dane odczytane z portu szeregowego.
    SensorData data; ///< Struktura przechowująca dane z czujników.

    /**
     * @brief Parsuje surowe dane z czujników.
     * @param raw Surowe dane w formie ciągu znaków.
     * @return Struktura SensorData zawierająca sparsowane dane.
     */
    SensorData parseSensorData(const std::string& raw);
};

#endif
