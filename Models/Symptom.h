#ifndef SYMPTOM_H
#define SYMPTOM_H

#include <string>
#include <fstream>
#include "LinkList.h"

class Symptom {
    private:
        std::string S_id;
        std::string sName;
        std::string Sp_id;
        int sScore;
    public:
        Symptom();
        Symptom(std::ifstream &f);
        Symptom(std::string S_id, std::string sName, std::string Sp_id, int sScore);
        Symptom(const Symptom &S);
        Symptom &operator=(const Symptom &S) = default;
        ~Symptom();

        void setDefault();
        bool isDefault() const;

        std::string getS_id() const;
        std::string getSName() const;
        std::string getSp_id() const;
        int getSScore() const;

        void setS_id(std::string id);
        void setSName(std::string name);
        void setSp_id(std::string spId);
        void setSScore(int score);

        void save(std::ofstream &f) const;
        std::string toString() const;
        static Symptom loadFromString(const std::string &str);
        static LinkList<Symptom>* loadFromFile(const std::string &path);
        static bool saveToFile(const std::string &path, const LinkList<Symptom> *list);
};

#endif
