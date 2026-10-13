#ifndef HUMAN_H
#define HUMAN_H

#include <string>

class Human {
protected: 
    std::string id;
    std::string fullName;
    std::string phone;
    bool gender;
    int age; 

public:
    Human(std::string id = "", std::string fullName = "", std::string phone = "", bool gender = false, int age = 0);
    virtual ~Human() = default;
    friend bool operator ==(const Human &, const Human &);
    virtual void displayInfo() const;

    std::string getID() const;
    std::string getfullName() const;
    std::string getPhone() const;
    bool getGender() const;
    int getAge() const;

    void setID(std::string id);
    void setfullName(std::string fullName);
    void setPhone(std::string phone);
    void setGender(bool gender);
    void setAge(int age);
};

#endif 