#ifndef LIBPBL2_H
#define LIBPBL2_H

#include <string>
#include "../DataStructures/LinkList.h"
#include "../Models/Permission.h"

class Manager;

namespace libPBL2 {

    using Permission = ::Permission;
    using PermissionType = ::PermissionType;

    void trim(std::string &str);
    void filterNewline(std::string &str);
    size_t utf8Length(const std::string &str);
    std::string padRightUtf8(const std::string &str, size_t targetWidth);

    bool isAllowed(const Manager &M, PermissionType Per, const LinkList<Permission> *PermissionList = nullptr);

}

#endif
