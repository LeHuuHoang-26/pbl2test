#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <windows.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Appointment.h"
#include "Date.h"
#include "Doctor.h"
#include "LinkList.h"
#include "Manager.h"
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "libPBL2.h"

using namespace std;

static const string PATIENT_FILE = "Data/Patient.txt";
static const string MANAGER_FILE = "Data/Manager.txt";
static const string APPOINTMENT_FILE = "Data/Appointment/Appointment.txt";
static const string SYMPTOM_PATIENT_FILE = "Data/Symptom/SymptomPatient.txt";

void clearScreen() { system("cls"); }
void pauseScreen() { cout << "\n  Nhấn Enter để tiếp tục..."; string line; getline(cin, line); }
string readText(const string &label) {
    string value;
    cout << label;
    getline(cin, value);
    libPBL2::trim(value);
    return value;
}
int readInt(const string &label, int minValue, int maxValue) {
    while (true) {
        string input = readText(label);
        try {
            size_t used = 0;
            int value = stoi(input, &used);
            if (used == input.size() && value >= minValue && value <= maxValue) return value;
        } catch (...) {}
        cout << "  Vui lòng nhập số từ " << minValue << " đến " << maxValue << ".\n";
    }
}
void loading(const string &message) {
    cout << "\n  " << message << "  ";
    for (int i = 0; i < 3; ++i) cout << "● ";
    cout << "\n";
}
template<class T> int itemCount(const LinkList<T> *list) {
    int count = 0;
    if (!list) return count;
    auto *node = list->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault()) ++count; node = node->next; }
    return count;
}
Patient *findPatient(LinkList<Patient> *list, const string &id) {
    if (!list) return nullptr;
    auto *node = list->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault() && node->info.getID() == id) return &node->info; node = node->next; }
    return nullptr;
}
Doctor *findDoctor(LinkList<Doctor> *list, const string &id) {
    if (!list) return nullptr;
    auto *node = list->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault() && node->info.getID() == id) return &node->info; node = node->next; }
    return nullptr;
}
string specialtyName(const string &id) {
    ifstream file("Data/Doctor/Specialization.txt");
    string line;
    getline(file, line); getline(file, line);
    while (getline(file, line)) {
        string code, name;
        stringstream row(line);
        if (getline(row, code, '|') && getline(row, name)) {
            libPBL2::trim(code); libPBL2::trim(name);
            if (code == id) return name;
        }
    }
    return id;
}
string symptomsForPatient(LinkList<SymptomPatient> *records, LinkList<Symptom> *symptoms, const string &patientId) {
    string result;
    if (!records) return "Chưa ghi nhận";
    auto *record = records->getHeader()->next;
    while (record && record->next) {
        if (!record->info.isDefault() && record->info.getP_id() == patientId) {
            stringstream ids(record->info.getS_id()); string symptomId;
            while (getline(ids, symptomId, ',')) {
                libPBL2::trim(symptomId);
                if (symptoms) {
                    auto *symptom = symptoms->getHeader()->next;
                    while (symptom && symptom->next) {
                        if (symptom->info.getS_id() == symptomId) {
                            if (!result.empty()) result += ", ";
                            result += symptom->info.getSName(); break;
                        }
                        symptom = symptom->next;
                    }
                }
            }
        }
        record = record->next;
    }
    return result.empty() ? "Chưa ghi nhận" : result;
}
vector<Symptom> chooseSymptoms(LinkList<Symptom> *symptoms) {
    vector<Symptom> selected;
    if (!symptoms) return selected;
    cout << "\n  TRIỆU CHỨNG (nhập số thứ tự, 0 để hoàn tất)\n";
    int index = 0;
    auto *node = symptoms->getHeader()->next;
    while (node && node->next) {
        if (!node->info.isDefault()) cout << "  " << ++index << ". " << node->info.getSName() << " — " << specialtyName(node->info.getSp_id()) << "\n";
        node = node->next;
    }
    while (true) {
        int choice = readInt("  Chọn triệu chứng: ", 0, index);
        if (choice == 0) break;
        node = symptoms->getHeader()->next; int current = 0;
        while (node && node->next) {
            if (!node->info.isDefault() && ++current == choice) break;
            node = node->next;
        }
        if (node && node->next) {
            bool duplicate = false;
            for (const auto &s : selected) if (s.getS_id() == node->info.getS_id()) duplicate = true;
            if (!duplicate) { selected.push_back(node->info); cout << "  Đã thêm: " << node->info.getSName() << "\n"; }
            else cout << "  Triệu chứng này đã được chọn.\n";
        }
    }
    return selected;
}
string joinedSymptomIds(const vector<Symptom> &items) {
    string result;
    for (const auto &item : items) { if (!result.empty()) result += ", "; result += item.getS_id(); }
    return result;
}
string nextAppointmentId(LinkList<Appointment> *list) {
    int maximum = 0;
    if (list) { auto *node = list->getHeader()->next; while (node && node->next) {
        string id = node->info.getId(); if (id.rfind("A_", 0) == 0) try { maximum = max(maximum, stoi(id.substr(2))); } catch (...) {}
        node = node->next;
    }}
    ostringstream out; out << "A_" << setfill('0') << setw(3) << maximum + 1; return out.str();
}
void recordSymptoms(LinkList<SymptomPatient> *records, const string &patientId, const vector<Symptom> &selected) {
    if (selected.empty() || !records) return;
    records->insert(SymptomPatient(SymptomPatient::generateNextId(records), joinedSymptomIds(selected), patientId, Date::now()));
}
void addPatient(LinkList<Patient> *patients, LinkList<Symptom> *symptoms, LinkList<SymptomPatient> *records) {
    cout << "\n  ── HỒ SƠ BỆNH NHÂN MỚI ──\n";
    string name = readText("  Họ và tên: ");
    string phone = readText("  Số điện thoại: ");
    int gender = readInt("  Giới tính (1 Nam, 0 Nữ): ", 0, 1);
    int age = readInt("  Tuổi: ", 1, 130);
    string cccd = readText("  CCCD: ");
    if (name.empty() || phone.empty() || cccd.empty()) { cout << "  Các trường không được để trống.\n"; return; }
    auto *node = patients->getHeader()->next;
    while (node && node->next) {
        if (node->info.getPhone() == phone || node->info.getCCCD() == cccd) { cout << "  Số điện thoại hoặc CCCD đã tồn tại.\n"; return; }
        node = node->next;
    }
    Patient patient(Patient::generateNextId(patients), name, phone, gender == 1, age, cccd);
    patients->insert(patient);
    cout << "\n  Hồ sơ " << patient.getID() << " đã được tạo.\n";
    cout << "  Ghi nhận triệu chứng cho bệnh nhân này (0 để bỏ qua).\n";
    recordSymptoms(records, patient.getID(), chooseSymptoms(symptoms));
}
void listPatients(LinkList<Patient> *patients, LinkList<SymptomPatient> *records, LinkList<Symptom> *symptoms) {
    cout << "\n  DANH SÁCH BỆNH NHÂN  ·  " << itemCount(patients) << " hồ sơ\n";
    cout << "  " << left << setw(9) << "Mã BN" << setw(27) << "Họ tên" << setw(15) << "Điện thoại" << setw(7) << "Tuổi" << "Triệu chứng\n";
    auto *node = patients->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault()) cout << "  " << left << setw(9) << node->info.getID() << setw(27) << node->info.getfullName() << setw(15) << node->info.getPhone() << setw(7) << node->info.getAge() << symptomsForPatient(records, symptoms, node->info.getID()) << "\n"; node = node->next; }
}
void listDoctors(LinkList<Doctor> *doctors) {
    cout << "\n  ĐỘI NGŨ BÁC SĨ  ·  " << itemCount(doctors) << " hồ sơ\n";
    cout << "  " << left << setw(9) << "Mã BS" << setw(25) << "Họ tên" << setw(22) << "Chuyên ngành" << setw(12) << "Phòng" << "Trạng thái\n";
    auto *node = doctors->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault()) cout << "  " << left << setw(9) << node->info.getID() << setw(25) << node->info.getfullName() << setw(22) << specialtyName(node->info.getSpId()) << setw(12) << node->info.getRoomId() << (node->info.getState() ? "Đang làm việc" : "Tạm nghỉ") << "\n"; node = node->next; }
}
void bookAppointment(LinkList<Patient> *patients, LinkList<Doctor> *doctors, LinkList<Symptom> *symptoms, LinkList<SymptomPatient> *records, LinkList<Appointment> *appointments) {
    cout << "\n  ĐẶT LỊCH KHÁM\n";
    Patient *patient = nullptr;
    while (!patient) {
        string id = readText("  Mã bệnh nhân (Enter để tạo hồ sơ mới): ");
        if (id.empty()) { addPatient(patients, symptoms, records); if (patients->getFooter()->prev != patients->getHeader()) patient = &patients->getFooter()->prev->info; }
        else { patient = findPatient(patients, id); if (!patient) cout << "  Không tìm thấy mã bệnh nhân.\n"; }
    }
    cout << "  Bệnh nhân: " << patient->getfullName() << " (" << patient->getID() << ")\n";
    cout << "  Triệu chứng đã ghi: " << symptomsForPatient(records, symptoms, patient->getID()) << "\n";
    if (readInt("  Bổ sung triệu chứng? (1 Có, 0 Không): ", 0, 1)) recordSymptoms(records, patient->getID(), chooseSymptoms(symptoms));
    // Use the most relevant recorded symptom specialty to present matching doctors first.
    string preferredSpecialty;
    if (records) { auto *r = records->getHeader()->next; while (r && r->next) { if (r->info.getP_id() == patient->getID()) {
        stringstream ids(r->info.getS_id()); string sid;
        while (getline(ids, sid, ',')) { libPBL2::trim(sid); auto *s = symptoms->getHeader()->next; while (s && s->next) { if (s->info.getS_id() == sid && preferredSpecialty.empty()) preferredSpecialty = s->info.getSp_id(); s = s->next; } }
    } r = r->next; }}
    cout << "\n  BÁC SĨ ĐANG NHẬN LỊCH\n";
    int idx = 0; auto *doctor = doctors->getHeader()->next;
    while (doctor && doctor->next) { if (!doctor->info.isDefault() && doctor->info.getState()) {
        ++idx; bool match = preferredSpecialty.empty() || doctor->info.getSpId() == preferredSpecialty;
        cout << "  " << idx << ". " << doctor->info.getID() << " · " << doctor->info.getfullName() << " · " << specialtyName(doctor->info.getSpId()) << " · " << doctor->info.getRoomId() << (match ? "  ★ phù hợp" : "") << "\n";
    } doctor = doctor->next; }
    if (idx == 0) { cout << "  Hiện chưa có bác sĩ nhận lịch.\n"; return; }
    int choice = readInt("  Chọn bác sĩ: ", 1, idx); doctor = doctors->getHeader()->next; int current = 0;
    while (doctor && doctor->next) { if (!doctor->info.isDefault() && doctor->info.getState() && ++current == choice) break; doctor = doctor->next; }
    string date = readText("  Ngày giờ hẹn (YYYY-MM-DD HH:MM): ");
    Date appointmentDate = Date::loadFromString(date);
    if (appointmentDate.isDefault()) { cout << "  Ngày giờ không đúng định dạng.\n"; return; }
    Appointment appointment(nextAppointmentId(appointments), patient->getID(), doctor->info.getID(), Date::now(), appointmentDate, 0);
    appointments->insert(appointment);
    cout << "\n  Đã đặt lịch " << appointment.getId() << " với " << doctor->info.getfullName() << " · " << specialtyName(doctor->info.getSpId()) << " vào " << appointmentDate.toString() << ".\n";
}
void manageAppointments(LinkList<Appointment> *appointments, LinkList<Patient> *patients, LinkList<Doctor> *doctors, LinkList<SymptomPatient> *records, LinkList<Symptom> *symptoms) {
    cout << "\n  QUẢN LÝ LỊCH KHÁM\n";
    string doctorFilter = readText("  Lọc theo mã bác sĩ (Enter = tất cả): ");
    string dateFilter = readText("  Lọc theo ngày YYYY-MM-DD (Enter = tất cả): ");
    vector<Appointment *> shown;
    auto *node = appointments->getHeader()->next;
    while (node && node->next) {
        if (!node->info.isDefault()) {
            Doctor *doctor = findDoctor(doctors, node->info.getDoctorId());
            string date = node->info.getDateGet().toString();
            if ((doctorFilter.empty() || node->info.getDoctorId() == doctorFilter) && (dateFilter.empty() || date.rfind(dateFilter, 0) == 0)) {
                Patient *patient = findPatient(patients, node->info.getPatientId());
                shown.push_back(&node->info);
                cout << "\n  " << node->info.getId() << " · " << date << " · " << (doctor ? doctor->getfullName() : node->info.getDoctorId()) << " (" << node->info.getDoctorId() << ")\n";
                cout << "    Bệnh nhân: " << (patient ? patient->getfullName() : node->info.getPatientId()) << " (" << node->info.getPatientId() << ")\n";
                cout << "    Trạng thái: " << (node->info.getState() ? "ĐÃ KHÁM" : "CHƯA KHÁM") << "\n";
                cout << "    Triệu chứng: " << symptomsForPatient(records, symptoms, node->info.getPatientId()) << "\n";
            }
        }
        node = node->next;
    }
    if (shown.empty()) cout << "  Không có lịch phù hợp.\n";
    else { string id = readText("\n  Nhập mã lịch để đổi trạng thái khám (Enter bỏ qua): "); if (!id.empty()) {
        bool found = false; for (auto *appointment : shown) if (appointment->getId() == id) { appointment->setState(appointment->getState() ? 0 : 1); cout << "  Đã cập nhật: " << (appointment->getState() ? "ĐÃ KHÁM" : "CHƯA KHÁM") << "\n"; found = true; }
        if (!found) cout << "  Mã lịch không nằm trong kết quả đang lọc.\n";
    }}
}
string nextManagerId(LinkList<Manager> *managers) {
    int maximum = 0; auto *node = managers->getHeader()->next;
    while (node && node->next) { string id = node->info.getID(); if (id.rfind("MA_", 0) == 0) try { maximum = max(maximum, stoi(id.substr(3))); } catch (...) {} node = node->next; }
    ostringstream out; out << "MA_" << setfill('0') << setw(3) << maximum + 1; return out.str();
}
void listManagers(LinkList<Manager> *managers) {
    cout << "\n  DANH SÁCH QUẢN LÝ\n";
    cout << "  " << left << setw(9) << "Mã" << setw(25) << "Họ tên" << setw(18) << "Tài khoản" << setw(24) << "Vai trò" << "Trạng thái\n";
    auto *node = managers->getHeader()->next;
    while (node && node->next) { if (!node->info.isDefault()) cout << "  " << left << setw(9) << node->info.getID() << setw(25) << node->info.getfullName() << setw(18) << node->info.getUsername() << setw(24) << Manager::roleToString(node->info.getRole()) << (node->info.isActive() ? "Hoạt động" : "Đã khóa") << "\n"; node = node->next; }
}
void addManager(LinkList<Manager> *managers) {
    cout << "\n  THÊM TÀI KHOẢN QUẢN LÝ\n";
    string name = readText("  Họ tên: "), phone = readText("  Điện thoại: ");
    int gender = readInt("  Giới tính (1 Nam, 0 Nữ): ", 0, 1), age = readInt("  Tuổi: ", 18, 130);
    string username = readText("  Tên đăng nhập: "), password = readText("  Mật khẩu: ");
    if (name.empty() || phone.empty() || username.empty() || password.empty()) { cout << "  Vui lòng điền đầy đủ thông tin.\n"; return; }
    auto *node = managers->getHeader()->next; while (node && node->next) { if (node->info.getUsername() == username) { cout << "  Tên đăng nhập đã tồn tại.\n"; return; } node = node->next; }
    int role = readInt("  Vai trò (1 Lễ tân, 2 Lễ tân chuyên trách, 3 Quản trị): ", 1, 3);
    ManagerRole managerRole = role == 3 ? ManagerRole::Administrator : (role == 2 ? ManagerRole::SpecialReceptionist : ManagerRole::Receptionist);
    managers->insert(Manager(nextManagerId(managers), name, phone, gender == 1, age, username, password, managerRole, 1));
    cout << "  Đã tạo tài khoản quản lý.\n";
}
void saveAll(LinkList<Patient> *patients, LinkList<Manager> *managers, LinkList<SymptomPatient> *records, LinkList<Appointment> *appointments) {
    bool ok = Patient::saveToFile(PATIENT_FILE, patients) && Manager::saveToFile(MANAGER_FILE, managers) && SymptomPatient::saveToFile(SYMPTOM_PATIENT_FILE, records) && Appointment::saveToFile(APPOINTMENT_FILE, appointments);
    cout << (ok ? "  ✓ Dữ liệu đã lưu.\n" : "  Có lỗi khi lưu dữ liệu.\n");
}

