#ifndef LIBPBL2_H
#define LIBPBL2_H

#include <string>
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "Manager.h"
#include "Doctor.h"
#include "QueuePatient.h"
#include "LinkList.h"
#include "Queue.h"

namespace libPBL2 {

    enum class PermissionType {
        Empty = 0,
        Per_01,
        Per_02,
        Per_03,
        Per_04,
        Per_05,
        Per_06,
        Per_07,
        Per_08,
        Per_00
    };

    struct Permission {
        std::string MA_id;
        PermissionType Per_id;
    };

    void trim(std::string &str);
    void filterNewline(std::string &str);
    size_t utf8Length(const std::string &str);
    std::string padRightUtf8(const std::string &str, size_t targetWidth);

    bool isAllowed(const Manager &M, PermissionType Per, const LinkList<Permission> *PermissionList = nullptr);

    template <typename T, template <typename> class U = LinkList>
    U<T>* LoadFromFile(const std::string &path);

    template <typename T, template <typename> class U = LinkList>
    bool SaveToFile(const std::string &path, const U<T> *container);

    template <typename T>
    T getFromStr(const std::string &str);

    template <typename T>
    std::string toStr(const T &val);

    template <typename T>
    std::string generateNext_id(const LinkList<T> *list);

    template <>
    LinkList<Manager>* LoadFromFile<Manager, LinkList>(const std::string &path);

    template <>
    LinkList<Permission>* LoadFromFile<Permission, LinkList>(const std::string &path);

    template <>
    LinkList<Patient>* LoadFromFile<Patient, LinkList>(const std::string &path);

    template <>
    LinkList<Symptom>* LoadFromFile<Symptom, LinkList>(const std::string &path);

    template <>
    LinkList<SymptomPatient>* LoadFromFile<SymptomPatient, LinkList>(const std::string &path);

    template <>
    LinkList<Doctor>* LoadFromFile<Doctor, LinkList>(const std::string &path);

    template <>
    Queue<QueuePatient>* LoadFromFile<QueuePatient, Queue>(const std::string &path);

    template <>
    bool SaveToFile<SymptomPatient, LinkList>(const std::string &path, const LinkList<SymptomPatient> *list);

    template <>
    bool SaveToFile<Patient, LinkList>(const std::string &path, const LinkList<Patient> *list);

    template <>
    bool SaveToFile<QueuePatient, Queue>(const std::string &path, const Queue<QueuePatient> *queue);

    template <>
    PermissionType getFromStr<PermissionType>(const std::string &str);

    template <>
    std::string toStr<PermissionType>(const PermissionType &PerT);

    template <>
    std::string toStr<ManagerRole>(const ManagerRole &MA_Role);

    template <>
    std::string generateNext_id<SymptomPatient>(const LinkList<SymptomPatient> *list);

    template <>
    std::string generateNext_id<Patient>(const LinkList<Patient> *list);

}

#endif
