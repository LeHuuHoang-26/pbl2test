#ifndef SYMPTOMPATIENT_H
#define SYMPTOMPATIENT_H

#include <string>
#include <fstream>
#include "Date.h"

class SymptomPatient {
    private:
        std::string SP_id;
        std::string S_id;
        std::string P_id;
        Date date;
    public:
        SymptomPatient();
        SymptomPatient(std::ifstream &f);
        SymptomPatient(std::string SP_id, std::string S_id, std::string P_id, Date date);
        SymptomPatient(const SymptomPatient &SP);
        SymptomPatient &operator=(const SymptomPatient &SP) = default;
        ~SymptomPatient();

        void setDefault();
        bool isDefault() const;

        std::string getSP_id() const;
        std::string getS_id() const;
        std::string getP_id() const;
        Date getDate() const;

        void setSP_id(std::string SP_id);
        void setS_id(std::string S_id);
        void setP_id(std::string P_id);
        void setDate(Date date);
        void addS_id(const std::string &id);

        void save(std::ofstream &f) const;
};

#endif
