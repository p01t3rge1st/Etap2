# Minimalna wersja CMake
cmake_minimum_required(VERSION 3.10)

# Nazwa projektu
project(SensorReaderApp)

# Znajdź Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# Ustawienia kompilatora
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodaj katalogi z plikami nagłówkowymi
include_directories(${CMAKE_SOURCE_DIR})

# Dodaj źródła
set(SOURCES
    main.cpp
    SensorReader.cpp
    MainWindow.cpp   # Dodaj nową klasę okna
)

set(HEADERS
    SensorReader.h
    MainWindow.h   # Dodaj nową klasę okna
)

# Tworzymy aplikację
add_executable(SensorReaderApp ${SOURCES} ${HEADERS})

# Podłącz Qt5 (Core, Gui, Widgets)
target_link_libraries(SensorReaderApp Qt5::Core Qt5::Gui Qt5::Widgets)

# Inne flagi kompilacji (jeśli są potrzebne)
target_compile_options(SensorReaderApp PRIVATE -Wall -Wextra -lftxui-screen -lftxui-dom -lftxui-component)

