#ifndef APPOINTMENTCONTROLLER_H
#define APPOINTMENTCONTROLLER_H
#include "../Models/Appointment.h"
#include "../DataStructures/LinkList.h"
class AppointmentController {
public:
    static int count(const LinkList<Appointment> *appointments);
};
#endif
