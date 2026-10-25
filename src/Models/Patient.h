#ifndef PATIENT_H
#define PATIENT_H

#include "Human.h"
#include "LinkList.h"
#include <string>
#include <fstream>

class Patient: public Human {
    private:
        std::string CCCD;
    public:
        Patient();
        Patient(std::ifstream &f);
        Patient(std::string id, std::string fullName, std::string phone, bool gender, int age, std::string CCCD);
        Patient(const Patient &P);
        Patient &operator=(const Patient &P) = default;
        ~Patient();

        void setDefault();
        bool isDefault() const;
        std::string getCCCD() const;
        void setCCCD(std::string CCCD);

        void save(std::ofstream &f) const;

        std::string toString() const;
        static Patient loadFromString(const std::string &str);
        static LinkList<Patient>* loadFromFile(const std::string &path);
        static bool saveToFile(const std::string &path, const LinkList<Patient> *list);
        static std::string generateNextId(const LinkList<Patient> *list);
};

#endif
