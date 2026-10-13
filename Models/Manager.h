#ifndef MANAGER_H
#define MANAGER_H

#include <string>
#include <fstream>
#include <ostream>
#include "human.h"

enum class ManagerRole {
    Empty,
    Receptionist,
    SpecialReceptionist,
    Administrator
};

class Manager: public Human {
    private:
        std::string username;
        std::string password;
        ManagerRole role;
        int state;
    public:
        Manager(std::ifstream &f);
        Manager(std::string id, std::string fullName, std::string phone, bool gender, int age, std::string username, std::string password, ManagerRole role, int state);
        Manager(const Manager &M);
        Manager();
        ~Manager();

        ManagerRole getRole() const;
        void setDefault();
        bool isDefault() const;
        friend std::ostream &operator << (std::ostream &out, const Manager &M);
        Manager &operator = (const Manager &M);
        bool isValid(const std::string &username, const std::string &password) const;
        bool isActive() const;

        void displayInfo() const override;

        
};

#endif