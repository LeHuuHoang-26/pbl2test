#include "ManagerController.h"
bool ManagerController::setLocked(LinkList<Manager> *managers, const std::string &managerId, bool locked, const std::string &actorId) {
    if (!managers || !managers->getHeader() || managerId.empty() || managerId == "MA_001" || managerId == actorId) return false;
    auto *node = managers->getHeader()->next;
    while (node && node->next) {
        if (node->info.getID() == managerId) { node->info.setState(locked ? 0 : 1); return true; }
        node = node->next;
    }
    return false;
}
