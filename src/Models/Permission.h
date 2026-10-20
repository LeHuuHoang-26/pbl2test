#ifndef PERMISSION_H
#define PERMISSION_H

#include <string>
#include "LinkList.h"

enum class PermissionType {
    Empty = 0,
    Per_01,
    Per_02,
    Per_03,
    Per_04,
    Per_05,
    Per_06,
    Per_07,
    Per_08,
    Per_00
};

class Permission {
public:
    std::string MA_id;
    PermissionType Per_id;

    Permission(std::string maId = "", PermissionType perId = PermissionType::Empty);
    Permission(const Permission &other);
    Permission &operator=(const Permission &other) = default;
    ~Permission();

    std::string getMA_id() const;
    PermissionType getPer_id() const;

    void setMA_id(const std::string &maId);
    void setPer_id(PermissionType perId);

    bool isDefault() const;
    void setDefault();

    std::string toString() const;
    static Permission loadFromString(const std::string &str);

    static std::string permissionTypeToString(PermissionType per);
    static PermissionType permissionTypeFromString(const std::string &str);

    static LinkList<Permission>* loadFromFile(const std::string &path);
    static bool saveToFile(const std::string &path, const LinkList<Permission> *list);
};

#endif

