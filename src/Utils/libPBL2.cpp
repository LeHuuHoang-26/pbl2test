#include "libPBL2.h"
#include "../Models/Manager.h"
#include "../Models/Patient.h"
#include "../Models/Doctor.h"
#include "../Models/Appointment.h"
#include "Date.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <QString>
#include <QCollator>
#include <QLocale>

using namespace std;

namespace libPBL2 {

void trim(string &str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    str = str.substr(start, end - start + 1);
}

void filterNewline(string &str) {
    string res = "";
    for (char c : str) {
        if (c != '\r' && c != '\n') {
            res += c;
        }
    }
    str = res;
}

size_t utf8Length(const string &str) {
    size_t len = 0;
    for (unsigned char c : str) {
        if ((c & 0xC0) != 0x80) {
            len++;
        }
    }
    return len;
}

string padRightUtf8(const string &str, size_t targetWidth) {
    size_t ulen = utf8Length(str);
    if (ulen >= targetWidth) return str;
    return str + string(targetWidth - ulen, ' ');
}

bool isAllowed(const Manager &M, PermissionType Per, const LinkList<Permission> *PermissionList) {
    if (!M.isActive())
        return false;

    if (Per == PermissionType::Empty)
        return true;

    if (M.getRole() == ManagerRole::Administrator)
        return true;

    if (PermissionList == nullptr || PermissionList->getHeader() == nullptr)
        return false;

    LinkList<Permission>::Node *tmp = PermissionList->getHeader()->next;
    while (tmp != nullptr && tmp->next != nullptr) {
        if (M.getID() == tmp->info.MA_id && tmp->info.Per_id == Per)
            return true;
        tmp = tmp->next;
    }

    return false;
}

// -------------------------------------------------------------
// Comparison Helpers
// -------------------------------------------------------------

int compareNatural(const string &a, const string &b) {
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (isdigit(static_cast<unsigned char>(a[i])) && isdigit(static_cast<unsigned char>(b[j]))) {
            long long numA = 0;
            while (i < a.size() && isdigit(static_cast<unsigned char>(a[i]))) {
                numA = numA * 10 + (a[i] - '0');
                i++;
            }
            long long numB = 0;
            while (j < b.size() && isdigit(static_cast<unsigned char>(b[j]))) {
                numB = numB * 10 + (b[j] - '0');
                j++;
            }
            if (numA != numB) return (numA < numB) ? -1 : 1;
        } else {
            char ca = tolower(static_cast<unsigned char>(a[i]));
            char cb = tolower(static_cast<unsigned char>(b[j]));
            if (ca != cb) return (ca < cb) ? -1 : 1;
            i++;
            j++;
        }
    }
    if (a.size() != b.size()) return (a.size() < b.size()) ? -1 : 1;
    return 0;
}

string extractLastName(const string &fullName) {
    size_t end = fullName.find_last_not_of(" \t\r\n");
    if (end == string::npos) return "";
    size_t start = fullName.find_last_of(" \t", end);
    if (start == string::npos) return fullName.substr(0, end + 1);
    return fullName.substr(start + 1, end - start);
}

int compareVietnameseNames(const string &name1, const string &name2) {
    string last1 = extractLastName(name1);
    string last2 = extractLastName(name2);

    static const QCollator collator = []() {
        QCollator c(QLocale(QLocale::Vietnamese));
        c.setCaseSensitivity(Qt::CaseInsensitive);
        return c;
    }();

    int cmp = collator.compare(QString::fromUtf8(last1.c_str()), QString::fromUtf8(last2.c_str()));
    if (cmp != 0) {
        return cmp < 0 ? -1 : 1;
    }

    cmp = collator.compare(QString::fromUtf8(name1.c_str()), QString::fromUtf8(name2.c_str()));
    if (cmp != 0) {
        return cmp < 0 ? -1 : 1;
    }
    return 0;
}

int compareDates(const Date &da, const Date &db) {
    if (da.getYear() != db.getYear()) return (da.getYear() < db.getYear()) ? -1 : 1;
    if (da.getMonth() != db.getMonth()) return (da.getMonth() < db.getMonth()) ? -1 : 1;
    if (da.getDay() != db.getDay()) return (da.getDay() < db.getDay()) ? -1 : 1;
    if (da.getHour() != db.getHour()) return (da.getHour() < db.getHour()) ? -1 : 1;
    if (da.getMinute() != db.getMinute()) return (da.getMinute() < db.getMinute()) ? -1 : 1;
    return 0;
}

// -------------------------------------------------------------
// Model Sorting Implementations
// -------------------------------------------------------------

