#include "libPBL2.h"
#include <fstream>
#include <string>
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

template <>
LinkList<Manager> *LoadFromFile<Manager, LinkList>(const string &path) {
    LinkList<Manager> *ManagerList = new LinkList<Manager>;
    if (!ManagerList)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete ManagerList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp);
    getline(f, tmp);

    while (true) {
        Manager M(f);
        if (M.isDefault())
            break;
        ManagerList->insert(M);
    }
    f.close();
    return ManagerList;
}

template <>
LinkList<Permission> *LoadFromFile<Permission, LinkList>(const string &path) {
    LinkList<Permission> *PerList = new LinkList<Permission>;
    if (!PerList)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete PerList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp);
    getline(f, tmp);

    while (true) {
        Permission tmpPer;
        if (!getline(f, tmpPer.MA_id, '|'))
            break;

        if (!getline(f, tmp, '\n'))
            break;

        trim(tmpPer.MA_id);
        trim(tmp);
        if (tmpPer.MA_id.empty())
            break;

        tmpPer.Per_id = getFromStr<PermissionType>(tmp);
        PerList->insert(tmpPer);
    }
    f.close();
    return PerList;
}

template <>
PermissionType getFromStr<PermissionType>(const string &str) {
    if (str == "Per_00") return PermissionType::Per_00;
    if (str == "Per_01") return PermissionType::Per_01;
    if (str == "Per_02") return PermissionType::Per_02;
    if (str == "Per_03") return PermissionType::Per_03;
    if (str == "Per_04") return PermissionType::Per_04;
    if (str == "Per_05") return PermissionType::Per_05;
    if (str == "Per_06") return PermissionType::Per_06;
    if (str == "Per_07") return PermissionType::Per_07;
    if (str == "Per_08") return PermissionType::Per_08;
    return PermissionType::Empty;
}

template <>
string toStr<PermissionType>(const PermissionType &PerT) {
    if (PerT == PermissionType::Per_00) return "Per_00";
    if (PerT == PermissionType::Per_01) return "Per_01";
    if (PerT == PermissionType::Per_02) return "Per_02";
    if (PerT == PermissionType::Per_03) return "Per_03";
    if (PerT == PermissionType::Per_04) return "Per_04";
    if (PerT == PermissionType::Per_05) return "Per_05";
    if (PerT == PermissionType::Per_06) return "Per_06";
    if (PerT == PermissionType::Per_07) return "Per_07";
    if (PerT == PermissionType::Per_08) return "Per_08";
    return "Empty";
}

template <>
string toStr<ManagerRole>(const ManagerRole &MA_Role) {
    if (MA_Role == ManagerRole::Administrator) return "Administrator";
    if (MA_Role == ManagerRole::Receptionist || MA_Role == ManagerRole::SpecialReceptionist) return "Receptionist";
    return "Empty";
}

template <>
LinkList<Patient> *LoadFromFile<Patient, LinkList>(const string &path) {
    LinkList<Patient> *PatientList = new LinkList<Patient>;
    if (!PatientList)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete PatientList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp);
    getline(f, tmp);

    while (true) {
        Patient P(f);
        if (P.isDefault())
            break;
        PatientList->insert(P);
    }
    f.close();
    return PatientList;
}

template <>
LinkList<Symptom> *LoadFromFile<Symptom, LinkList>(const string &path) {
    LinkList<Symptom> *SymptomList = new LinkList<Symptom>;
    if (!SymptomList)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete SymptomList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp);
    getline(f, tmp);

    while (true) {
        Symptom S(f);
        if (S.isDefault())
            break;
        SymptomList->insert(S);
    }
    f.close();
    return SymptomList;
}

template <>
LinkList<SymptomPatient> *LoadFromFile<SymptomPatient, LinkList>(const string &path) {
    LinkList<SymptomPatient> *SPList = new LinkList<SymptomPatient>;
    if (!SPList)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete SPList;
        return nullptr;
    }

    while (true) {
        SymptomPatient SP(f);
        if (SP.isDefault())
            break;
        SPList->insert(SP);
    }
    f.close();
    return SPList;
}

