#include "StatisticController.h"

int StatisticController::patientCount(const LinkList<Patient> *items) {
    return items ? items->count : 0;
}

int StatisticController::doctorCount(const LinkList<Doctor> *items) {
    return items ? items->count : 0;
}

int StatisticController::appointmentCount(const LinkList<Appointment> *items) {
    return items ? items->count : 0;
}