void sortPatients(LinkList<Patient> &list, PatientSortBy criteria, bool ascending) {
    qsort(list, [criteria, ascending](const Patient &a, const Patient &b) {
        int cmp = 0;
        switch (criteria) {
            case PatientSortBy::ID:
                cmp = compareNatural(a.getID(), b.getID());
                break;
            case PatientSortBy::Name:
                cmp = compareVietnameseNames(a.getfullName(), b.getfullName());
                break;
            case PatientSortBy::Age:
                if (a.getAge() != b.getAge()) cmp = (a.getAge() < b.getAge()) ? -1 : 1;
                break;
            case PatientSortBy::CCCD:
                cmp = compareNatural(a.getCCCD(), b.getCCCD());
                break;
            case PatientSortBy::Phone:
                cmp = compareNatural(a.getPhone(), b.getPhone());
                break;
            case PatientSortBy::Gender:
                if (a.getGender() != b.getGender()) cmp = a.getGender() ? -1 : 1;
                break;
        }
        if (cmp == 0) cmp = compareNatural(a.getID(), b.getID());
        return ascending ? (cmp < 0) : (cmp > 0);
    });
}

void sortPatients(LinkList<Patient> *list, PatientSortBy criteria, bool ascending) {
    if (list) sortPatients(*list, criteria, ascending);
}

void sortDoctors(LinkList<Doctor> &list, DoctorSortBy criteria, bool ascending) {
    qsort(list, [criteria, ascending](const Doctor &a, const Doctor &b) {
        int cmp = 0;
        switch (criteria) {
            case DoctorSortBy::ID:
                cmp = compareNatural(a.getID(), b.getID());
                break;
            case DoctorSortBy::Name:
                cmp = compareVietnameseNames(a.getfullName(), b.getfullName());
                break;
            case DoctorSortBy::Rating:
                if (a.getRating() != b.getRating()) cmp = (a.getRating() < b.getRating()) ? -1 : 1;
                break;
            case DoctorSortBy::Fee:
                if (a.getFee() != b.getFee()) cmp = (a.getFee() < b.getFee()) ? -1 : 1;
                break;
            case DoctorSortBy::Specialty:
                cmp = compareNatural(a.getSpId(), b.getSpId());
                break;
            case DoctorSortBy::Room:
                cmp = compareNatural(a.getRoomId(), b.getRoomId());
                break;
            case DoctorSortBy::State:
                if (a.getState() != b.getState()) cmp = a.getState() ? -1 : 1;
                break;
        }
        if (cmp == 0) cmp = compareNatural(a.getID(), b.getID());
        return ascending ? (cmp < 0) : (cmp > 0);
    });
}

void sortDoctors(LinkList<Doctor> *list, DoctorSortBy criteria, bool ascending) {
    if (list) sortDoctors(*list, criteria, ascending);
}

void sortAppointments(LinkList<Appointment> &list, AppointmentSortBy criteria, bool ascending) {
    qsort(list, [criteria, ascending](const Appointment &a, const Appointment &b) {
        int cmp = 0;
        switch (criteria) {
            case AppointmentSortBy::Date:
                cmp = compareDates(a.getDateGet(), b.getDateGet());
                break;
            case AppointmentSortBy::ID:
                cmp = compareNatural(a.getId(), b.getId());
                break;
            case AppointmentSortBy::State:
                if (a.getState() != b.getState()) cmp = (a.getState() < b.getState()) ? -1 : 1;
                break;
            case AppointmentSortBy::DoctorID:
                cmp = compareNatural(a.getDoctorId(), b.getDoctorId());
                break;
        }
        if (cmp == 0) cmp = compareDates(a.getDateGet(), b.getDateGet());
        return ascending ? (cmp < 0) : (cmp > 0);
    });
}

void sortAppointments(LinkList<Appointment> *list, AppointmentSortBy criteria, bool ascending) {
    if (list) sortAppointments(*list, criteria, ascending);
}

void sortAccounts(LinkList<Manager> &list, AccountSortBy criteria, bool ascending) {
    qsort(list, [criteria, ascending](const Manager &a, const Manager &b) {
        int cmp = 0;
        switch (criteria) {
            case AccountSortBy::ID:
                cmp = compareNatural(a.getID(), b.getID());
                break;
            case AccountSortBy::Username:
                cmp = compareNatural(a.getUsername(), b.getUsername());
                break;
            case AccountSortBy::Name:
                cmp = compareVietnameseNames(a.getfullName(), b.getfullName());
                break;
            case AccountSortBy::Role:
                if (static_cast<int>(a.getRole()) != static_cast<int>(b.getRole()))
                    cmp = (static_cast<int>(a.getRole()) < static_cast<int>(b.getRole())) ? -1 : 1;
                break;
            case AccountSortBy::State:
                if (a.getState() != b.getState()) cmp = (a.getState() < b.getState()) ? -1 : 1;
                break;
        }
        if (cmp == 0) cmp = compareNatural(a.getID(), b.getID());
        return ascending ? (cmp < 0) : (cmp > 0);
    });
}

void sortAccounts(LinkList<Manager> *list, AccountSortBy criteria, bool ascending) {
    if (list) sortAccounts(*list, criteria, ascending);
}

void sortManagers(LinkList<Manager> &list, AccountSortBy criteria, bool ascending) {
    sortAccounts(list, criteria, ascending);
}

void sortManagers(LinkList<Manager> *list, AccountSortBy criteria, bool ascending) {
    if (list) sortAccounts(*list, criteria, ascending);
}

} // namespace libPBL2
