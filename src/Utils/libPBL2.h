#ifndef LIBPBL2_H
#define LIBPBL2_H

#include <string>
#include "../DataStructures/LinkList.h"
#include "../Models/Permission.h"
#include "Date.h"

class Patient;
class Doctor;
class Appointment;
class Manager;

namespace libPBL2 {

    using Permission = ::Permission;
    using PermissionType = ::PermissionType;

    // String utilities
    void trim(std::string &str);
    void filterNewline(std::string &str);
    size_t utf8Length(const std::string &str);
    std::string padRightUtf8(const std::string &str, size_t targetWidth);

    // Permission checking
    bool isAllowed(const Manager &M, PermissionType Per, const LinkList<Permission> *PermissionList = nullptr);

    // -------------------------------------------------------------
    // Generic QuickSort Algorithm
    // -------------------------------------------------------------

    template <typename T>
    inline void swapVal(T &a, T &b) {
        T tmp = static_cast<T&&>(a);
        a = static_cast<T&&>(b);
        b = static_cast<T&&>(tmp);
    }

    // Partition logic for doubly linked list
    template <typename T, typename Compare>
    typename LinkList<T>::Node* partitionNodes(typename LinkList<T>::Node *left,
                                                typename LinkList<T>::Node *right,
                                                Compare comp) {
        T pivot = right->info;
        typename LinkList<T>::Node *i = left->prev;

        for (typename LinkList<T>::Node *j = left; j != right; j = j->next) {
            if (comp(j->info, pivot)) {
                i = (i == left->prev) ? left : i->next;
                if (i != j) {
                    swapVal(i->info, j->info);
                }
            }
        }
        i = (i == left->prev) ? left : i->next;
        if (i != right) {
            swapVal(i->info, right->info);
        }
        return i;
    }

    // Recursive node quicksort
    template <typename T, typename Compare>
    void qsortNodes(typename LinkList<T>::Node *left, typename LinkList<T>::Node *right, Compare comp) {
        if (!left || !right || left == right || right->next == left) {
            return;
        }

        typename LinkList<T>::Node *pivot = partitionNodes<T, Compare>(left, right, comp);

        if (pivot != left && pivot->prev) {
            qsortNodes<T, Compare>(left, pivot->prev, comp);
        }
        if (pivot != right && pivot->next) {
            qsortNodes<T, Compare>(pivot->next, right, comp);
        }
    }

    // QuickSort on LinkList<T>
    template <typename T, typename Compare>
    void qsort(LinkList<T> &list, Compare comp) {
        if (list.getCount() <= 1 || !list.getHeader() || !list.getFooter()) return;
        typename LinkList<T>::Node *first = list.getHeader()->next;
        typename LinkList<T>::Node *last = list.getFooter()->prev;
        if (!first || !last || first == list.getFooter() || first == last) return;
        qsortNodes<T, Compare>(first, last, comp);
    }

    template <typename T, typename Compare>
    void qsort(LinkList<T> *list, Compare comp) {
        if (list) {
            qsort<T, Compare>(*list, comp);
        }
    }

    // QuickSort on C-arrays
    template <typename T, typename Compare>
    void qsort(T *arr, int low, int high, Compare comp) {
        if (!arr || low >= high) return;
        T pivot = arr[high];
        int i = low - 1;
        for (int j = low; j < high; ++j) {
            if (comp(arr[j], pivot)) {
                i++;
                swapVal(arr[i], arr[j]);
            }
        }
        swapVal(arr[i + 1], arr[high]);
        int pi = i + 1;
        qsort(arr, low, pi - 1, comp);
        qsort(arr, pi + 1, high, comp);
    }

    // QuickSort on random-access indexable containers (QList, std::vector, etc.)
    template <typename Container, typename Compare>
    void qsortRandomAccess(Container &c, int low, int high, Compare comp) {
        if (low >= high) return;
        auto pivot = c[high];
        int i = low - 1;
        for (int j = low; j < high; ++j) {
            if (comp(c[j], pivot)) {
                i++;
                auto tmp = c[i];
                c[i] = c[j];
                c[j] = tmp;
            }
        }
        auto tmp = c[i + 1];
        c[i + 1] = c[high];
        c[high] = tmp;
        int pi = i + 1;
        qsortRandomAccess(c, low, pi - 1, comp);
        qsortRandomAccess(c, pi + 1, high, comp);
    }

    template <typename Container, typename Compare>
    void qsort(Container &c, Compare comp) {
        if (c.size() <= 1) return;
        qsortRandomAccess(c, 0, static_cast<int>(c.size()) - 1, comp);
    }

    // Comparison helpers
    int compareNatural(const std::string &a, const std::string &b);
    std::string extractLastName(const std::string &fullName);
    int compareVietnameseNames(const std::string &name1, const std::string &name2);
    int compareDates(const Date &da, const Date &db);

    // -------------------------------------------------------------
    // Patient Sorting
    // -------------------------------------------------------------
    enum class PatientSortBy {
        ID,
        Name,
        Age,
        CCCD,
        Phone,
        Gender
    };
    void sortPatients(LinkList<Patient> &list, PatientSortBy criteria = PatientSortBy::Name, bool ascending = true);
    void sortPatients(LinkList<Patient> *list, PatientSortBy criteria = PatientSortBy::Name, bool ascending = true);

    // -------------------------------------------------------------
    // Doctor Sorting
    // -------------------------------------------------------------
    enum class DoctorSortBy {
        ID,
        Name,
        Rating,
        Fee,
        Specialty,
        Room,
        State
    };
    void sortDoctors(LinkList<Doctor> &list, DoctorSortBy criteria = DoctorSortBy::Name, bool ascending = true);
    void sortDoctors(LinkList<Doctor> *list, DoctorSortBy criteria = DoctorSortBy::Name, bool ascending = true);

    // -------------------------------------------------------------
    // Appointment Sorting
    // -------------------------------------------------------------
    enum class AppointmentSortBy {
        Date,
        ID,
        State,
        DoctorID
    };
    void sortAppointments(LinkList<Appointment> &list, AppointmentSortBy criteria = AppointmentSortBy::Date, bool ascending = true);
    void sortAppointments(LinkList<Appointment> *list, AppointmentSortBy criteria = AppointmentSortBy::Date, bool ascending = true);

    // -------------------------------------------------------------
    // Account (Manager) Sorting
    // -------------------------------------------------------------
    enum class AccountSortBy {
        ID,
        Username,
        Name,
        Role,
        State
    };
    void sortAccounts(LinkList<Manager> &list, AccountSortBy criteria = AccountSortBy::ID, bool ascending = true);
    void sortAccounts(LinkList<Manager> *list, AccountSortBy criteria = AccountSortBy::ID, bool ascending = true);
    void sortManagers(LinkList<Manager> &list, AccountSortBy criteria = AccountSortBy::ID, bool ascending = true);
    void sortManagers(LinkList<Manager> *list, AccountSortBy criteria = AccountSortBy::ID, bool ascending = true);

} // namespace libPBL2

#endif // LIBPBL2_H
