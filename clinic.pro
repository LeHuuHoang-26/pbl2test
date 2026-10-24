QT += widgets
CONFIG += c++17
CONFIG -= console
TEMPLATE = app
TARGET = AnTamClinic
RC_ICONS = assets/textures/icon_app.ico

INCLUDEPATH += src/Models src/Controllers src/Utils src/DataStructures

SOURCES += src/main.cpp \
    src/Views/MainWindow.cpp \
    src/Models/Appointment.cpp \
    src/Utils/Date.cpp \
    src/Models/Doctor.cpp \
    src/Models/Manager.cpp \
    src/Models/Patient.cpp \
    src/Models/Permission.cpp \
    src/Models/QueuePatient.cpp \
    src/Models/Symptom.cpp \
    src/Models/SymptomPatient.cpp \
    src/Models/Human.cpp \
    src/Utils/libPBL2.cpp \
    src/Controllers/AppointmentController.cpp \
    src/Controllers/AuthController.cpp \
    src/Controllers/DoctorController.cpp \
    src/Controllers/ManagerController.cpp \
    src/Controllers/PatientController.cpp \
    src/Controllers/StatisticController.cpp

HEADERS += src/Models/Appointment.h \
    src/Utils/Date.h \
    src/Models/Doctor.h \
    src/Models/Manager.h \
    src/Models/Patient.h \
    src/Models/Permission.h \
    src/Models/QueuePatient.h \
    src/Models/Symptom.h \
    src/Models/SymptomPatient.h \
    src/Models/Human.h \
    src/Utils/libPBL2.h \
    src/DataStructures/LinkList.h \
    src/DataStructures/Queue.h \
    src/Controllers/AppointmentController.h \
    src/Controllers/AuthController.h \
    src/Controllers/DoctorController.h \
    src/Controllers/ManagerController.h \
    src/Controllers/PatientController.h \
    src/Controllers/StatisticController.h \
    src/Views/MainWindow.h
