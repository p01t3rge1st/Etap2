/**
 * @file main.cpp
 * @brief Program monitorujący dane z czujników środowiskowych.
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
#include <QTranslator>
#include <QLocale>
#include "MainWindow.h"

// Definicje kolorów
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define CLEAR   "\033[2J\033[1;1H"

/**
 * @brief Wyświetla status inicjalizacji systemu.
 * 
 * @param portOpen Status portu szeregowego.
 * @param co2Ready Status czujnika CO2.
 * @param pmReady Status czujnika pyłu zawieszonego.
 * @param radReady Status czujnika promieniowania.
 */
void printInitStatus(bool portOpen, bool co2Ready, bool pmReady, bool radReady) {
    std::cout << CLEAR;
    std::cout << "[Port szeregowy]: " << (portOpen ? GREEN "OK" : RED "ERROR") << RESET << std::endl;
    std::cout << "[SCD30 CO2]: " << (co2Ready ? GREEN "OK" : RED "WAITING") << RESET << std::endl;
    std::cout << "[PMS7003]: " << (pmReady ? GREEN "OK" : RED "WAITING") << RESET << std::endl;
    std::cout << "[RAD]: " << (radReady ? GREEN "OK" : RED "WAITING") << RESET << std::endl;
}

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
    
    std::cout << "Start..." << std::endl;
    bool portOpen = false;
    bool co2Ready = false;
    bool pmReady = false;
    bool radReady = false;
    
    // Pierwszy status - wszystko czerwone
    printInitStatus(portOpen, co2Ready, pmReady, radReady);
    
    // Próba otwarcia portu
    if(reader.openPort()) {
        portOpen = true;
        printInitStatus(portOpen, co2Ready, pmReady, radReady);
    } else {
        std::cerr << RED "\nBłąd: Nie można otworzyć portu szeregowego!" RESET << std::endl;
        return 1;
    }
    
    // Czekaj na inicjalizację czujników
    int timeout = 45;
    bool initialized = false;
    
    for (int i = 0; i < timeout; ++i) {
        if (reader.readData()) {
            SensorData data = reader.getData();
            
            if (data.co2 != -1 && !co2Ready) {
                co2Ready = true;
                printInitStatus(portOpen, co2Ready, pmReady, radReady);
            }
            if (data.pm1 != -1 && !pmReady) {
                pmReady = true;
                printInitStatus(portOpen, co2Ready, pmReady, radReady);
            }
            if (data.radiation != -1 && !radReady) {
                radReady = true;
                printInitStatus(portOpen, co2Ready, pmReady, radReady);
            }
            
            if (co2Ready && pmReady && radReady) {
                initialized = true;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (!initialized) {
        std::cout << RED "\nBłąd: Nie udało się zainicjalizować wszystkich czujników!" RESET << std::endl;
        return 2;
    }

    std::cout << GREEN "\nAll ok...Qt start" RESET << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    QApplication app(argc, argv);

    // Dodaj obsługę tłumaczeń
    QTranslator translator;
    // Możesz ustawić język ręcznie:
    // translator.load(":/translations/wds_en.qm");
    // lub użyć języka systemowego:
    const QString locale = QLocale::system().name();
    if (translator.load(":/translations/" + locale))
        app.installTranslator(&translator);

    MainWindow mainWindow(&reader);
    mainWindow.show();

    return app.exec();
}
