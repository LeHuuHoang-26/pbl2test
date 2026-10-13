#ifndef DATE_H
#define DATE_H

#include <iostream>
#include <string>

class Date {
    private:
        int day;
        int month;
        int year;
    public:
        Date(int day = 0, int month = 0, int year = 0);
        Date(const Date &D);
        Date &operator=(const Date &D) = default;
        ~Date();

        int getDay() const;
        int getMonth() const;
        int getYear() const;
        void setDay(int d);
        void setMonth(int m);
        void setYear(int y);
        bool isDefault() const;

        std::string toString() const;
        std::string toDisplayString() const;
        static Date fromString(const std::string &str);
        static Date now();

        void insert(int dd, int mm, int yyyy);
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
