#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "Date.h"
#include "LinkList.h"
#include <string>

class SymptomPatient;

class Appointment {
private:
    std::string id;
    std::string symptomPatientId; // SP_id (SymptomPatient)
    std::string doctorId;
    Date dateSet;
    Date dateGet;
    int state;

public:
    enum State {
        Delayed = -1,  // Trễ hẹn
        Pending = 0,   // Chưa khám
        Completed = 1  // Đã khám
    };

    Appointment(std::string id = "", std::string symptomPatientId = "", std::string doctorId = "", const Date &dateSet = Date(), const Date &dateGet = Date(), int state = 0);

    std::string getId() const;
    std::string getSymptomPatientId() const;
    std::string getDoctorId() const;
    Date getDateSet() const;
    Date getDateGet() const;
    int getState() const;

    void setId(const std::string &id);
    void setSymptomPatientId(const std::string &spId);
    void setDoctorId(const std::string &doctorId);
    void setDateSet(const Date &dateSet);
    void setDateGet(const Date &dateGet);
    void setState(int state);

    std::string getStateString() const;
    static std::string stateToString(int state);
    static int stringToState(const std::string &str);
    int computeEffectiveState(int currentDay, int currentMonth, int currentYear, int currentHour, int currentMinute) const;

    void displayInfo() const;

    bool isDefault() const;
    void setDefault();

    std::string toString() const;
    static Appointment loadFromString(const std::string &str);
    static LinkList<Appointment>* loadFromFile(const std::string &path);
    static bool saveToFile(const std::string &path, const LinkList<Appointment> *list);
};

#endif
