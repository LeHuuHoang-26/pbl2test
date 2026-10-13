#define _CRT_SECURE_NO_WARNINGS
#include "Date.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
using namespace std;

Date::Date(int day, int month, int year): day(day), month(month), year(year) {}

Date::Date(const Date &D) {
    this->day = D.day;
    this->month = D.month;
    this->year = D.year;
}

Date::~Date() {}

istream &operator >> (istream &in, Date &D) {
    cout << "Date (dd/mm/yyyy): ";
    char c;
    int flag;
    do {
        flag = 0;
        in >> D.day >> c >> D.month >> c >> D.year;
        if (D.year < 1 || D.month < 1 || D.month > 12) {
            int dayinmonth = D.posofdayinmonth();
            if (D.day < 1 || D.day > dayinmonth)
                flag = 1;
        }
        else
            flag = 1;

        if (flag)
            cout << "Nhap lai: ";
    } while (flag);

    return in;
}

ostream &operator << (ostream &out, const Date &D) {
    out << "Date (dd/mm/yyyy): ";
    out << setfill('0');
    out << setw(2) << D.day << '/'
        << setw(2) << D.month << '/'
        << setw(4) << D.year;

    out << endl;
    return out;
}

int Date::posofdayinmonth(int monthVar) const {
    if (monthVar == 0)
        monthVar = month;
    int dayinmonth_list[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (this->isLeapYear() && monthVar == 2)
        return 29;
    return dayinmonth_list[monthVar];
}

bool Date::isLeapYear(int yearVar) const {
    if (yearVar == 0)
        yearVar = year;
    if ((yearVar % 4 == 0 && yearVar % 100 != 0) || yearVar % 400 == 0)
        return 1;
    return 0;
}

Date Date::operator ++ (int) {
    Date tmp = *this;
    int dayinmonth = this->posofdayinmonth();
    day++;
    if (day > dayinmonth) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }

    return tmp;
}

Date Date::operator -- (int) {
    Date tmp = *this;

    if (day == 1 && month == 1 && year == 1){
        cout << "Can't decrease this date!\nIt's not change!\n";
        return tmp;
    }
    day--;
    if (day <= 0) {
        month--;
        if (month <= 0){
            year--;
            month = 12;
        }
        int dayinmonth = this->posofdayinmonth();
        day = dayinmonth;
    }

    return tmp;
}

Date Date::operator ++ () {
    int dayinmonth = this->posofdayinmonth();
    day++;
    if (day > dayinmonth) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }

    return *this;
}

Date Date::operator -- () {
    if (day == 1 && month == 1 && year == 1){
        cout << "Can't decrease this date!\nIt's not change!\n";
        return *this;
    }
    day--;
    if (day <= 0) {
        month--;
        if (month <= 0) {
            year--;
            month = 12;
        }
        int dayinmonth = this->posofdayinmonth();
        day = dayinmonth;
    }

    return *this;
}

Date Date::operator + (const int &value) {
    Date tmp = *this;
    int valueVar = value + tmp.day - 1;
    tmp.day = 1;

    while (tmp.month > 1) {
        valueVar += tmp.posofdayinmonth(--tmp.month);
    }

    int tmpVar = tmp.isLeapYear() ? 366 : 365;
    while (valueVar >= tmpVar) {
        valueVar -= tmpVar;
        tmp.year++;

        tmpVar = tmp.isLeapYear() ? 366 : 365;
    }
    tmpVar = tmp.posofdayinmonth();
    while (valueVar >= tmpVar) {
        valueVar -= tmpVar;
        if (++tmp.month > 12) {
            tmp.month = 1;
            tmp.year++;
        }

        tmpVar = tmp.posofdayinmonth();

    }
    tmp.day += valueVar;

    return tmp;
}

Date Date::operator - (const int &value) {
    Date tmp = *this;

    int tmpVar = tmp.posofdayinmonth();
    int valueVar = value + (tmpVar - tmp.day);

    tmp.day = tmpVar;

    while (tmp.month < 12) {
        valueVar += tmp.posofdayinmonth(++tmp.month);
    }

    tmpVar = tmp.isLeapYear() ? 366 : 365;
    while (valueVar >= tmpVar) {
        if (tmp.year == 1) {
            tmp.day = 1;
            tmp.year = 1;
            return tmp;
        }
        tmp.year--;
        valueVar -= tmpVar;
        tmpVar = tmp.isLeapYear() ? 366 : 365;
    }

    tmpVar = tmp.posofdayinmonth();
    while (valueVar >= tmpVar) {
        tmp.month--;
        valueVar -= tmpVar;

        tmpVar = tmp.posofdayinmonth();
        tmp.day = tmpVar;
    }

    tmp.day -= valueVar;

    return tmp;
}

void Date::insert(int dd, int mm, int yyyy) {
    day = dd;
    month = mm;
    year = yyyy;
}

void Date::show() const {
    cout << "Date (dd/mm/yyyy): ";
    cout << setfill('0');
    cout << setw(2) << day << '/'
        << setw(2) << month << '/'
        << setw(4) << year;

    cout << endl;
}

int Date::getDay() const { return day; }
int Date::getMonth() const { return month; }
int Date::getYear() const { return year; }

void Date::setDay(int d) { day = d; }
void Date::setMonth(int m) { month = m; }
void Date::setYear(int y) { year = y; }

bool Date::isDefault() const {
    return (day == 0 && month == 0 && year == 0);
}

string Date::toString() const {
    ostringstream oss;
    oss << setfill('0') << setw(4) << year << "-"
        << setw(2) << month << "-"
        << setw(2) << day;
    return oss.str();
}

string Date::toDisplayString() const {
    ostringstream oss;
    oss << setfill('0') << setw(2) << day << "/"
        << setw(2) << month << "/"
        << setw(4) << year;
    return oss.str();
}

Date Date::fromString(const string &str) {
    if (str.empty()) return Date(0, 0, 0);
    int d = 0, m = 0, y = 0;
    char sep1 = '\0', sep2 = '\0';
    istringstream iss(str);
    if (str.find('-') != string::npos) {
        if (iss >> y >> sep1 >> m >> sep2 >> d && sep1 == '-' && sep2 == '-') {
            return Date(d, m, y);
        }
    } else if (str.find('/') != string::npos) {
        if (iss >> d >> sep1 >> m >> sep2 >> y && sep1 == '/' && sep2 == '/') {
            return Date(d, m, y);
        }
    }
    return Date(0, 0, 0);
}

Date Date::now() {
    time_t t = time(nullptr);
    tm *curr = localtime(&t);
    if (!curr) return Date(0, 0, 0);
    return Date(curr->tm_mday, 1 + curr->tm_mon, 1900 + curr->tm_year);
}

