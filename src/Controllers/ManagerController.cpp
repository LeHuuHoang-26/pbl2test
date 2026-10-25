#include "ManagerController.h"
#include "libPBL2.h"
#include <fstream>
#include <sstream>
#include <cstdio>

Manager *ManagerController::findById(LinkList<Manager> *managers, const std::string &id) {
    if (!managers || !managers->getHeader()) return nullptr;
    auto *node = managers->getHeader()->next;
    while (node && node->next) {
        if (node->info.getID() == id) return &node->info;
        node = node->next;
    }
    return nullptr;
}

int ManagerController::count(const LinkList<Manager> *managers) {
    return managers ? managers->count : 0;
}

bool ManagerController::setLocked(LinkList<Manager> *managers, const std::string &managerId, bool locked, const std::string &actorId) {
    if (!managers || !managers->getHeader() || managerId.empty() || managerId == "MA_001" || managerId == actorId) return false;
    auto *node = managers->getHeader()->next;
    while (node && node->next) {
        if (node->info.getID() == managerId) {
            node->info.setState(locked ? 0 : 1);
            return true;
        }
        node = node->next;
    }
    return false;
}

QString ManagerController::roleName(ManagerRole role) {
    switch (role) {
        case ManagerRole::Administrator: return QStringLiteral("Quản trị viên");
        case ManagerRole::Receptionist: return QStringLiteral("Lễ tân");
        case ManagerRole::SpecialReceptionist: return QStringLiteral("Lễ tân chuyên trách");
        default: return QStringLiteral("Chưa phân vai trò");
    }
}

bool ManagerController::isRootAdmin(const Manager *m) {
    if (!m) return false;
    std::string u = m->getUsername();
    libPBL2::trim(u);
    std::string id = m->getID();
    libPBL2::trim(id);
    return u == "HoangMA001" || id == "MA_001" || m->getRole() == ManagerRole::Administrator;
}

QDateTime ManagerController::permissionExpiry(const std::string &managerId, const std::string &permissionId, const std::string &filePath) {
    std::ifstream input(filePath);
    if (!input.is_open()) return QDateTime();
    std::string line;
    while (std::getline(input, line)) {
        std::stringstream row(line);
        std::string id, permission, expiry;
        if (!std::getline(row, id, '|') || !std::getline(row, permission, '|')) continue;
        std::getline(row, expiry);
        libPBL2::trim(id);
        libPBL2::trim(permission);
        libPBL2::trim(expiry);
        if (id == managerId && permission == permissionId && !expiry.empty()) {
            QDateTime parsed = QDateTime::fromString(QString::fromUtf8(expiry.c_str()), Qt::ISODate);
            if (!parsed.isValid()) parsed = QDateTime::fromString(QString::fromUtf8(expiry.c_str()), "yyyy-MM-dd HH:mm:ss");
            return parsed;
        }
    }
    return QDateTime();
}

bool ManagerController::isTemporaryAdmin(const Manager *m, const std::string &filePath) {
    if (!m || isRootAdmin(m)) return false;
    QDateTime expiry = permissionExpiry(m->getID(), "Per_1", filePath);
    return expiry.isValid() && expiry > QDateTime::currentDateTime();
}

bool ManagerController::isAdministrator(const Manager *m, const std::string &filePath) {
    return isRootAdmin(m) || isTemporaryAdmin(m, filePath);
}

bool ManagerController::hasPermission(const Manager *m, const std::string &permissionId, const std::string &filePath) {
    if (!m) return false;
    if (isRootAdmin(m) || isTemporaryAdmin(m, filePath)) return true;
    if (permissionId == "Per_1") return false;

    std::ifstream input(filePath);
    if (!input.is_open()) return false;
    std::string line;
    while (std::getline(input, line)) {
        std::stringstream row(line);
        std::string id, permission;
        if (!std::getline(row, id, '|') || !std::getline(row, permission, '|')) continue;
        libPBL2::trim(id);
        libPBL2::trim(permission);
        if (id == m->getID() && permission == permissionId) return true;
    }
    return false;
}

std::string ManagerController::generateNextId(const LinkList<Manager> *managers) {
    int maximum = 0;
    if (managers && managers->getHeader()) {
        auto *n = managers->getHeader()->next;
        while (n && n->next) {
            std::string id = n->info.getID();
            if (id.rfind("MA_", 0) == 0) {
                try {
                    int val = std::stoi(id.substr(3));
                    if (val > maximum) maximum = val;
                } catch (...) {}
            }
            n = n->next;
        }
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "MA_%03d", maximum + 1);
    return std::string(buf);
}

QStringList ManagerController::getGrantedPermissions(const std::string &managerId, const std::string &filePath) {
    QStringList granted;
    std::ifstream input(filePath);
    if (!input.is_open()) return granted;
    std::string line;
    while (std::getline(input, line)) {
        std::stringstream row(line);
        std::string id, permission;
        if (!std::getline(row, id, '|') || !std::getline(row, permission, '|')) continue;
        libPBL2::trim(id);
        libPBL2::trim(permission);
        if (id == managerId) {
            granted.append(QString::fromUtf8(permission.c_str()));
        }
    }
    return granted;
}

bool ManagerController::savePermissions(const std::string &managerId, const QList<PermissionRecord> &grantsForManager, const std::string &filePath) {
    QList<PermissionRecord> all;
    std::ifstream existing(filePath);
    std::string line;
    while (std::getline(existing, line)) {
        std::stringstream row(line);
        std::string id, permission, expiry;
        if (!std::getline(row, id, '|') || !std::getline(row, permission, '|')) continue;
        std::getline(row, expiry);
        libPBL2::trim(id);
        libPBL2::trim(permission);
        libPBL2::trim(expiry);
        if (id.empty() || id == "MA_ID" || id.find('-') != std::string::npos || id == managerId) continue;
        all.append({id, permission, expiry});
    }

    for (const auto &item : grantsForManager) {
        all.append(item);
    }

    std::ofstream output(filePath);
    if (!output.is_open()) return false;
    output << "MA_ID | Per_ID | Expires_At\n-------+--------+--------------------\n";
    for (const auto &entry : all) {
        output << entry.id << " | " << entry.permission << " | " << entry.expiry << "\n";
    }
    return true;
}
