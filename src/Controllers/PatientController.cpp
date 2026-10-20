#include "PatientController.h"
Patient *PatientController::findById(LinkList<Patient> *patients, const std::string &id) {
    if (!patients || !patients->getHeader()) return nullptr;
    auto *node = patients->getHeader()->next;
    while (node && node->next) { if (node->info.getID() == id) return &node->info; node = node->next; }
    return nullptr;
}
