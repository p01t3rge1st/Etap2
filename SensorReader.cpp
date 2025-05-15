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
 * @brief Parsuje surowe dane z czujników i weryfikuje CRC.
 * @param raw Surowe dane w formie ciągu znaków.
 * @return Struktura SensorData zawierająca sparsowane dane.
 */
SensorData SensorReader::parseSensorData(const std::string& raw) {
    SensorData result;
    uint16_t calculatedCRC = 0;
    
    std::istringstream iss(raw);
    std::string line;
    bool startFound = false;
    
    while (std::getline(iss, line)) {
        if (line == "--[new_line]--") {
            startFound = true;
            continue;
        }
        if (line == "--[end_line]--") {
            break;
        }
        if (!startFound) continue;
        
        size_t pos = line.find(": ");
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 2);
            
            try {
                if (line.find("CRC") != std::string::npos) {
                    result.crc = std::stoul(value, nullptr, 16);
                    continue;
                }
                
                if (line.find("PM 1.0") != std::string::npos) {
                    result.pm1 = std::stoi(value);
                    calculatedCRC += result.pm1;
                } else if (line.find("PM 2.5") != std::string::npos) {
                    result.pm25 = std::stoi(value);
                    calculatedCRC += result.pm25;
                } else if (line.find("PM 10.0") != std::string::npos) {
                    result.pm10 = std::stoi(value);
                    calculatedCRC += result.pm10;
                } else if (line.find("CO2 Level") != std::string::npos) {
                    result.co2 = std::stoi(value);
                    calculatedCRC += result.co2;
                } else if (line.find("Temperature") != std::string::npos) {
                    result.co2_temp = std::stoi(value);
                    calculatedCRC += result.co2_temp;
                } else if (line.find("Humidity") != std::string::npos) {
                    result.co2_hum = std::stoi(value);
                    calculatedCRC += result.co2_hum;
                } else if (line.find("Radiation: ") != std::string::npos) {
                    result.radiation = std::stoi(value);
                    calculatedCRC += result.radiation;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing value: " << e.what() << std::endl;
            }
        }
    }
    
    result.crcValid = (calculatedCRC == result.crc);
    // std::cout << "Calculated CRC: " << std::hex << calculatedCRC 
    //           << " Received CRC: " << std::hex << result.crc << std::endl;
    return result;
}
