#include "DoctorController.h"
Doctor *DoctorController::findById(LinkList<Doctor> *doctors, const std::string &id) {
    if (!doctors || !doctors->getHeader()) return nullptr;
    auto *node = doctors->getHeader()->next;
    while (node && node->next) { if (node->info.getID() == id) return &node->info; node = node->next; }
    return nullptr;
}
