#include "Date.h"
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace std;

Date::Date(int day, int month, int year, int hour, int minute)
    : day(day), month(month), year(year), hour(hour), minute(minute) {}

Date::Date(const Date &D)
    : day(D.day), month(D.month), year(D.year), hour(D.hour), minute(D.minute) {}

Date::~Date() {}

int Date::getDay() const { return day; }
int Date::getMonth() const { return month; }
int Date::getYear() const { return year; }
int Date::getHour() const { return hour; }
int Date::getMinute() const { return minute; }

void Date::setDay(int d) { day = d; }
void Date::setMonth(int m) { month = m; }
void Date::setYear(int y) { year = y; }
void Date::setHour(int h) { hour = h; }
void Date::setMinute(int min) { minute = min; }

istream &operator >> (istream &in, Date &D) {
    cout << "Date and time (dd/mm/yyyy hh:mm): ";
    char c1, c2, c3;
    int flag;
    do {
        flag = 0;
        in >> D.day >> c1 >> D.month >> c2 >> D.year;
        if (in.peek() == ' ' || in.peek() == '\t') {
            in >> D.hour >> c3 >> D.minute;
        } else {
            D.hour = 0;
            D.minute = 0;
        }

        if (D.year < 1 || D.month < 1 || D.month > 12 || D.hour < 0 || D.hour >= 24 || D.minute < 0 || D.minute >= 60) {
            flag = 1;
        } else {
            int dayinmonth = D.posofdayinmonth();
            if (D.day < 1 || D.day > dayinmonth)
                flag = 1;
        }

        if (flag)
            cout << "Nhap lai (dd/mm/yyyy hh:mm): ";
    } while (flag);

    return in;
}

ostream &operator << (ostream &out, const Date &D) {
    out << D.toString();
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
    if (day == 1 && month == 1 && year == 1) {
        cout << "Can't decrease this date!\nIt's not change!\n";
        return tmp;
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
    if (day == 1 && month == 1 && year == 1) {
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

void Date::insert(int dd, int mm, int yyyy, int hh, int min) {
    day = dd;
    month = mm;
    year = yyyy;
    hour = hh;
    minute = min;
}

void Date::show() const {
    cout << "Date and time: " << toString() << "\n";
}

bool Date::isDefault() const {
    return (day == 0 && month == 0 && year == 0);
}

string Date::toString() const {
    ostringstream oss;
    oss << setfill('0')
        << setw(4) << year << "-"
        << setw(2) << month << "-"
        << setw(2) << day << " "
        << setw(2) << hour << ":"
        << setw(2) << minute;
    return oss.str();
}

Date Date::loadFromString(const string &str) {
    if (str.empty()) return Date(0, 0, 0, 0, 0);
    int d = 0, m = 0, y = 0, hh = 0, mm = 0;
    char sep1 = '\0', sep2 = '\0', timeSep = '\0';
    istringstream iss(str);
    if (str.find('-') != string::npos) {
        if (iss >> y >> sep1 >> m >> sep2 >> d && sep1 == '-' && sep2 == '-') {
            if (iss >> hh >> timeSep >> mm && timeSep == ':') {
                return Date(d, m, y, hh, mm);
            }
            return Date(d, m, y, 0, 0);
        }
    } else if (str.find('/') != string::npos) {
        if (iss >> d >> sep1 >> m >> sep2 >> y && sep1 == '/' && sep2 == '/') {
            if (iss >> hh >> timeSep >> mm && timeSep == ':') {
                return Date(d, m, y, hh, mm);
            }
            return Date(d, m, y, 0, 0);
        }
    }
    return Date(0, 0, 0, 0, 0);
}

Date Date::now() {
    time_t t = time(nullptr);
    tm *curr = localtime(&t);
    if (!curr) return Date(0, 0, 0, 0, 0);
    return Date(curr->tm_mday, curr->tm_mon + 1, curr->tm_year + 1900, curr->tm_hour, curr->tm_min);
}
