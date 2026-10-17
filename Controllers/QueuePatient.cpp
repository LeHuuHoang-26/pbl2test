#include "QueuePatient.h"
#include "SymptomPatient.h"
#include <string>
#include <fstream>
#include <sstream>
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

void QueuePatient::setFromSymptomPatient(const SymptomPatient &sp, string Sp_id, int state) {
    this->QP_id = sp.getQP_id();
    this->Sp_id = Sp_id;
    this->state = state;
}

void QueuePatient::save(std::ofstream &f) const {
    f << toString() << "\n";
}

string QueuePatient::toString() const {
    return libPBL2::padRightUtf8(QP_id, 6) + " | "
         + libPBL2::padRightUtf8(Sp_id, 6) + " | "
         + to_string(state);
}

QueuePatient QueuePatient::loadFromString(const string &str) {
    if (str.empty()) return QueuePatient();
    stringstream ss(str);
    string qpId, spId, stateStr;

    if (!getline(ss, qpId, '|') ||
        !getline(ss, spId, '|') ||
        !getline(ss, stateStr)) {
        return QueuePatient();
    }

    libPBL2::trim(qpId);
    libPBL2::trim(spId);
    libPBL2::trim(stateStr);

    if (qpId.empty()) return QueuePatient();
    int st = (!stateStr.empty() ? stoi(stateStr) : 0);

    return QueuePatient(qpId, spId, st);
}

Queue<QueuePatient>* QueuePatient::loadFromFile(const string &path) {
    Queue<QueuePatient> *queue = new Queue<QueuePatient>;
    if (!queue) return nullptr;

    ifstream f(path);
    if (!f.is_open()) {
        delete queue;
        return nullptr;
    }

    string tmp;
    getline(f, tmp); // Bỏ qua tiêu đề
    getline(f, tmp); // Bỏ qua dòng phân cách

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

bool QueuePatient::saveToFile(const string &path, const Queue<QueuePatient> *queue) {
    if (!queue || queue->getHeader() == nullptr)
        return false;

    ofstream f(path);
    if (!f.is_open())
        return false;

    f << "QP_id  | Sp_id  | State\n";
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
