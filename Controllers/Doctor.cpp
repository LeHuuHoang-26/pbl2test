#include "Doctor.h"
#include <iostream>

using namespace std;

Doctor::Doctor(string _id, string _fullName, string _phone, 
               bool _gender, int _age, string _spId, bool _state,
               string _roomId, double _fee, double _rating)
    : spId(_spId), state(_state), roomId(_roomId), fee(_fee), rating(_rating) 
{
    this->id = _id;
    this->fullName = _fullName;
    this->phone = _phone;
    this->gender = _gender;
    this->age = _age;
}

Doctor::~Doctor() {
}

void Doctor::displayInfo() const {
    cout << "--- DOCTOR (D_ID | D_FullName | D_Phone | D_Gender | D_Age | Sp_ID | D_State | D_RoomID | D_Fee | D_Rating) ---\n"
         << "D_ID: " << id << "\n"
         << "D_FullName: " << fullName << "\n"
         << "D_Phone: " << phone << "\n"
         << "D_Gender: " << (gender ? "Nam" : "Nu") << "\n"
         << "D_Age: " << age << "\n"
         << "Sp_ID: " << spId << "\n"
         << "D_State: " << (state ? "Dang lam viec" : "Nghi") << "\n"
         << "D_RoomID: " << roomId << "\n"
         << "D_Fee: " << fee << "\n"
         << "D_Rating: " << rating << "\n";
}

string Doctor::getSpId() const { return spId; }
bool Doctor::getState() const { return state; }
string Doctor::getRoomId() const { return roomId; }
double Doctor::getFee() const { return fee; }
double Doctor::getRating() const { return rating; }

void Doctor::setSpId(string _spId) { this->spId = _spId; }
void Doctor::setState(bool _state) { this->state = _state; }
void Doctor::setRoomId(string _roomId) { this->roomId = _roomId; }
void Doctor::setFee(double _fee) { this->fee = _fee; }
void Doctor::setRating(double _rating) { this->rating = _rating; }