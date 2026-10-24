#include "AppointmentController.h"

int AppointmentController::count(const LinkList<Appointment> *appointments) {
    return appointments ? appointments->count : 0;
}
