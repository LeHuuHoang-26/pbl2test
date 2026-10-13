#include <string>
#include <fstream>
#include "Manager.h"
#include "Human.h"
#include "libPBL2.h"
#include <iostream>
using namespace std;

Manager::Manager(): username(""), password(""), role(ManagerRole::Empty), state(0) {}

Manager::Manager(ifstream &f) {
    string tmp;
    if (!getline(f, id, '|')) {
        this->setDefault();
        return;
    }
    libPBL2::trim(id);

    if (!getline(f, fullName, '|')) {
        this->setDefault();
        return;
    }
    libPBL2::trim(fullName);

    if (!getline(f, phone, '|')) {
        this->setDefault();
        return;
    }
    libPBL2::trim(phone);

    if (!getline(f, tmp, '|')) {
        this->setDefault();
        return;
    }
    gender = stoi(tmp);

    if (!getline(f, tmp, '|')) {
        this->setDefault();
        return;
    }
    age = stoi(tmp);

    if (!getline(f, username, '|')) {
        this->setDefault();
        return;
    }
    libPBL2::trim(username);

    if (!getline(f, password, '|')) {
        this->setDefault();
        return;
    }
    libPBL2::trim(password);

    if (!getline(f, tmp, '|')) {
        this->setDefault();
        return;
    }
    switch (stoi(tmp)) {
        case 1: role = ManagerRole::Receptionist; break;
        case 2: role = ManagerRole::SpecialReceptionist; break;
        case 3: role = ManagerRole::Administrator; break;
    }

    if (!getline(f, tmp, '\n')) {
        this->setDefault();
        return;
    }
    state = stoi(tmp);
}

Manager::Manager(string id, string fullName, string phone, bool gender, int age, string username, string password, ManagerRole role, int state)
    : Human(id, fullName, phone, gender, age), username(username), password(password), role(role), state(state) {}

Manager::Manager(const Manager &M)
    : Human(M.id, M.fullName, M.phone, M.gender, M.age), username(M.username), password(M.password), role(M.role), state(M.state) {}

Manager::~Manager() {}

void Manager::setDefault() {
    Manager tmp;
    *this = tmp;
}

ostream &operator << (ostream &out, const Manager &M) {
    out << M.id << " | " << M.fullName << " | " << M.phone << " | " << (M.gender ? "Nam" : "Nữ") << " | " << M.age << endl;
    return out;
}

Manager &Manager::operator = (const Manager &M) {
    id = M.id;
    fullName = M.fullName;
    phone = M.phone;
    gender = M.gender;
    age = M.age;
    username = M.username;
    password = M.password;
    role = M.role;
    state = M.state;

    return *this;
}

bool Manager::isDefault() const {
    return id == ""
        && fullName == ""
        && phone == ""
        && gender == 0
        && age == 0
        && username == ""
        && password == ""
        && role == ManagerRole::Empty
        && state == 0;
}

bool Manager::isValid(const string &username, const string &password) const {
    return this->username == username && this->password == password;
}

ManagerRole Manager::getRole() const {
    return this->role;
}

bool Manager::isActive() const {
    return this->state;
}

void Manager::displayInfo() const {
    cout << "Thông tin Manager:\n";
    cout << "Username: " << this->username << endl;
    cout << "Password: " << this->password << endl;
    cout << "Role: ";
    cout << libPBL2::toStr<ManagerRole>(this->role);
}
