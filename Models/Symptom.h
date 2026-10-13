#ifndef SYMPTOM_H
#define SYMPTOM_H

#include <string>
#include <fstream>

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

        void setS_id(std::string S_id);
        void setSName(std::string sName);
        void setSp_id(std::string Sp_id);
        void setSScore(int sScore);
};

#endif