int main() {
    SetConsoleOutputCP(65001); SetConsoleCP(65001);
    loading("Đang khởi tạo Phòng khám An Tâm");
    auto *managers = Manager::loadFromFile(MANAGER_FILE);
    auto *patients = Patient::loadFromFile(PATIENT_FILE);
    auto *doctors = Doctor::loadFromFile("Data/Doctor/Doctor.txt");
    auto *symptoms = Symptom::loadFromFile("Data/Symptom/Symptom.txt");
    auto *records = SymptomPatient::loadFromFile(SYMPTOM_PATIENT_FILE);
    auto *appointments = Appointment::loadFromFile(APPOINTMENT_FILE);
    if (!managers || !patients || !doctors || !symptoms || !records || !appointments) {
        cerr << "Không thể tải dữ liệu. Hãy chạy chương trình từ thư mục gốc dự án.\n";
        delete managers; delete patients; delete doctors; delete symptoms; delete records; delete appointments; return 1;
    }
    bool exitProgram = false;
    while (!exitProgram) {
        clearScreen();
        cout << "\n  ╭──────────────────────────────────────────────╮\n"
             << "  │              PHÒNG KHÁM AN TÂM               │\n"
             << "  │       Chăm sóc tận tâm · Sức khỏe vẹn toàn   │\n"
             << "  ╰──────────────────────────────────────────────╯\n\n"
             << "  ĐĂNG NHẬP HỆ THỐNG\n";
        string username = readText("  Tài khoản: ");
        if (username.empty() || username == "0") { exitProgram = true; break; }
        string password = readText("  Mật khẩu: ");
        Manager *current = nullptr; auto *node = managers->getHeader()->next;
        while (node && node->next) { if (!node->info.isDefault() && node->info.isActive() && node->info.isValid(username, password)) { current = &node->info; break; } node = node->next; }
        if (!current) { cout << "\n  Sai tài khoản/mật khẩu hoặc tài khoản đã khóa.\n"; pauseScreen(); continue; }
        loading("Đang đăng nhập");
        bool logout = false;
        while (!logout) {
            clearScreen();
            cout << "\n  Xin chào, " << current->getfullName() << "  ·  " << current->getID() << "\n"
                 << "  ──────────────────────────────────────────────\n"
                 << "  [1] Đặt lịch khám\n"
                 << "  [2] Thêm bệnh nhân\n"
                 << "  [3] Danh sách bác sĩ\n"
                 << "  [4] Danh sách bệnh nhân\n"
                 << "  [5] Quản lý lịch khám\n";
            bool superAdmin = current->getID() == "MA_001" || current->getID() == "001";
            if (superAdmin) cout << "  [6] Danh sách quản lý\n  [7] Thêm quản lý\n";
            cout << "  [8] Lưu dữ liệu\n  [0] Đăng xuất\n  ──────────────────────────────────────────────\n";
            int choice = readInt("  Chọn chức năng: ", 0, superAdmin ? 8 : 8);
            switch (choice) {
                case 1: bookAppointment(patients, doctors, symptoms, records, appointments); pauseScreen(); break;
                case 2: addPatient(patients, symptoms, records); pauseScreen(); break;
                case 3: listDoctors(doctors); pauseScreen(); break;
                case 4: listPatients(patients, records, symptoms); pauseScreen(); break;
                case 5: manageAppointments(appointments, patients, doctors, records, symptoms); pauseScreen(); break;
                case 6: if (superAdmin) { listManagers(managers); pauseScreen(); } else cout << "  Chức năng không khả dụng.\n"; break;
                case 7: if (superAdmin) { addManager(managers); pauseScreen(); } else cout << "  Chức năng không khả dụng.\n"; break;
                case 8: saveAll(patients, managers, records, appointments); pauseScreen(); break;
                case 0: saveAll(patients, managers, records, appointments); loading("Đang đăng xuất · Cảm ơn bạn đã đồng hành"); cout << "\n  Chúc bạn một ngày tốt lành, hẹn gặp lại!\n"; logout = true; pauseScreen(); break;
            }
        }
    }
    loading("Đang đóng ứng dụng · Hẹn gặp lại");
    delete managers; delete patients; delete doctors; delete symptoms; delete records; delete appointments;
    cout << "\n  Cảm ơn bạn đã sử dụng Phòng khám An Tâm.\n";
    return 0;
}
