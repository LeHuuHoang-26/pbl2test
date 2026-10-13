#include "QueuePatient.h"
#include <string>
#include <fstream>
#include "libPBL2.h"
using namespace std;

QueuePatient::QueuePatient()
    : SP_id(""), Sp_id(""), state(0) {}

QueuePatient::QueuePatient(std::ifstream &f) {
    string tmp;
    if (!getline(f, SP_id, '|')) {
        setDefault();
        return;
    }
    libPBL2::trim(SP_id);

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

QueuePatient::QueuePatient(std::string SP_id, std::string Sp_id, int state)
    : SP_id(SP_id), Sp_id(Sp_id), state(state) {}

QueuePatient::QueuePatient(const QueuePatient &QP)
    : SP_id(QP.SP_id), Sp_id(QP.Sp_id), state(QP.state) {}

QueuePatient::~QueuePatient() {}

void QueuePatient::setDefault() {
    QueuePatient tmp;
    *this = tmp;
}

bool QueuePatient::isDefault() const {
    return SP_id == "" && Sp_id == "" && state == 0;
}

string QueuePatient::getSP_id() const { return SP_id; }
string QueuePatient::getSp_id() const { return Sp_id; }
int QueuePatient::getState() const { return state; }

void QueuePatient::setSP_id(string SP_id) { this->SP_id = SP_id; }
void QueuePatient::setSp_id(string Sp_id) { this->Sp_id = Sp_id; }
void QueuePatient::setState(int state) { this->state = state; }
