QT += core gui widgets charts
CONFIG += c++23

TARGET = AnTamMng
TEMPLATE = app

# --- Cấu hình thư mục build ---
DESTDIR     = $$PWD/bin        # Exe xuất ra bin/
OBJECTS_DIR = $$PWD/release    # Object files vào release/
MOC_DIR     = $$PWD/release
RCC_DIR     = $$PWD/release
UI_DIR      = $$PWD/release

# --- Include paths ---
INCLUDEPATH += \
    $$PWD/src/Models \
    $$PWD/src/Controllers \
    $$PWD/src/Views \
    $$PWD/src/Views/Login \
    $$PWD/src/Views/Doctor \
    $$PWD/src/Views/Patient \
    $$PWD/src/Views/Appointment \
    $$PWD/src/Views/Account \
    $$PWD/src/Views/Statistic \
    $$PWD/src/DataStructures \
    $$PWD/src/Utils

# --- Source files ---
SOURCES += \
    src/main.cpp \
    src/Models/Appointment.cpp \
    src/Models/Doctor.cpp \
    src/Models/Human.cpp \
    src/Models/Manager.cpp \
    src/Models/Patient.cpp \
    src/Models/Permission.cpp \
    src/Models/Symptom.cpp \
    src/Models/SymptomPatient.cpp \
    src/Utils/Date.cpp \
    src/Utils/libPBL2.cpp \
    src/Controllers/AuthController.cpp \
    src/Controllers/ManagerController.cpp \
    src/Controllers/DoctorController.cpp \
    src/Controllers/PatientController.cpp \
    src/Controllers/AppointmentController.cpp \
    src/Controllers/StatisticController.cpp \
    src/Views/MainWindow.cpp \
    src/Views/UIHelper.cpp \
    src/Views/Login/Login.cpp \
    src/Views/Doctor/DoctorView.cpp \
    src/Views/Patient/PatientView.cpp \
    src/Views/Appointment/AppointmentView.cpp \
    src/Views/Account/AccountView.cpp \
    src/Views/Statistic/StatisticView.cpp \

# --- Header files ---
HEADERS += \
    src/Models/Appointment.h \
    src/Models/Doctor.h \
    src/Models/Human.h \
    src/Models/Manager.h \
    src/Models/Patient.h \
    src/Models/Permission.h \
    src/Models/Symptom.h \
    src/Models/SymptomPatient.h \
    src/DataStructures/LinkList.h \
    src/DataStructures/Queue.h \
    src/Utils/Date.h \
    src/Utils/libPBL2.h \
    src/Controllers/AuthController.h \
    src/Controllers/ManagerController.h \
    src/Controllers/DoctorController.h \
    src/Controllers/PatientController.h \
    src/Controllers/AppointmentController.h \
    src/Controllers/StatisticController.h \
    src/Views/MainWindow.h \
    src/Views/UIHelper.h \
    src/Views/Login/Login.h \
    src/Views/Doctor/DoctorView.h \
    src/Views/Patient/PatientView.h \
    src/Views/Appointment/AppointmentView.h \
    src/Views/Account/AccountView.h \
    src/Views/Statistic/StatisticView.h \

RESOURCES += resource.qrc
