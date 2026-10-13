#ifndef PATIENT_H
#define PATIENT_H
#include "human.h"
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
};

#endif
