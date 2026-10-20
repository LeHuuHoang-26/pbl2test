QT += widgets
CONFIG += c++17
CONFIG -= console
TEMPLATE = app
TARGET = AnTamClinic

INCLUDEPATH += Models Controllers Data_structures

SOURCES += qt_main.cpp \
    Controllers/Appointment.cpp \
    Controllers/Date.cpp \
    Controllers/Doctor.cpp \
    Controllers/Manager.cpp \
    Controllers/Patient.cpp \
    Controllers/Permission.cpp \
    Controllers/QueuePatient.cpp \
    Controllers/Symptom.cpp \
    Controllers/SymptomPatient.cpp \
    Controllers/human.cpp \
    Controllers/libPBL2.cpp

HEADERS += Models/Appointment.h \
    Models/Date.h \
    Models/Doctor.h \
    Models/Manager.h \
    Models/Patient.h \
    Models/Permission.h \
    Models/QueuePatient.h \
    Models/Symptom.h \
    Models/SymptomPatient.h \
    Models/human.h \
    Models/libPBL2.h \
    Data_structures/LinkList.h \
    Data_structures/Queue.h
