#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Appointment.h"
#include "libPBL2.h"

using namespace std;

Appointment::Appointment(string id, string symptomPatientId, string doctorId, const Date &dateSet, const Date &dateGet, int state)
    : id(id), symptomPatientId(symptomPatientId), doctorId(doctorId), dateSet(dateSet), dateGet(dateGet), state(state) {}

string Appointment::getId() const { return id; }
string Appointment::getSymptomPatientId() const { return symptomPatientId; }
string Appointment::getDoctorId() const { return doctorId; }
Date Appointment::getDateSet() const { return dateSet; }
Date Appointment::getDateGet() const { return dateGet; }
int Appointment::getState() const { return state; }

void Appointment::setId(const string &value) { id = value; }
void Appointment::setSymptomPatientId(const string &value) { symptomPatientId = value; }
void Appointment::setDoctorId(const string &value) { doctorId = value; }
void Appointment::setDateSet(const Date &value) { dateSet = value; }
void Appointment::setDateGet(const Date &value) { dateGet = value; }
void Appointment::setState(int value) { state = value; }

string Appointment::stateToString(int s) {
    switch (s) {
        case 1: return "Đã khám";
        case -1: return "Trễ hẹn";
        case 0:
        default: return "Chưa khám";
    }
}

int Appointment::stringToState(const string &str) {
    if (str == "1" || str == "Đã khám" || str == "Completed") return 1;
    if (str == "-1" || str == "Trễ hẹn" || str == "Delayed") return -1;
    return 0; // Chưa khám
}

string Appointment::getStateString() const {
    return stateToString(state);
}

int Appointment::computeEffectiveState(int curDay, int curMonth, int curYear, int curHour, int curMin) const {
    if (state == 1) return 1;   // Đã khám
    if (state == -1) return -1; // Trễ hẹn

    Date ad = getDateGet();
    if (ad.getYear() < curYear ||
       (ad.getYear() == curYear && ad.getMonth() < curMonth) ||
       (ad.getYear() == curYear && ad.getMonth() == curMonth && ad.getDay() < curDay)) {
        return -1; // Trễ hẹn (ngày hẹn đã qua)
    }
    if (ad.getYear() == curYear && ad.getMonth() == curMonth && ad.getDay() == curDay) {
        if (ad.getHour() < curHour || (ad.getHour() == curHour && ad.getMinute() <= curMin)) {
            return -1; // Trễ hẹn (hôm nay nhưng đã quá giờ hẹn)
        }
    }
    return 0; // Chưa khám (chưa đến giờ)
}

void Appointment::displayInfo() const {
    cout << "--- APPOINTMENT ---\n"
         << "A_ID: " << id << "\n"
         << "SP_ID: " << symptomPatientId << "\n"
         << "D_ID: " << doctorId << "\n"
         << "A_DateSet: " << dateSet.toString() << "\n"
         << "A_DateGet: " << dateGet.toString() << "\n"
         << "A_State: " << state << "\n";
}

bool Appointment::isDefault() const {
    return id.empty() && symptomPatientId.empty() && doctorId.empty();
}

void Appointment::setDefault() {
    id = "";
    symptomPatientId = "";
    doctorId = "";
    dateSet = Date();
    dateGet = Date();
    state = 0;
}

string Appointment::toString() const {
    string stStr = to_string(state);
    if (stStr.length() < 7) {
        stStr = string(7 - stStr.length(), ' ') + stStr;
    }
    return libPBL2::padRightUtf8(id, 5) + " | "
         + libPBL2::padRightUtf8(symptomPatientId, 6) + " | "
         + libPBL2::padRightUtf8(doctorId, 5) + " | "
         + libPBL2::padRightUtf8(dateSet.toString(), 16) + " | "
         + libPBL2::padRightUtf8(dateGet.toString(), 16) + " | "
         + stStr;
}

Appointment Appointment::loadFromString(const string &str) {
    if (str.empty()) return Appointment();
    stringstream ss(str);
    string id, spId, doctorId, dateSetStr, dateGetStr, stateStr;

    if (!getline(ss, id, '|') ||
        !getline(ss, spId, '|') ||
        !getline(ss, doctorId, '|') ||
        !getline(ss, dateSetStr, '|') ||
        !getline(ss, dateGetStr, '|') ||
        !getline(ss, stateStr)) {
        return Appointment();
    }

    libPBL2::trim(id);
    libPBL2::trim(spId);
    libPBL2::trim(doctorId);
    libPBL2::trim(dateSetStr);
    libPBL2::trim(dateGetStr);
    libPBL2::trim(stateStr);

    if (id.empty() || spId.empty() || doctorId.empty())
        return Appointment();

    Date dSet = Date::loadFromString(dateSetStr);
    Date dGet = Date::loadFromString(dateGetStr);
    int st = (!stateStr.empty() ? stoi(stateStr) : 0);

    return Appointment(id, spId, doctorId, dSet, dGet, st);
}

LinkList<Appointment>* Appointment::loadFromFile(const string &path) {
    LinkList<Appointment> *list = new LinkList<Appointment>;
    if (!list) return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete list;
        return nullptr;
    }

    string tmp;
    getline(f, tmp); // Bỏ qua tiêu đề
    getline(f, tmp); // Bỏ qua dòng phân cách

    string line;
    while (getline(f, line)) {
        libPBL2::trim(line);
        if (line.empty()) continue;
        Appointment app = Appointment::loadFromString(line);
        if (!app.isDefault()) {
            list->insert(app);
        }
    }

    f.close();
    return list;
}

bool Appointment::saveToFile(const string &path, const LinkList<Appointment> *list) {
    if (!list || list->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    f << "A_ID  | SP_ID  | D_ID  | A_DateSet        | A_DateGet        | A_State\n";
    f << "------+--------+-------+------------------+------------------+---------\n";

    LinkList<Appointment>::Node *tmp = list->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (!tmp->info.isDefault()) {
            f << tmp->info.toString() << "\n";
        }
        tmp = tmp->next;
    }

    f.close();
    return true;
}
