#include "Doctor.h"
#include "libPBL2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

Doctor::Doctor(string _id, string _fullName, string _phone, bool _gender, int _age, string _spId, bool _state, string _roomId, double _fee, double _rating)
    : Human(_id, _fullName, _phone, _gender, _age), spId(_spId), state(_state), roomId(_roomId), fee(_fee), rating(_rating) {}

Doctor::~Doctor() {}

void Doctor::displayInfo() const {
    cout << "--- DOCTOR (D_ID | D_FullName | D_Phone | D_Gender | D_Age | Sp_ID | D_State | D_RoomID | D_Fee | D_Rating) ---\n"
         << "D_ID: " << id << "\n"
         << "D_FullName: " << fullName << "\n"
         << "D_Phone: " << phone << "\n"
         << "D_Gender: " << (gender ? "Nam" : "Nu") << "\n"
         << "D_Age: " << age << "\n"
         << "Sp_ID: " << spId << "\n"
         << "D_State: " << (state ? "Dang lam viec" : "Nghi") << "\n"
         << "D_RoomID: " << roomId << "\n"
         << "D_Fee: " << fee << "\n"
         << "D_Rating: " << rating << "\n";
}

bool Doctor::isDefault() const {
    return id.empty() && fullName.empty();
}

void Doctor::setDefault() {
    id = "";
    fullName = "";
    phone = "";
    gender = true;
    age = 0;
    spId = "";
    state = true;
    roomId = "";
    fee = 0.0;
    rating = 5.0;
}

string Doctor::getSpId() const { return spId; }
bool Doctor::getState() const { return state; }
string Doctor::getRoomId() const { return roomId; }
double Doctor::getFee() const { return fee; }
double Doctor::getRating() const { return rating; }

void Doctor::setSpId(string _spId) { this->spId = _spId; }
void Doctor::setState(bool _state) { this->state = _state; }
void Doctor::setRoomId(string _roomId) { this->roomId = _roomId; }
void Doctor::setFee(double _fee) { this->fee = _fee; }
void Doctor::setRating(double _rating) { this->rating = _rating; }

string Doctor::toString() const {
    ostringstream oss;
    oss << libPBL2::padRightUtf8(id, 6) << " | "
        << libPBL2::padRightUtf8(fullName, 20) << " | "
        << libPBL2::padRightUtf8(phone, 10) << " | "
        << libPBL2::padRightUtf8(gender ? "1" : "0", 8) << " | "
        << libPBL2::padRightUtf8(to_string(age), 5) << " | "
        << libPBL2::padRightUtf8(spId, 6) << " | "
        << libPBL2::padRightUtf8(state ? "1" : "0", 7) << " | "
        << libPBL2::padRightUtf8(roomId, 8) << " | "
        << libPBL2::padRightUtf8([&]() {
            ostringstream fss;
            fss << fixed << setprecision(1) << fee;
            return fss.str();
        }(), 16) << " | "
        << libPBL2::padRightUtf8([&]() {
            ostringstream rss;
            rss << fixed << setprecision(1) << rating;
            return rss.str();
        }(), 6);
    return oss.str();
}

Doctor Doctor::loadFromString(const string &str) {
    if (str.empty()) return Doctor();
    stringstream ss(str);
    string id, fullName, phone, genderStr, ageStr;
    string spId, stateStr, roomId, feeStr, ratingStr;

    if (!getline(ss, id, '|') ||
        !getline(ss, fullName, '|') ||
        !getline(ss, phone, '|') ||
        !getline(ss, genderStr, '|') ||
        !getline(ss, ageStr, '|') ||
        !getline(ss, spId, '|') ||
        !getline(ss, stateStr, '|') ||
        !getline(ss, roomId, '|') ||
        !getline(ss, feeStr, '|') ||
        !getline(ss, ratingStr)) {
        return Doctor();
    }

    libPBL2::trim(id);
    libPBL2::trim(fullName);
    libPBL2::trim(phone);
    libPBL2::trim(genderStr);
    libPBL2::trim(ageStr);
    libPBL2::trim(spId);
    libPBL2::trim(stateStr);
    libPBL2::trim(roomId);
    libPBL2::trim(feeStr);
    libPBL2::trim(ratingStr);

    if (id.empty()) return Doctor();

    bool gender = (genderStr == "1");
    int age = (!ageStr.empty() ? stoi(ageStr) : 0);
    bool state = (stateStr == "1");
    double fee = (!feeStr.empty() ? stod(feeStr) : 0.0);
    double rating = (!ratingStr.empty() ? stod(ratingStr) : 0.0);

    return Doctor(id, fullName, phone, gender, age, spId, state, roomId, fee, rating);
}

LinkList<Doctor>* Doctor::loadFromFile(const string &path) {
    LinkList<Doctor> *doctorList = new LinkList<Doctor>;
    if (!doctorList) return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete doctorList;
        return nullptr;
    }

    string tmp;
    getline(f, tmp); // Bỏ qua tiêu đề
    getline(f, tmp); // Bỏ qua dòng phân cách

    string line;
    while (getline(f, line)) {
        libPBL2::trim(line);
        if (line.empty()) continue;
        Doctor doc = Doctor::loadFromString(line);
        if (!doc.isDefault()) {
            doctorList->insert(doc);
        }
    }

    f.close();
    return doctorList;
}

bool Doctor::saveToFile(const string &path, const LinkList<Doctor> *list) {
    if (!list || list->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    f << "D_ID   | D_FullName           | D_Phone    | D_Gender | D_Age | Sp_ID  | D_State | Room_ID  | Consultation_Fee | Rating\n";
    f << "-------+----------------------+------------+----------+-------+--------+---------+----------+------------------+-------\n";

    LinkList<Doctor>::Node *tmp = list->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (!tmp->info.isDefault()) {
            f << tmp->info.toString() << "\n";
        }
        tmp = tmp->next;
    }

    f.close();
    return true;
}
