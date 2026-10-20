#include "AppointmentController.h"
int AppointmentController::count(const LinkList<Appointment> *appointments) {
    if (!appointments || !appointments->getHeader()) return 0;
    int total = 0; auto *node = appointments->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault()) ++total; node = node->next; }
    return total;
}
