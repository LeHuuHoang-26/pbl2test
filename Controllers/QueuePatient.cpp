#include "QueuePatient.h"
#include "SymptomPatient.h"
#include <string>
#include <fstream>
#include <iomanip>
#include "libPBL2.h"
using namespace std;

QueuePatient::QueuePatient()
    : QP_id(""), Sp_id(""), state(0) {}

QueuePatient::QueuePatient(std::ifstream &f) {
    string tmp;
    if (!getline(f, QP_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(QP_id);

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
    state = stoi(tmp);
}

QueuePatient::QueuePatient(std::string QP_id, std::string Sp_id, int state)
    : QP_id(QP_id), Sp_id(Sp_id), state(state) {}

QueuePatient::QueuePatient(const SymptomPatient &sp, std::string Sp_id, int state)
    : QP_id(sp.getQP_id()), Sp_id(Sp_id), state(state) {}

QueuePatient::QueuePatient(const QueuePatient &QP)
    : QP_id(QP.QP_id), Sp_id(QP.Sp_id), state(QP.state) {}

QueuePatient::~QueuePatient() {}

void QueuePatient::setDefault() {
    QueuePatient tmp;
    *this = tmp;
}

bool QueuePatient::isDefault() const {
    return QP_id == "" && Sp_id == "" && state == 0;
}

string QueuePatient::getQP_id() const { return QP_id; }
string QueuePatient::getSp_id() const { return Sp_id; }
int QueuePatient::getState() const { return state; }

void QueuePatient::setQP_id(string QP_id) { this->QP_id = QP_id; }
void QueuePatient::setSp_id(string Sp_id) { this->Sp_id = Sp_id; }
void QueuePatient::setState(int state) { this->state = state; }

void QueuePatient::setFromSymptomPatient(const SymptomPatient &sp, std::string Sp_id, int state) {
    this->QP_id = sp.getQP_id();
    if (!Sp_id.empty()) {
        this->Sp_id = Sp_id;
    }
    this->state = state;
}

void QueuePatient::save(std::ofstream &f) const {
    f << left << setw(6) << QP_id << " | "
      << setw(6) << Sp_id << " | "
      << state << "\n";
}
