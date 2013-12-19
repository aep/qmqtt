TEMPLATE = app
TARGET = mqtt
INCLUDEPATH += .

QT = core network


MOC_DIR = .tmp
OBJECTS_DIR = .tmp

HEADERS += qmqtt.hpp qmqtt_p.hpp
SOURCES += main.cpp qmqtt.cpp frame.cpp
