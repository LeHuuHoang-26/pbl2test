#ifndef DATE_H
#define DATE_H

#include <iostream>
#include <string>

class Date {
    private:
        int day;
        int month;
        int year;
        int hour;
        int minute;

    public:
        Date(int day = 0, int month = 0, int year = 0, int hour = 0, int minute = 0);
        Date(const Date &D);
        Date &operator=(const Date &D) = default;
        ~Date();

        int getDay() const;
        int getMonth() const;
        int getYear() const;
        int getHour() const;
        int getMinute() const;

        void setDay(int d);
        void setMonth(int m);
        void setYear(int y);
        void setHour(int h);
        void setMinute(int min);

        bool isDefault() const;

        std::string toString() const;
        static Date loadFromString(const std::string &str);
        static Date now();

        void insert(int dd, int mm, int yyyy, int hh = 0, int min = 0);
        void show() const;
        friend std::istream &operator >> (std::istream &in, Date &D);
        friend std::ostream &operator << (std::ostream &out, const Date &D);
        Date operator ++ (int);
        Date operator -- (int);
        Date operator ++ ();
        Date operator -- ();
        Date operator + (const int &value);
        Date operator - (const int &value);
        int posofdayinmonth(int = 0) const;
        bool isLeapYear(int = 0) const;
};

#endif
