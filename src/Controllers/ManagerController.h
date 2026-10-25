#ifndef MANAGERCONTROLLER_H
#define MANAGERCONTROLLER_H

#include "../Models/Manager.h"
#include "../DataStructures/LinkList.h"
#include <string>
#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>

struct PermissionRecord {
    std::string id;
    std::string permission;
    std::string expiry;
};

class ManagerController {
public:
    static Manager *findById(LinkList<Manager> *managers, const std::string &id);
    static int count(const LinkList<Manager> *managers);
    static bool setLocked(LinkList<Manager> *managers, const std::string &managerId, bool locked, const std::string &actorId);
    static QString roleName(ManagerRole role);
    static bool isRootAdmin(const Manager *m);
    static QDateTime permissionExpiry(const std::string &managerId, const std::string &permissionId, const std::string &filePath);
    static bool isTemporaryAdmin(const Manager *m, const std::string &filePath);
    static bool isAdministrator(const Manager *m, const std::string &filePath);
    static bool hasPermission(const Manager *m, const std::string &permissionId, const std::string &filePath);
    static std::string generateNextId(const LinkList<Manager> *managers);
    static QStringList getGrantedPermissions(const std::string &managerId, const std::string &filePath);
    static bool savePermissions(const std::string &managerId, const QList<PermissionRecord> &grantsForManager, const std::string &filePath);
};

#endif // MANAGERCONTROLLER_H
