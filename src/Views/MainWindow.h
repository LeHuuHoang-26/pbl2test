#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QStringList>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <string>

#include "LinkList.h"
#include "Manager.h"
#include "Patient.h"
#include "Doctor.h"
#include "Appointment.h"
#include "Symptom.h"
#include "SymptomPatient.h"

class QStackedWidget;
class QPushButton;
class QTableWidget;
class QLabel;
class QCalendarWidget;
class QFrame;

// Khai báo trước các lớp Views mô-đun
class AppointmentView;
class PatientView;
class DoctorView;
class AccountView;
class StatisticView;
class Login;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool initApp();
    void showLogin();
    void handleLogin();
    void handleLogout();
    void setPage(int index);
    void refreshPage(int index);
    void saveAll();

    // Truy cập nguồn dữ liệu (Data Getters)
    LinkList<Manager> *getManagers() const { return managers; }
    LinkList<Patient> *getPatients() const { return patients; }
    LinkList<Doctor> *getDoctors() const { return doctors; }
    LinkList<Symptom> *getSymptoms() const { return symptoms; }
    LinkList<SymptomPatient> *getSymptomPatients() const { return symptomPatients; }
    LinkList<Appointment> *getAppointments() const { return appointments; }
    Manager *getCurrentManager() const { return currentManager; }

    // Kiểm tra phân quyền (ủy thác ManagerController)
    bool isRootAdmin() const;
    bool isAdministrator() const;
    bool hasPermission(const std::string &permissionId) const;

    // Tiện ích dữ liệu (ủy thác Controllers)
    QString specialty(const std::string &id) const;
    Patient *findPatient(const std::string &id) const;
    Doctor *findDoctor(const std::string &id) const;
    SymptomPatient *findSymptomPatient(const std::string &id) const;
    Patient *findPatientForAppointment(const Appointment &a) const;
    std::string getPatientIdForAppointment(const Appointment &a) const;
    QString getSymptomNamesForAppointment(const Appointment &a) const;
    bool isHoliday(const QDate &date) const;
    QString holidayNote(const QDate &date) const;
    QString patientSymptoms(const std::string &patientId) const;
    QString unresolvedPatientSymptoms(const std::string &patientId) const;
    void updatePatientSymptomStatus(const std::string &patientId);
    std::string nextAppointmentId() const;

    // View pointers và điều phối hộp thoại
    AppointmentView *getAppointmentView() const { return appointmentView; }
    PatientView *getPatientView() const { return patientView; }
    DoctorView *getDoctorView() const { return doctorView; }
    AccountView *getAccountView() const { return accountView; }
    StatisticView *getStatisticView() const { return statisticView; }

    bool showBookingDialog(const QDate &defaultDate = QDate::currentDate());
    void editAppointmentDialog(const std::string &id);

    // Giao diện và Phong cách
    QString style() const;
    QPushButton *button(const QString &text, const QString &kind = "primary");

private:
    LinkList<Manager> *managers = nullptr;
    LinkList<Patient> *patients = nullptr;
    LinkList<Doctor> *doctors = nullptr;
    LinkList<Symptom> *symptoms = nullptr;
    LinkList<SymptomPatient> *symptomPatients = nullptr;
    LinkList<Appointment> *appointments = nullptr;

    Manager *currentManager = nullptr;
    QWidget *root = nullptr;
    QStackedWidget *pages = nullptr;
    QWidget *sidebar = nullptr;
    QList<QPushButton *> navButtons;
    int activeNav = 0;

    // Con trỏ tới các Views mô-đun
    QStackedWidget *windowStack = nullptr;
    Login *loginView = nullptr;
    AppointmentView *appointmentView = nullptr;
    PatientView *patientView = nullptr;
    DoctorView *doctorView = nullptr;
    AccountView *accountView = nullptr;
    StatisticView *statisticView = nullptr;

    void updateNavigation();
    void setupShell();
};

#endif // MAINWINDOW_H
