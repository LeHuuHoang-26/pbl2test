#ifndef MANAGERCONTROLLER_H
#define MANAGERCONTROLLER_H
#include "../Models/Manager.h"
#include "../DataStructures/LinkList.h"
#include <string>
class ManagerController {
public:
    static bool setLocked(LinkList<Manager> *managers, const std::string &managerId, bool locked, const std::string &actorId);
};
#endif
