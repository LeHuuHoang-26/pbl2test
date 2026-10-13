#ifndef LIBPBL2_H
#define LIBPBL2_H

#include <string>
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "Manager.h"
#include "Doctor.h"
#include "LinkList.h"

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

    // Truyền PermissionType theo giá trị (pass-by-value)
    bool isAllowed(const Manager &M, PermissionType Per, const LinkList<Permission> *PermissionList = nullptr);

    template <typename T>
    LinkList<T>* LoadFromFile(const std::string &path);

    template <typename T>
    bool SaveToFile(const std::string &path, const LinkList<T> *list);

    template <typename T>
    T getFromStr(const std::string &str);

    template <typename T>
    std::string toStr(const T &val);

    template <typename T>
    std::string generateNext_id(const LinkList<T> *list);

    // --- Template Specialization Declarations ---
    template <>
    LinkList<Manager>* LoadFromFile<Manager>(const std::string &path);

    template <>
    LinkList<Permission>* LoadFromFile<Permission>(const std::string &path);

    template <>
    LinkList<Patient>* LoadFromFile<Patient>(const std::string &path);

    template <>
    LinkList<Symptom>* LoadFromFile<Symptom>(const std::string &path);

    template <>
    LinkList<SymptomPatient>* LoadFromFile<SymptomPatient>(const std::string &path);

    template <>
    LinkList<Doctor>* LoadFromFile<Doctor>(const std::string &path);

    // Giữ nguyên chỉ riêng SaveToFile cho SymptomPatient
    template <>
    bool SaveToFile<SymptomPatient>(const std::string &path, const LinkList<SymptomPatient> *list);

    template <>
    PermissionType getFromStr<PermissionType>(const std::string &str);

    template <>
    std::string toStr<PermissionType>(const PermissionType &PerT);

    template <>
    std::string toStr<ManagerRole>(const ManagerRole &MA_Role);

    template <>
    std::string generateNext_id<SymptomPatient>(const LinkList<SymptomPatient> *list);

} // namespace libPBL2

#endif // LIBPBL2_H