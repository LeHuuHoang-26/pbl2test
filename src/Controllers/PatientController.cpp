#include "PatientController.h"
#include "libPBL2.h"
#include <sstream>

Patient *PatientController::findById(LinkList<Patient> *patients, const std::string &id) {
    if (!patients || !patients->getHeader()) return nullptr;
    auto *node = patients->getHeader()->next;
    while (node && node->next) {
        if (node->info.getID() == id) return &node->info;
        node = node->next;
    }
    return nullptr;
}

int PatientController::count(const LinkList<Patient> *patients) {
    return patients ? patients->count : 0;
}

QStringList PatientController::getSymptomsForPatient(const std::string &patientId,
                                                    const LinkList<SymptomPatient> *spList,
                                                    const LinkList<Symptom> *sList) {
    QStringList result;
    if (!spList || !sList) return result;
    auto *spNode = spList->getHeader()->next;
    while (spNode && spNode->next) {
        if (spNode->info.getP_id() == patientId) {
            std::stringstream ss(spNode->info.getS_id());
            std::string singleId;
            while (std::getline(ss, singleId, ',')) {
                libPBL2::trim(singleId);
                auto *sNode = sList->getHeader()->next;
                while (sNode && sNode->next) {
                    if (sNode->info.getS_id() == singleId) {
                        result.append(QString::fromUtf8(sNode->info.getSName().c_str()));
                        break;
                    }
                    sNode = sNode->next;
                }
            }
        }
        spNode = spNode->next;
    }
    result.removeDuplicates();
    return result;
}

QString PatientController::getUnresolvedSymptoms(const std::string &patientId,
                                                 const LinkList<SymptomPatient> *spList,
                                                 const LinkList<Symptom> *sList,
                                                 const LinkList<Appointment> *aList) {
    (void)aList;
    QStringList names;
    if (!spList || !spList->getHeader()) return QStringLiteral("Đã khám hết");
    auto *record = spList->getHeader()->next;
    while (record && record->next) {
        if (record->info.getP_id() == patientId) {
            if (record->info.getResolved() == 1) {
                record = record->next;
                continue;
            }
            std::stringstream ids(record->info.getS_id());
            std::string id;
            while (std::getline(ids, id, ',')) {
                libPBL2::trim(id);
                if (id.empty()) continue;
                if (sList && sList->getHeader()) {
                    auto *s = sList->getHeader()->next;
                    while (s && s->next) {
                        if (s->info.getS_id() == id) {
                            names.append(QString::fromUtf8(s->info.getSName().c_str()));
                            break;
                        }
                        s = s->next;
                    }
                }
            }
        }
        record = record->next;
    }
    names.removeDuplicates();
    return names.isEmpty() ? QStringLiteral("Đã khám hết") : QStringLiteral("Chưa khám: %1").arg(names.join(QStringLiteral(", ")));
}

void PatientController::updateSymptomStatus(const std::string &patientId,
                                           LinkList<SymptomPatient> *spList,
                                           const LinkList<Appointment> *aList) {
    if (!aList || !spList || !spList->getHeader() || !aList->getHeader()) return;

    auto *appointment = aList->getHeader()->next;
    while (appointment && appointment->next) {
        std::string spId = appointment->info.getSymptomPatientId();
        int st = appointment->info.getState();
        auto *record = spList->getHeader()->next;
        while (record && record->next) {
            if (record->info.getSP_id() == spId) {
                if (patientId.empty() || record->info.getP_id() == patientId) {
                    record->info.setResolved(st);
                }
                break;
            }
            record = record->next;
        }
        appointment = appointment->next;
    }
}
