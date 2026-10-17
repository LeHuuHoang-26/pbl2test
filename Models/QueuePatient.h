#ifndef QUEUEPATIENT_H
#define QUEUEPATIENT_H

#include <string>
#include <fstream>
#include "Queue.h"

class SymptomPatient;

class QueuePatient {
    private:
        std::string QP_id;
        std::string Sp_id;
        int state;
    public:
        QueuePatient();
        QueuePatient(std::ifstream &f);
        QueuePatient(std::string QP_id, std::string Sp_id, int state);
        QueuePatient(const SymptomPatient &sp, std::string Sp_id = "", int state = 0);
        QueuePatient(const QueuePatient &QP);
        QueuePatient &operator=(const QueuePatient &QP) = default;
        ~QueuePatient();

        void setDefault();
        bool isDefault() const;

        std::string getQP_id() const;
        std::string getSp_id() const;
        int getState() const;

        void setQP_id(std::string QP_id);
        void setSp_id(std::string Sp_id);
        void setState(int state);
        void setFromSymptomPatient(const SymptomPatient &sp, std::string Sp_id = "", int state = 0);

        void save(std::ofstream &f) const;

        std::string toString() const;
        static QueuePatient loadFromString(const std::string &str);
        static Queue<QueuePatient>* loadFromFile(const std::string &path);
        static bool saveToFile(const std::string &path, const Queue<QueuePatient> *queue);
};

#endif
