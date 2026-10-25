#ifndef STATISTICCONTROLLER_H
#define STATISTICCONTROLLER_H

#include "../Models/Patient.h"
#include "../Models/Doctor.h"
#include "../Models/Appointment.h"
#include "../DataStructures/LinkList.h"
#include <string>

struct DoctorLeaderboardItem {
    std::string doctorId;
    std::string doctorName;
    std::string specialtyId;
    std::string roomId;
    double rating = 0.0;
    int appointmentCount = 0;
};

struct SpecialtyDemandItem {
    std::string spId;
    int count = 0;
};

class StatisticController {
public:
    static int patientCount(const LinkList<Patient> *items);
    static int doctorCount(const LinkList<Doctor> *items);
    static int appointmentCount(const LinkList<Appointment> *items);
    static int pendingAppointmentCount(const LinkList<Appointment> *items);

    static double totalRevenue(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs);
    static double todayRevenue(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs, int day, int month, int year);
    static double completionRate(const LinkList<Appointment> *appts);
    static double completionRate(const LinkList<Appointment> *appts, int currentDay, int currentMonth, int currentYear, int currentHour, int currentMinute, int &completedCount, int &totalUntilNow);

    static void appointmentStatusCounts(const LinkList<Appointment> *appts, int &completed, int &pending, int &delayed);
    static void appointmentStatusCounts(const LinkList<Appointment> *appts, int &completed, int &pending, int &delayed, int currentDay, int currentMonth, int currentYear, int currentHour, int currentMinute);
    static void todayAppointmentCounts(const LinkList<Appointment> *appts, int day, int month, int year, int &totalToday, int &completedToday, int &pendingToday);

    static void patientGenderCounts(const LinkList<Patient> *patients, int &male, int &female);
    static void patientAgeDistribution(const LinkList<Patient> *patients, int &under18, int &from18to45, int &from46to60, int &over60);

    static int getTopDoctors(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs, DoctorLeaderboardItem outList[], int maxCount);
    static int getSpecialtyDemand(const LinkList<Appointment> *appts, const LinkList<Doctor> *docs, SpecialtyDemandItem outList[], int maxCount);
    static void getWeeklyTrend(const LinkList<Appointment> *appts, int currentDay, int currentMonth, int currentYear, int outCounts[7]);
};

#endif // STATISTICCONTROLLER_H
