#include "libPBL2.h"
#include "../Models/Manager.h"
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

namespace libPBL2 {

void trim(string &str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    str = str.substr(start, end - start + 1);
}

void filterNewline(string &str) {
    string res = "";
    for (char c : str) {
        if (c != '\r' && c != '\n') {
            res += c;
        }
    }
    str = res;
}

size_t utf8Length(const string &str) {
    size_t len = 0;
    for (unsigned char c : str) {
        if ((c & 0xC0) != 0x80) {
            len++;
        }
    }
    return len;
}

string padRightUtf8(const string &str, size_t targetWidth) {
    size_t ulen = utf8Length(str);
    if (ulen >= targetWidth) return str;
    return str + string(targetWidth - ulen, ' ');
}

bool isAllowed(const Manager &M, PermissionType Per, const LinkList<Permission> *PermissionList) {
    if (!M.isActive())
        return false;

    if (Per == PermissionType::Empty)
        return true;

    if (M.getRole() == ManagerRole::Administrator)
        return true;

    if (PermissionList == nullptr || PermissionList->getHeader() == nullptr)
        return false;

    LinkList<Permission>::Node *tmp = PermissionList->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (M.getID() == tmp->info.MA_id && tmp->info.Per_id == Per)
            return true;
        tmp = tmp->next;
    }

    return false;
}

} // namespace libPBL2

