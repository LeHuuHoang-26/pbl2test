#include "SymptomPatient.h"
#include <string>
#include <fstream>
#include <iomanip>
#include "libPBL2.h"
using namespace std;

SymptomPatient::SymptomPatient()
    : SP_id(""), S_id(""), P_id(""), date(0, 0, 0) {}

SymptomPatient::SymptomPatient(std::ifstream &f) {
    if (!getline(f, SP_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(SP_id);

    if (!getline(f, S_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(S_id);

    if (!getline(f, P_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(P_id);

    string dateStr;
    if (!getline(f, dateStr, '\n')) {
        setDefault();
        return;
    }
    libPBL2::trim(dateStr);
    date = Date::fromString(dateStr);
}

SymptomPatient::SymptomPatient(std::string SP_id, std::string S_id, std::string P_id, Date date)
    : SP_id(SP_id), S_id(S_id), P_id(P_id), date(date) {}

SymptomPatient::SymptomPatient(const SymptomPatient &SP)
    : SP_id(SP.SP_id), S_id(SP.S_id), P_id(SP.P_id), date(SP.date) {}

SymptomPatient::~SymptomPatient() {}

void SymptomPatient::setDefault() {
    SymptomPatient tmp;
    *this = tmp;
}

bool SymptomPatient::isDefault() const {
    return SP_id == "" && S_id == "" && P_id == "" && date.isDefault();
}

string SymptomPatient::getSP_id() const { return SP_id; }
string SymptomPatient::getS_id() const { return S_id; }
string SymptomPatient::getP_id() const { return P_id; }
Date SymptomPatient::getDate() const { return date; }

void SymptomPatient::setSP_id(string SP_id) { this->SP_id = SP_id; }
void SymptomPatient::setS_id(string S_id) { this->S_id = S_id; }
void SymptomPatient::setP_id(string P_id) { this->P_id = P_id; }
void SymptomPatient::setDate(Date date) { this->date = date; }

void SymptomPatient::save(std::ofstream &f) const {
    f << left << setw(6) << SP_id << " | "
      << setw(5) << S_id << " | "
      << setw(5) << P_id << " | "
      << date.toString() << "\n";
}
