#include "Permission.h"
#include "libPBL2.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

Permission::Permission(string maId, PermissionType perId)
    : MA_id(maId), Per_id(perId) {}

Permission::Permission(const Permission &other)
    : MA_id(other.MA_id), Per_id(other.Per_id) {}

Permission::~Permission() {}

string Permission::getMA_id() const {
    return MA_id;
}

PermissionType Permission::getPer_id() const {
    return Per_id;
}

void Permission::setMA_id(const string &maId) {
    MA_id = maId;
}

void Permission::setPer_id(PermissionType perId) {
    Per_id = perId;
}

bool Permission::isDefault() const {
    return MA_id.empty() && Per_id == PermissionType::Empty;
}

void Permission::setDefault() {
    MA_id = "";
    Per_id = PermissionType::Empty;
}

string Permission::permissionTypeToString(PermissionType per) {
    switch (per) {
        case PermissionType::Per_00: return "00";
        case PermissionType::Per_01: return "01";
        case PermissionType::Per_02: return "02";
        case PermissionType::Per_03: return "03";
        case PermissionType::Per_04: return "04";
        case PermissionType::Per_05: return "05";
        case PermissionType::Per_06: return "06";
        case PermissionType::Per_07: return "07";
        case PermissionType::Per_08: return "08";
        default: return "Empty";
    }
}

PermissionType Permission::permissionTypeFromString(const string &str) {
    if (str == "Per_00" || str == "00") return PermissionType::Per_00;
    if (str == "Per_01" || str == "01") return PermissionType::Per_01;
    if (str == "Per_02" || str == "02") return PermissionType::Per_02;
    if (str == "Per_03" || str == "03") return PermissionType::Per_03;
    if (str == "Per_04" || str == "04") return PermissionType::Per_04;
    if (str == "Per_05" || str == "05") return PermissionType::Per_05;
    if (str == "Per_06" || str == "06") return PermissionType::Per_06;
    if (str == "Per_07" || str == "07") return PermissionType::Per_07;
    if (str == "Per_08" || str == "08") return PermissionType::Per_08;
    return PermissionType::Empty;
}

string Permission::toString() const {
    return libPBL2::padRightUtf8(MA_id, 6) + " | " + permissionTypeToString(Per_id);
}

Permission Permission::loadFromString(const string &str) {
    Permission p;
    p.Per_id = PermissionType::Empty;
    if (str.empty()) return p;

    stringstream ss(str);
    string maId, perStr;
    if (!getline(ss, maId, '|') || !getline(ss, perStr)) {
        return p;
    }

    libPBL2::trim(maId);
    libPBL2::trim(perStr);

    p.MA_id = maId;
    p.Per_id = permissionTypeFromString(perStr);
    return p;
}

LinkList<Permission>* Permission::loadFromFile(const string &path) {
    LinkList<Permission> *PerList = new LinkList<Permission>;
    if (!PerList) return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete PerList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp); // Bỏ qua dòng tiêu đề
    getline(f, tmp); // Bỏ qua dòng phân cách

    string line;
    while (getline(f, line)) {
        libPBL2::trim(line);
        if (line.empty()) continue;
        Permission p = Permission::loadFromString(line);
        if (!p.MA_id.empty() && p.Per_id != PermissionType::Empty) {
            PerList->insert(p);
        }
    }

    f.close();
    return PerList;
}

bool Permission::saveToFile(const string &path, const LinkList<Permission> *list) {
    if (!list || list->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    f << "MA_ID  | Per_ID\n";
    f << "-------+---------\n";

    LinkList<Permission>::Node *tmp = list->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (!tmp->info.MA_id.empty()) {
            f << tmp->info.toString() << "\n";
        }
        tmp = tmp->next;
    }

    f.close();
    return true;
}

