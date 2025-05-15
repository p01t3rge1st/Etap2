QT += core gui widgets charts
TARGET = SensorReaderApp
TEMPLATES = app

SOURCES += main.cpp \
           MainWindow.cpp \
           SensorReader.cpp \
           SensorDataLogger.cpp

HEADERS += MainWindow.h \
           SensorReader.h \
           SensorDataLogger.h

INCLUDEPATH += /usr/include/qt5
LIBS += -L/usr/lib/x86_64-linux-gnu -lQt5Widgets -lQt5Core -lQt5Gui