template <>
LinkList<Doctor> *LoadFromFile<Doctor, LinkList>(const string &path) {
    LinkList<Doctor> *DoctorList = new LinkList<Doctor>;
    if (!DoctorList)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete DoctorList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp);
    getline(f, tmp);

    while (true) {
        string id, fullName, phone, genderStr, ageStr;
        string spId, stateStr, roomId, feeStr, ratingStr;

        if (!getline(f, id, '|')) break;
        if (!getline(f, fullName, '|')) break;
        if (!getline(f, phone, '|')) break;
        if (!getline(f, genderStr, '|')) break;
        if (!getline(f, ageStr, '|')) break;
        if (!getline(f, spId, '|')) break;
        if (!getline(f, stateStr, '|')) break;
        if (!getline(f, roomId, '|')) break;
        if (!getline(f, feeStr, '|')) break;
        if (!getline(f, ratingStr, '\n')) break;

        trim(id);
        trim(fullName);
        trim(phone);
        trim(genderStr);
        trim(ageStr);
        trim(spId);
        trim(stateStr);
        trim(roomId);
        trim(feeStr);
        trim(ratingStr);

        if (id.empty()) continue;

        bool gender = false;
        int age = 0;
        bool state = false;
        double fee = 0.0;
        double rating = 0.0;

        try {
            gender = (!genderStr.empty() && stoi(genderStr) == 1);
            age = (!ageStr.empty() ? stoi(ageStr) : 0);
            state = (!stateStr.empty() && stoi(stateStr) == 1);
            fee = (!feeStr.empty() ? stod(feeStr) : 0.0);
            rating = (!ratingStr.empty() ? stod(ratingStr) : 0.0);
        } catch (...) {}

        Doctor doc(id, fullName, phone, gender, age, spId, state, roomId, fee, rating);
        DoctorList->insert(doc);
    }

    f.close();
    return DoctorList;
}

template <>
bool SaveToFile<SymptomPatient, LinkList>(const string &path, const LinkList<SymptomPatient> *list) {
    if (!list || list->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    LinkList<SymptomPatient>::Node *tmp = list->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (!tmp->info.isDefault()) {
            tmp->info.save(f);
        }
        tmp = tmp->next;
    }

    f.close();
    return true;
}

template <>
string generateNext_id<SymptomPatient>(const LinkList<SymptomPatient> *list) {
    int maxId = 0;
    if (list && list->getHeader()) {
        LinkList<SymptomPatient>::Node *cur = list->getHeader()->next;
        while (cur != nullptr && cur->next != nullptr) {
            string idStr = cur->info.getSP_id();
            if (idStr.length() > 3 && idStr.substr(0, 3) == "SP_") {
                try {
                    int num = stoi(idStr.substr(3));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
            cur = cur->next;
        }
    }
    ostringstream oss;
    oss << "SP_" << setfill('0') << setw(3) << (maxId + 1);
    return oss.str();
}

template <>
string generateNext_id<Patient>(const LinkList<Patient> *list) {
    int maxId = 0;
    if (list && list->getHeader()) {
        LinkList<Patient>::Node *cur = list->getHeader()->next;
        while (cur != nullptr && cur->next != nullptr) {
            string idStr = cur->info.getID();
            if (idStr.length() > 2 && idStr.substr(0, 2) == "P_") {
                try {
                    int num = stoi(idStr.substr(2));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
            cur = cur->next;
        }
    }
    ostringstream oss;
    oss << "P_" << setfill('0') << setw(3) << (maxId + 1);
    return oss.str();
}

template <>
bool SaveToFile<Patient, LinkList>(const string &path, const LinkList<Patient> *list) {
    if (!list || list->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    f << "P_ID  | P_FullName              | P_Phone    | P_Gender | P_Age | P_CCCD\n";
    f << "------+-------------------------+------------+----------+-------+-------------\n";

    LinkList<Patient>::Node *tmp = list->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (!tmp->info.isDefault()) {
            tmp->info.save(f);
        }
        tmp = tmp->next;
    }

    f.close();
    return true;
}

template <>
Queue<QueuePatient>* LoadFromFile<QueuePatient, Queue>(const string &path) {
    Queue<QueuePatient> *queue = new Queue<QueuePatient>;
    if (!queue)
        return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete queue;
        return nullptr;
    }

    string tmp;
    getline(f, tmp);
    getline(f, tmp);

    while (true) {
        QueuePatient QP(f);
        if (QP.isDefault())
            break;
        queue->enQueue(QP);
    }
    f.close();
    queue->updateFirstWaiting();
    return queue;
}

template <>
bool SaveToFile<QueuePatient, Queue>(const string &path, const Queue<QueuePatient> *queue) {
    if (!queue || queue->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    f << "SP_id  | Sp_id  | State\n";
    f << "-------+--------+------\n";

    Queue<QueuePatient>::Node *tmp = queue->getHeader()->next;
    while (tmp != nullptr && tmp != queue->getFooter()) {
        if (tmp->data && !tmp->data->isDefault()) {
            tmp->data->save(f);
        }
        tmp = tmp->next;
    }

    f.close();
    return true;
}

}
