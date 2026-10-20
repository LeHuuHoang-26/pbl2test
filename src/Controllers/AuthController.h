#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H
#include "../Models/Manager.h"
#include "../DataStructures/LinkList.h"
#include <string>
class AuthController {
public:
    static Manager *authenticate(LinkList<Manager> *managers, const std::string &username, const std::string &password);
};
#endif
