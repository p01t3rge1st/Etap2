#include "SensorReader.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <regex>
#include "MainWindow.h"

/**
 * @brief Konstruktor klasy SensorReader.
 * @param portname Nazwa portu szeregowego.
 * @param baudrate Prędkość transmisji (domyślnie B9600).
 */
SensorReader::SensorReader(const std::string& portname, int baudrate)
    : portname(portname), baudrate(baudrate), serial_port(-1) {}

/**
 * @brief Destruktor klasy SensorReader.
 */
SensorReader::~SensorReader() {
    if (serial_port >= 0) {
        closePort();
    }
}

/**
 * @brief Otwiera port szeregowy.
 * @return true, jeśli port został otwarty pomyślnie, false w przeciwnym razie.
 */
bool SensorReader::openPort() {
    serial_port = open(portname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_port < 0) {
        std::cerr << "Failed to open port: " << portname << std::endl;
        return false;
    }

    termios options;
    tcgetattr(serial_port, &options);
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 1;

    tcsetattr(serial_port, TCSANOW, &options);
    return true;
}

/**
 * @brief Zamyka port szeregowy.
 */
void SensorReader::closePort() {
    close(serial_port);
    serial_port = -1;
}

/**
 * @brief Odczytuje dane z portu szeregowego.
 * @return true, jeśli dane zostały odczytane pomyślnie, false w przeciwnym razie.
 */
bool SensorReader::readData() {
    char temp_buffer[256];
    int bytes_read = read(serial_port, temp_buffer, sizeof(temp_buffer) - 1);
    if (bytes_read > 0) {
        temp_buffer[bytes_read] = '\0';
        serial_data += temp_buffer;
        if (serial_data.find("\n") != std::string::npos) {
            SensorData newData = parseSensorData(serial_data);
            serial_data.clear();
            if (newData.co2 == 0) {
                return false;
            }
            data = newData;
            return true;
        }
    }
    return false;
}

/**
 * @brief Zwraca ostatnio odczytane dane z czujników.
 * @return Struktura SensorData zawierająca dane z czujników.
 */
SensorData SensorReader::getData() const {
    return data;
}

/**
 * @brief Parsuje surowe dane z czujników.
 * @param raw Surowe dane w formie ciągu znaków.
 * @return Struktura SensorData zawierająca sparsowane dane.
 */
SensorData SensorReader::parseSensorData(const std::string& raw) {
    SensorData data;
    std::regex co2_re(R"(CO2 Level:\s*(\d+))");
    std::regex co2_temp_re(R"(Temperature:\s*(\d+))");
    std::regex co2_hum_re(R"(Humidity:\s*(\d+))");
    std::regex pm1_re(R"(PM 1\.0 \(ug/m3\):\s*(\d+))");
    std::regex pm25_re(R"(PM 2\.5 \(ug/m3\):\s*(\d+))");
    std::regex pm10_re(R"(PM 10\.0 \(ug/m3\):\s*(\d+))");
    std::regex radiation_re(R"(Radiation:\s*(\d+)\s*CPM)");
    std::regex radiation_dose_re(R"(Radiation dose per h:\s*([\d.]+)\s*uSv)");

    std::smatch match;
    if (std::regex_search(raw, match, co2_re)) {
        data.co2 = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, co2_temp_re)) {
        data.co2_temp = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, co2_hum_re)) {
        data.co2_hum = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, pm1_re)) {
        data.pm1 = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, pm25_re)) {
        data.pm25 = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, pm10_re)) {
        data.pm10 = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, radiation_re)) {
        data.radiation = std::stoi(match[1]);
    }
    if (std::regex_search(raw, match, radiation_dose_re)) {
        data.radiation_dose_per_hour = std::stof(match[1]);
    }

    return data;
}
