#include "Patient.h"
#include <string>
#include <fstream>
#include "libPBL2.h"
using namespace std;

Patient::Patient()
    :Human(), CCCD("") {}

Patient::Patient(std::ifstream &f) {
    string tmp;
    if (!getline(f, id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(id);

    if (!getline(f, fullName, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(fullName);

    if (!getline(f, phone, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(phone);

    if (!getline(f, tmp, '|')) {
        setDefault();
        return;
    }
    gender = stoi(tmp);

    if (!getline(f, tmp, '|')) {
        setDefault();
        return;
    }
    age = stoi(tmp);

    if (!getline(f, CCCD, '\n')) {
        setDefault();
        return;
    }
    libPBL2::trim(CCCD);
}
Patient::Patient(std::string id, std::string fullName, std::string phone, bool gender, int age, std::string CCCD)
    :Human(id, fullName, phone, gender, age), CCCD(CCCD) {}

Patient::Patient(const Patient &P)
    :Human(P.id, P.fullName, P.phone, P.gender, P.age), CCCD(P.CCCD) {}

Patient::~Patient() {}

void Patient::setDefault() {
    Patient tmp;
    *this = tmp;
}

bool Patient::isDefault() const {
    return id == ""
        && fullName == ""
        && phone == ""
        && gender == 0
        && age == 0
        && CCCD == "";
}

string Patient::getCCCD() const {return this->CCCD;}
void Patient::setCCCD(string CCCD) {this->CCCD = CCCD;}