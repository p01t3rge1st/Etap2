/**
 * @file main.cpp
 * @brief Główna funkcja programu.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "SensorReader.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <regex>
#include <termios.h>
#include <QApplication>
#include "MainWindow.h"

/**
 * @brief Wyświetla pasek ładowania w terminalu.
 * 
 */

/**
 * @brief Główna funkcja programu.
 * 
 * Inicjalizuje obiekt SensorReader, otwiera port szeregowy i uruchamia aplikację Qt.
 * 
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica argumentów wiersza poleceń.
 * @return int Kod wyjścia programu.
 */
int main(int argc, char *argv[]) {

    const char* portname = "/dev/ttyUSB0";
    SensorReader reader(portname);


    // Inicjalizacja czujników – czekaj aż CO2 przestanie być -1
    std::cout << "Inicjalizacja czujników... Proszę czekać (ok. 20 sekund)..." << std::endl;
    int timeout = 45; // maksymalnie 30 prób (ok. 30 sekund)
    bool initialized = false;
    if(reader.openPort() == false) {
        std::cerr << "Nie można otworzyć portu szeregowego!" << std::endl;
        return 1;
    }
    else{
        std::cout << "Port szeregowy otwarty pomyślnie." << std::endl;
    }
    for (int i = 0; i < timeout; ++i) {
        if (reader.readData()) {
            SensorData data = reader.getData();
            if (data.co2 != -1) {
                initialized = true;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (!initialized) {
        std::cout << "Nie udało się zainicjalizować czujników (brak odczytu CO2)!" << std::endl;
        return 2;
    }
    std::cout << "Czujniki zainicjalizowane. Uruchamianie aplikacji..." << std::endl;

    std::cout << "Debug: przed QApplication" << std::endl;
    QApplication app(argc, argv);

    std::cout << "Debug: przed MainWindow" << std::endl;
    MainWindow mainWindow(&reader);

    std::cout << "Debug: przed show()" << std::endl;
    mainWindow.show();

    std::cout << "Debug: przed app.exec()" << std::endl;
    int ret = app.exec();
    std::cout << "Debug: po app.exec()" << std::endl;

    return ret;
}
