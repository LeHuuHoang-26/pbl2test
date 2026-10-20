#include "AuthController.h"
Manager *AuthController::authenticate(LinkList<Manager> *managers, const std::string &username, const std::string &password) {
    if (!managers || !managers->getHeader()) return nullptr;
    auto *node = managers->getHeader()->next;
    while (node && node->next) {
        if (node->info.isActive() && node->info.isValid(username, password)) return &node->info;
        node = node->next;
    }
    return nullptr;
}
