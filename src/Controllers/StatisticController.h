#ifndef STATISTICCONTROLLER_H
#define STATISTICCONTROLLER_H
#include "../Models/Patient.h"
#include "../Models/Doctor.h"
#include "../Models/Appointment.h"
#include "../DataStructures/LinkList.h"
class StatisticController {
public:
    static int patientCount(const LinkList<Patient> *items);
    static int doctorCount(const LinkList<Doctor> *items);
    static int appointmentCount(const LinkList<Appointment> *items);
};
#endif
