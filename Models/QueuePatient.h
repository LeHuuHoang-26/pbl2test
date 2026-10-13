#ifndef QUEUEPATIENT_H
#define QUEUEPATIENT_H

#include <string>
#include <fstream>

class QueuePatient {
    private:
        std::string SP_id;
        std::string Sp_id;
        int state;
    public:
        QueuePatient();
        QueuePatient(std::ifstream &f);
        QueuePatient(std::string SP_id, std::string Sp_id, int state);
        QueuePatient(const QueuePatient &QP);
        QueuePatient &operator=(const QueuePatient &QP) = default;
        ~QueuePatient();

        void setDefault();
        bool isDefault() const;

        std::string getSP_id() const;
        std::string getSp_id() const;
        int getState() const;

        void setSP_id(std::string SP_id);
        void setSp_id(std::string Sp_id);
        void setState(int state);
};

#endif
