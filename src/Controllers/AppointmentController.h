#ifndef APPOINTMENTCONTROLLER_H
#define APPOINTMENTCONTROLLER_H

#include "../Models/Appointment.h"
#include "../DataStructures/LinkList.h"
#include <QList>
#include <QDate>
#include <QString>
#include <string>

struct HolidayEntry {
    QString date;
    QString note;
};

class AppointmentController {
public:
    static int count(const LinkList<Appointment> *appointments);
    static int pendingCount(const LinkList<Appointment> *appointments);
    static QList<Appointment*> getAppointmentsForDate(const LinkList<Appointment> *appointments, const QDate &targetDate);
    static QList<Appointment*> getUpcomingAppointments(const LinkList<Appointment> *appointments, const QDate &today);
    static void sortAppointmentsByDate(QList<Appointment*> &list);
    static void getMonthDayCounts(const LinkList<Appointment> *appointments, int year, int month, int dayCounts[32]);
    static std::string generateNextId(const LinkList<Appointment> *appointments);
    static QDate getLunarNewYear(int year);
    static bool isTetHoliday(const QDate &date, int holidayDays = 5);
    static bool isHoliday(const QDate &date, const std::string &configPath);
    static QString getHolidayNote(const QDate &date, const std::string &configPath);
    static QList<HolidayEntry> loadHolidays(const std::string &configPath);
    static bool saveHolidays(const std::string &configPath, const QList<HolidayEntry> &list);
};

#endif // APPOINTMENTCONTROLLER_H
