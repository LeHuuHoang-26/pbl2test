#include "StatisticController.h"
template<class T> static int countItems(const LinkList<T> *items) { if (!items || !items->getHeader()) return 0; int n = 0; auto *node = items->getHeader()->next; while (node && node->next) { if (!node->info.isDefault()) ++n; node = node->next; } return n; }
int StatisticController::patientCount(const LinkList<Patient> *items) { return countItems(items); }
int StatisticController::doctorCount(const LinkList<Doctor> *items) { return countItems(items); }
int StatisticController::appointmentCount(const LinkList<Appointment> *items) { return countItems(items); }
