#ifndef SYMPTOMPATIENT_H
#define SYMPTOMPATIENT_H

#include <string>
#include <fstream>
#include "Date.h"
#include "LinkList.h"

class SymptomPatient {
    private:
        std::string QP_id;
        std::string S_id;
        std::string P_id;
        Date date;
    public:
        SymptomPatient();
        SymptomPatient(std::ifstream &f);
        SymptomPatient(std::string QP_id, std::string S_id, std::string P_id, Date date);
        SymptomPatient(const SymptomPatient &SP);
        SymptomPatient &operator=(const SymptomPatient &SP) = default;
        ~SymptomPatient();

        void setDefault();
        bool isDefault() const;

        std::string getQP_id() const;
        std::string getS_id() const;
        std::string getP_id() const;
        Date getDate() const;

        void setQP_id(std::string QP_id);
        void setS_id(std::string S_id);
        void setP_id(std::string P_id);
        void setDate(Date date);
        void addS_id(const std::string &id);

        void save(std::ofstream &f) const;

        std::string toString() const;
        static SymptomPatient loadFromString(const std::string &str);
        static LinkList<SymptomPatient>* loadFromFile(const std::string &path);
        static bool saveToFile(const std::string &path, const LinkList<SymptomPatient> *list);
        static std::string generateNextId(const LinkList<SymptomPatient> *list);
};

#endif
