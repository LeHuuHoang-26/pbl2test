#ifndef PATIENTCONTROLLER_H
#define PATIENTCONTROLLER_H

#include "../Models/Patient.h"
#include "../Models/Symptom.h"
#include "../Models/SymptomPatient.h"
#include "../Models/Appointment.h"
#include "../DataStructures/LinkList.h"
#include <string>
#include <QString>
#include <QStringList>

class PatientController {
public:
    static Patient *findById(LinkList<Patient> *patients, const std::string &id);
    static int count(const LinkList<Patient> *patients);
    static QStringList getSymptomsForPatient(const std::string &patientId,
                                            const LinkList<SymptomPatient> *spList,
                                            const LinkList<Symptom> *sList);
    static QString getUnresolvedSymptoms(const std::string &patientId,
                                         const LinkList<SymptomPatient> *spList,
                                         const LinkList<Symptom> *sList,
                                         const LinkList<Appointment> *aList);
    static void updateSymptomStatus(const std::string &patientId,
                                    LinkList<SymptomPatient> *spList,
                                    const LinkList<Appointment> *aList);
};

#endif // PATIENTCONTROLLER_H
