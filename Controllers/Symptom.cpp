#include "Symptom.h"
#include <string>
#include <fstream>
#include "libPBL2.h"
using namespace std;

Symptom::Symptom()
    : S_id(""), sName(""), Sp_id(""), sScore(0) {}

Symptom::Symptom(std::ifstream &f) {
    string tmp;
    if (!getline(f, S_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(S_id);

    if (!getline(f, sName, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(sName);

    if (!getline(f, Sp_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(Sp_id);

    if (!getline(f, tmp, '\n')) {
        setDefault();
        return;
    }
    libPBL2::trim(tmp);
    if (tmp.empty()) {
        setDefault();
        return;
    }
    sScore = stoi(tmp);
}

Symptom::Symptom(std::string S_id, std::string sName, std::string Sp_id, int sScore)
    : S_id(S_id), sName(sName), Sp_id(Sp_id), sScore(sScore) {}

Symptom::Symptom(const Symptom &S)
    : S_id(S.S_id), sName(S.sName), Sp_id(S.Sp_id), sScore(S.sScore) {}

Symptom::~Symptom() {}

void Symptom::setDefault() {
    Symptom tmp;
    *this = tmp;
}

bool Symptom::isDefault() const {
    return S_id == "" && sName == "" && Sp_id == "" && sScore == 0;
}

string Symptom::getS_id() const { return S_id; }
string Symptom::getSName() const { return sName; }
string Symptom::getSp_id() const { return Sp_id; }
int Symptom::getSScore() const { return sScore; }

void Symptom::setS_id(string S_id) { this->S_id = S_id; }
void Symptom::setSName(string sName) { this->sName = sName; }
void Symptom::setSp_id(string Sp_id) { this->Sp_id = Sp_id; }
void Symptom::setSScore(int sScore) { this->sScore = sScore; }
