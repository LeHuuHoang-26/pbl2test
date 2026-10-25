#ifndef SYMPTOMPATIENT_H
#define SYMPTOMPATIENT_H

#include <string>
#include <fstream>
#include "Date.h"
#include "LinkList.h"

class SymptomPatient {
private:
    std::string SP_id;
    std::string S_id;
    std::string P_id;
    std::string Sp_id;
    Date date;
    int resolved; // 1: Đã khám, 0: Chưa khám, -1: Trễ hẹn

public:
    SymptomPatient();
    SymptomPatient(std::ifstream &f);
    SymptomPatient(std::string SP_id, std::string S_id, std::string P_id, std::string Sp_id = "", Date date = Date(), int resolved = 0);
    SymptomPatient(const SymptomPatient &SP);
    SymptomPatient &operator=(const SymptomPatient &SP) = default;
    ~SymptomPatient();

    void setDefault();
    bool isDefault() const;

    std::string getSP_id() const;
    std::string getS_id() const;
    std::string getP_id() const;
    std::string getSp_id() const;
    Date getDate() const;
    int getResolved() const;
    bool isResolved() const;

    void setSP_id(const std::string &SP_id);
    void setS_id(const std::string &S_id);
    void setP_id(const std::string &P_id);
    void setSp_id(const std::string &Sp_id);
    void setDate(const Date &date);
    void setResolved(int resolved);
    void addS_id(const std::string &id);

    void save(std::ofstream &f) const;

    std::string toString() const;
    static SymptomPatient loadFromString(const std::string &str);
    static LinkList<SymptomPatient>* loadFromFile(const std::string &path);
    static bool saveToFile(const std::string &path, const LinkList<SymptomPatient> *list);
    static std::string generateNextId(const LinkList<SymptomPatient> *list);
};

#endif
