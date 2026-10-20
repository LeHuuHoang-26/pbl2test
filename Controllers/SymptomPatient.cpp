#include "SymptomPatient.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "libPBL2.h"

using namespace std;

SymptomPatient::SymptomPatient()
    : QP_id(""), S_id(""), P_id(""), date(0, 0, 0, 0, 0) {}

SymptomPatient::SymptomPatient(std::ifstream &f) {
    setDefault();
    string line;
    bool foundOpenBrace = false;

    while (getline(f, line)) {
        libPBL2::trim(line);
        if (line.empty()) continue;
        if (line.find('{') != string::npos) {
            foundOpenBrace = true;
            break;
        }
    }

    if (!foundOpenBrace) {
        setDefault();
        return;
    }

    while (getline(f, line)) {
        libPBL2::trim(line);
        if (line.empty()) continue;

        if (line.find('}') != string::npos) {
            break;
        }

        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string key = line.substr(0, colonPos);
            string val = line.substr(colonPos + 1);
            libPBL2::trim(key);
            libPBL2::trim(val);

            if (!val.empty() && val.back() == ',') {
                val.pop_back();
                libPBL2::trim(val);
            }

            if (key == "QP_id" || key == "SP_id") {
                this->QP_id = val;
            } else if (key == "S_id") {
                this->S_id = val;
            } else if (key == "P_id") {
                this->P_id = val;
            } else if (key == "Date") {
                this->date = Date::loadFromString(val);
            }
        }
    }

    if (this->QP_id.empty()) {
        setDefault();
    }
}

SymptomPatient::SymptomPatient(std::string QP_id, std::string S_id, std::string P_id, Date date)
    : QP_id(QP_id), S_id(S_id), P_id(P_id), date(date) {}

SymptomPatient::SymptomPatient(const SymptomPatient &SP)
    : QP_id(SP.QP_id), S_id(SP.S_id), P_id(SP.P_id), date(SP.date) {}

SymptomPatient::~SymptomPatient() {}

void SymptomPatient::setDefault() {
    SymptomPatient tmp;
    *this = tmp;
}

bool SymptomPatient::isDefault() const {
    return QP_id == "" && S_id == "" && P_id == "" && date.isDefault();
}

string SymptomPatient::getQP_id() const { return QP_id; }
string SymptomPatient::getS_id() const { return S_id; }
string SymptomPatient::getP_id() const { return P_id; }
Date SymptomPatient::getDate() const { return date; }

void SymptomPatient::setQP_id(string QP_id) { this->QP_id = QP_id; }
void SymptomPatient::setS_id(string S_id) { this->S_id = S_id; }
void SymptomPatient::setP_id(string P_id) { this->P_id = P_id; }
void SymptomPatient::setDate(Date date) { this->date = date; }

void SymptomPatient::addS_id(const string &id) {
    if (this->S_id.empty()) {
        this->S_id = id;
    } else {
        this->S_id += ", " + id;
    }
}

void SymptomPatient::save(std::ofstream &f) const {
    f << toString() << "\n";
}

string SymptomPatient::toString() const {
    ostringstream oss;
    oss << "{\n"
        << "\tQP_id: " << QP_id << ",\n"
        << "\tS_id: " << S_id << ",\n"
        << "\tP_id: " << P_id << ",\n"
        << "\tDate: " << date.toString() << ",\n"
        << "},";
    return oss.str();
}

SymptomPatient SymptomPatient::loadFromString(const string &str) {
    stringstream ss(str);
    SymptomPatient sp;
    string line;
    while (getline(ss, line)) {
        libPBL2::trim(line);
        if (line.empty() || line == "{" || line == "}," || line == "}") continue;
        size_t colon = line.find(':');
        if (colon != string::npos) {
            string key = line.substr(0, colon);
            string val = line.substr(colon + 1);
            libPBL2::trim(key);
            libPBL2::trim(val);
            if (!val.empty() && val.back() == ',') val.pop_back();
            libPBL2::trim(val);

            if (key == "QP_id" || key == "SP_id") sp.QP_id = val;
            else if (key == "S_id") sp.S_id = val;
            else if (key == "P_id") sp.P_id = val;
            else if (key == "Date") sp.date = Date::loadFromString(val);
        }
    }
    return sp;
}

LinkList<SymptomPatient>* SymptomPatient::loadFromFile(const string &path) {
    LinkList<SymptomPatient> *SPList = new LinkList<SymptomPatient>;
    if (!SPList) return nullptr;

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

bool SymptomPatient::saveToFile(const string &path, const LinkList<SymptomPatient> *list) {
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

string SymptomPatient::generateNextId(const LinkList<SymptomPatient> *list) {
    int maxId = 0;
    if (list && list->getHeader()) {
        LinkList<SymptomPatient>::Node *cur = list->getHeader()->next;
        while (cur != nullptr && cur->next != nullptr) {
            string idStr = cur->info.getQP_id();
            if (idStr.length() > 3 && (idStr.substr(0, 3) == "QP_" || idStr.substr(0, 3) == "SP_")) {
                try {
                    int num = stoi(idStr.substr(3));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
            cur = cur->next;
        }
    }
    ostringstream oss;
    oss << "QP_" << setfill('0') << setw(3) << (maxId + 1);
    return oss.str();
}
