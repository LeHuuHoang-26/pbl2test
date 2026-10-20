#ifndef PATIENTCONTROLLER_H
#define PATIENTCONTROLLER_H
#include "../Models/Patient.h"
#include "../DataStructures/LinkList.h"
#include <string>
class PatientController {
public:
    static Patient *findById(LinkList<Patient> *patients, const std::string &id);
};
#endif
