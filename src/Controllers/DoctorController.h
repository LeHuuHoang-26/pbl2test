#ifndef DOCTORCONTROLLER_H
#define DOCTORCONTROLLER_H
#include "../Models/Doctor.h"
#include "../DataStructures/LinkList.h"
#include <string>
class DoctorController {
public:
    static Doctor *findById(LinkList<Doctor> *doctors, const std::string &id);
};
#endif
