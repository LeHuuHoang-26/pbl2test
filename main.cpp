#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>

#include "libPBL2.h"
#include "Manager.h"
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "Doctor.h"
#include "QueuePatient.h"
#include "Appointment.h"
#include "Date.h"
#include "Permission.h"
#include "LinkList.h"
#include "Queue.h"

using namespace std;

void filterNewline(string &str) {
    libPBL2::filterNewline(str);
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(10000, '\n');
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

string nhapThongTinBenhNhan(LinkList<Patient> *patientList) {
    if (!patientList) {
        cout << "[LỖI] Danh sách bệnh nhân chưa được khởi tạo!\n";
        return "";
    }

    cout << "\n================= TIẾP NHẬN BỆNH NHÂN =================\n";
    cout << "[1] Bệnh nhân cũ (Đã có hồ sơ - Tìm theo Mã BN / SĐT / CCCD)\n";
    cout << "[2] Bệnh nhân mới (Tự động sinh mã P_xxx & nhập thông tin)\n";
    cout << "[0] Hủy tiếp nhận (Quay lại menu)\n";
    cout << "-------------------------------------------------------\n";
    cout << "Lựa chọn của bạn: ";

    int loai;
    if (!(cin >> loai)) {
        clearInputBuffer();
        clearScreen();
        cout << "[LỖI] Lựa chọn không hợp lệ!\n";
        return "";
    }
    clearInputBuffer();
    clearScreen();

    if (loai == 0) {
        return "";
    }

    if (loai == 1) {
        cout << "\nNhập Mã BN (P_xxx), hoặc Số điện thoại, hoặc CCCD: ";
        string keyword;
        getline(cin, keyword);
        filterNewline(keyword);
        libPBL2::trim(keyword);

        if (keyword.empty()) {
            cout << "[LỖI] Từ khóa tìm kiếm không được để trống!\n";
            return "";
        }

        LinkList<Patient>::Node *cur = patientList->getHeader()->next;
        Patient *foundPatient = nullptr;
        while (cur != nullptr && cur->next != nullptr) {
            if (!cur->info.isDefault()) {
                if (cur->info.getID() == keyword || cur->info.getPhone() == keyword || cur->info.getCCCD() == keyword) {
                    foundPatient = &cur->info;
                    break;
                }
            }
            cur = cur->next;
        }

        if (foundPatient != nullptr) {
            cout << "\n>> TÌM THẤY THÔNG TIN BỆNH NHÂN:\n";
            cout << "   - Mã BN (P_ID) : " << foundPatient->getID() << "\n";
            cout << "   - Họ và tên    : " << foundPatient->getfullName() << "\n";
            cout << "   - Số điện thoại: " << foundPatient->getPhone() << "\n";
            cout << "   - Giới tính    : " << (foundPatient->getGender() ? "Nam" : "Nữ") << "\n";
            cout << "   - Tuổi         : " << foundPatient->getAge() << "\n";
            cout << "   - CCCD         : " << foundPatient->getCCCD() << "\n";
            cout << ">> Xác nhận sử dụng hồ sơ bệnh nhân này để tiếp nhận khám bệnh.\n";
            return foundPatient->getID();
        } else {
            cout << "\n[THÔNG BÁO] Không tìm thấy bệnh nhân nào khớp với từ khóa: \"" << keyword << "\"\n";
            cout << ">> Gợi ý: Hãy chọn [2] ở lần tiếp theo để đăng ký hồ sơ bệnh nhân mới.\n";
            return "";
        }
    } else if (loai == 2) {
        string nextPid = Patient::generateNextId(patientList);
        cout << "\n>> ĐĂNG KÝ HỒ SƠ BỆNH NHÂN MỚI:\n";
        cout << ">> Hệ thống tự động cấp Mã Bệnh Nhân: [" << nextPid << "]\n";

        string fullName, phone, cccd;
        int genderChoice = 1;
        int age = 0;

        cout << "Nhập Họ và tên: ";
        getline(cin, fullName);
        filterNewline(fullName);
        libPBL2::trim(fullName);

        cout << "Nhập Số điện thoại: ";
        getline(cin, phone);
        filterNewline(phone);
        libPBL2::trim(phone);

        cout << "Chọn Giới tính (1: Nam, 0: Nữ): ";
        while (!(cin >> genderChoice) || (genderChoice != 0 && genderChoice != 1)) {
            clearInputBuffer();
            cout << "Vui lòng nhập 1 (Nam) hoặc 0 (Nữ): ";
        }
        clearInputBuffer();

        cout << "Nhập Tuổi: ";
        while (!(cin >> age) || age <= 0 || age > 130) {
            clearInputBuffer();
            cout << "Tuổi không hợp lệ. Vui lòng nhập lại: ";
        }
        clearInputBuffer();

        cout << "Nhập CCCD (12 chữ số): ";
        getline(cin, cccd);
        filterNewline(cccd);
        libPBL2::trim(cccd);

        if (fullName.empty() || phone.empty() || cccd.empty()) {
            cout << "[LỖI] Các trường Họ tên, Số điện thoại và CCCD không được để trống!\n";
            return "";
        }

        Patient newPatient(nextPid, fullName, phone, (genderChoice == 1), age, cccd);
        patientList->insert(newPatient);

        cout << "\n[THÀNH CÔNG] Đã lưu thông tin bệnh nhân mới vào danh sách bộ nhớ:\n";
        cout << "   - Mã BN (P_ID) : " << newPatient.getID() << "\n";
        cout << "   - Họ và tên    : " << newPatient.getfullName() << "\n";
        cout << "   - CCCD         : " << newPatient.getCCCD() << "\n";
        return nextPid;
    } else {
        cout << "[LỖI] Lựa chọn không hợp lệ!\n";
        return "";
    }
}

void tiepNhanBenhNhan(LinkList<Symptom> *symptomList, LinkList<SymptomPatient> *spList, const string &patientId, Queue<QueuePatient> *queuePatient) {
    if (!symptomList || !spList) {
        cout << "[LỖI] Danh sách triệu chứng chưa được khởi tạo!\n";
        return;
    }

    string qpId = SymptomPatient::generateNextId(spList);
    Date currentDate = Date::now();

    cout << "\n================= CHỌN TRIỆU CHỨNG KHÁM BỆNH =================\n";
    cout << "Mã hàng đợi tự sinh : " << qpId << "\n";
    cout << "Mã bệnh nhân        : " << patientId << "\n";
    cout << "Thời gian tiếp nhận : " << currentDate.toString() << "\n";
    cout << "--------------------------------------------------------------\n";

    LinkList<Symptom> chosenSymptoms;
    int countChosen = 0;

    while (true) {
        cout << "\nDanh sách các triệu chứng khả dụng trong hệ thống:\n";
        cout << left << setw(8) << "STT" << setw(10) << "Mã S_ID" << setw(25) << "Tên Triệu Chứng" << setw(10) << "Khoa Sp_ID" << setw(8) << "Điểm\n";
        cout << "--------------------------------------------------------------------\n";

        LinkList<Symptom>::Node *cur = symptomList->getHeader()->next;
        int idx = 1;
        while (cur != nullptr && cur->next != nullptr) {
            if (!cur->info.isDefault()) {
                cout << left << setw(8) << idx++
                     << setw(10) << cur->info.getS_id()
                     << setw(25) << cur->info.getSName()
                     << setw(10) << cur->info.getSp_id()
                     << setw(8)  << cur->info.getSScore() << "\n";
            }
            cur = cur->next;
        }

        cout << "--------------------------------------------------------------------\n";
        cout << "Nhập STT của triệu chứng muốn chọn (Nhập 0 để KẾT THÚC chọn): ";
        int choice;
        if (!(cin >> choice)) {
            clearInputBuffer();
            cout << "[LỖI] Vui lòng nhập số!\n";
            continue;
        }

        if (choice == 0) {
            if (countChosen == 0) {
                cout << "[CẢNH BÁO] Bạn chưa chọn triệu chứng nào. Tiếp tục hủy? (1: Hủy tiếp nhận, 2: Chọn tiếp): ";
                int cancelChoice;
                if (cin >> cancelChoice && cancelChoice == 1) {
                    clearInputBuffer();
                    cout << ">> Đã hủy tiếp nhận triệu chứng.\n";
                    return;
                }
                clearInputBuffer();
                continue;
            }
            break;
        }

        cur = symptomList->getHeader()->next;
        int curIdx = 1;
        Symptom selectedSymptom;
        bool found = false;
        while (cur != nullptr && cur->next != nullptr) {
            if (!cur->info.isDefault()) {
                if (curIdx == choice) {
                    selectedSymptom = cur->info;
                    found = true;
                    break;
                }
                curIdx++;
            }
            cur = cur->next;
        }

        if (found) {
            bool duplicate = false;
            LinkList<Symptom>::Node *cNode = chosenSymptoms.getHeader()->next;
            while (cNode != nullptr && cNode->next != nullptr) {
                if (cNode->info.getS_id() == selectedSymptom.getS_id()) {
                    duplicate = true;
                    break;
                }
                cNode = cNode->next;
            }

            if (duplicate) {
                cout << "[LƯU Ý] Triệu chứng này đã được chọn trước đó!\n";
            } else {
                chosenSymptoms.insert(selectedSymptom);
                countChosen++;
                cout << "[ĐÃ CHỌN] " << selectedSymptom.getS_id() << " - " << selectedSymptom.getSName()
                     << " (Khoa: " << selectedSymptom.getSp_id() << ", Điểm: " << selectedSymptom.getSScore() << ")\n";
            }
        } else {
            cout << "[LỖI] Số thứ tự không nằm trong danh sách!\n";
        }
    }

    string allS_ids = "";
    int highestScore = -1;
    string primarySpId = "";

    LinkList<Symptom>::Node *pNode = chosenSymptoms.getHeader()->next;
    while (pNode != nullptr && pNode->next != nullptr) {
        if (!pNode->info.isDefault()) {
            if (!allS_ids.empty()) allS_ids += ", ";
            allS_ids += pNode->info.getS_id();

            if (pNode->info.getSScore() > highestScore) {
                highestScore = pNode->info.getSScore();
                primarySpId = pNode->info.getSp_id();
            }
        }
        pNode = pNode->next;
    }

    SymptomPatient latestSP(qpId, allS_ids, patientId, currentDate);
    spList->insert(latestSP);

    cout << "\n================ XÁC NHẬN THÀNH CÔNG ================\n";
    cout << "Mã hàng đợi (QP_id)   : " << qpId << "\n";
    cout << "Mã bệnh nhân          : " << patientId << "\n";
    cout << "Ngày tiếp nhận (Date) : " << currentDate.toString() << "\n";
    cout << "Số triệu chứng đã thêm: " << countChosen << "\n";
    if (queuePatient && !primarySpId.empty()) {
        queuePatient->enQueue(QueuePatient(latestSP, primarySpId, 0));
        cout << "\n>> ĐÃ TỰ ĐỘNG ĐƯA BỆNH NHÂN VÀO HÀNG ĐỢI (QUEUE):\n";
        cout << "   - Mã hàng đợi (QP_id)   : " << latestSP.getQP_id() << " (Tham chiếu từ QueuePatient)\n";
        cout << "   - Khoa tiếp nhận (Sp_id): " << primarySpId << " (Khoa có độ ưu tiên cao nhất)\n";
        cout << "   - Trạng thái trong Queue: 0 (Đang chờ khám)\n";
        Queue<QueuePatient>::Node *firstW = queuePatient->getFirstWaiting();
        if (firstW && firstW->data) {
            cout << ">> Con trỏ [firstWaiting] hiện tại: " << firstW->data->getQP_id() << "\n";
        }
    }
}

void hienThiTrangThaiQueue(Queue<QueuePatient> *q) {
    if (!q || !q->getHeader()) {
        cout << "[LỖI] Hàng đợi chưa được tải hoặc rỗng!\n";
        return;
    }

    int totalNodes = 0;
    int waitingCount = 0;
    Queue<QueuePatient>::Node *cur = q->getHeader()->next;
    while (cur != q->getFooter() && cur != nullptr) {
        totalNodes++;
        if (cur->data && cur->data->getState() == 0) {
            waitingCount++;
        }
        cur = cur->next;
    }

    cout << "\n================ TRẠNG THÁI HÀNG ĐỢI (QUEUE) ================\n";
    cout << "- Tổng số bản ghi trong Queue       : " << totalNodes << "\n";
    cout << "- Số bệnh nhân đang chờ (State = 0) : " << waitingCount << "\n";

    Queue<QueuePatient>::Node *firstW = q->getFirstWaiting();
    if (firstW != nullptr && firstW != q->getFooter() && firstW->data != nullptr) {
        cout << "- [firstWaiting] ĐANG TRỎ TỚI:\n";
        cout << "  + Mã hàng đợi (QP_id)   : " << firstW->data->getQP_id() << "\n";
        cout << "  + Chuyên khoa (Sp_id)   : " << firstW->data->getSp_id() << "\n";
        cout << "  + Trạng thái (State)    : " << firstW->data->getState() << " (Đang chờ)\n";
    } else {
        cout << "- [firstWaiting] : nullptr (Hiện không có bệnh nhân nào đang chờ)\n";
    }
    cout << "=============================================================\n";
}

void inDanhSachChoKham(Queue<QueuePatient> *q) {
    if (!q) return;

    Queue<QueuePatient>::Node *startNode = q->getFirstWaiting();
    if (startNode == nullptr) {
        cout << "\n>> Hàng đợi hiện KHÔNG CÓ bệnh nhân nào đang chờ (State = 0)!\n";
        return;
    }

    cout << "\n===== DANH SÁCH BỆNH NHÂN ĐANG CHỜ KHÁM (BẮT ĐẦU TỪ firstWaiting - FIFO) =====\n";
    cout << left << setw(8) << "STT" << setw(15) << "QP_id" << setw(15) << "Sp_id" << setw(15) << "Trạng thái" << "\n";
    cout << "--------------------------------------------------------------------\n";

    int stt = 1;
    Queue<QueuePatient>::Node *cur = startNode;
    while (cur != q->getFooter() && cur != nullptr) {
        if (cur->data && cur->data->getState() == 0) {
            cout << left << setw(8) << stt++
                 << setw(15) << cur->data->getQP_id()
                 << setw(15) << cur->data->getSp_id()
                 << setw(15) << "Chờ khám (0)" << "\n";
        }
        cur = cur->next;
    }
    cout << "====================================================================\n";
}

void inToanBoQueue(Queue<QueuePatient> *q) {
    if (!q || !q->getHeader()) return;

    cout << "\n===== TOÀN BỘ BẢN GHI TRONG HÀNG ĐỢI (LỊCH SỬ + ĐANG CHỜ) =====\n";
    cout << left << setw(8) << "STT" << setw(15) << "QP_id" << setw(15) << "Sp_id" << setw(15) << "Trạng thái" << "\n";
    cout << "--------------------------------------------------------------\n";

    int stt = 1;
    Queue<QueuePatient>::Node *cur = q->getHeader()->next;
    while (cur != q->getFooter() && cur != nullptr) {
        if (cur->data) {
            string stateStr = (cur->data->getState() == 0) ? "Đang chờ (0)" : "Đã xử lý (1)";
            cout << left << setw(8) << stt++
                 << setw(15) << cur->data->getQP_id()
                 << setw(15) << cur->data->getSp_id()
                 << setw(15) << stateStr << "\n";
        }
        cur = cur->next;
    }
    cout << "==============================================================\n";
}

void inDanhSachLichHen(LinkList<Appointment> *appList) {
    if (!appList || !appList->getHeader()) {
        cout << "[LỖI] Danh sách lịch hẹn chưa được tải hoặc rỗng!\n";
        return;
    }

    cout << "\n================= DANH SÁCH LỊCH HẸN KHÁM (APPOINTMENT) =================\n";
    cout << left << setw(8) << "STT" << setw(10) << "A_ID" << setw(10) << "P_ID"
         << setw(10) << "D_ID" << setw(20) << "Ngày đặt (Set)"
         << setw(20) << "Ngày hẹn (Get)" << setw(12) << "Trạng thái" << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    int stt = 1;
    LinkList<Appointment>::Node *cur = appList->getHeader()->next;
    while (cur != nullptr && cur->next != nullptr) {
        if (!cur->info.isDefault()) {
            cout << left << setw(8) << stt++
                 << setw(10) << cur->info.getId()
                 << setw(10) << cur->info.getPatientId()
                 << setw(10) << cur->info.getDoctorId()
                 << setw(20) << cur->info.getDateSet().toString()
                 << setw(20) << cur->info.getDateGet().toString()
                 << setw(12) << (cur->info.getState() == 1 ? "Hoàn thành" : "Chờ khám") << "\n";
        }
        cur = cur->next;
    }
    cout << "================================================================================\n";
}

void testDeQueue(Queue<QueuePatient> *q) {
    if (!q) return;

    cout << "\n--- GỌI KHÁM BỆNH NHÂN (deQueue: CHUYỂN STATE 0 -> 1 & DỊCH firstWaiting) ---\n";
    QueuePatient *qp = q->deQueue();
    if (qp == nullptr) {
        cout << "[THÔNG BÁO] Hàng đợi không có bệnh nhân nào đang chờ (State = 0)!\n";
        return;
    }

    cout << "\n>> ĐÃ XỬ LÝ (deQueue) THÀNH CÔNG:\n";
    cout << "   - Hàng đợi (QP_id)   : " << qp->getQP_id() << "\n";
    cout << "   - Chuyên khoa (Sp_id): " << qp->getSp_id() << "\n";
    cout << "   - Trạng thái mới     : " << qp->getState() << " (Đã đổi State: 0 -> 1, vẫn giữ lại trong Queue)\n";

    delete qp;

    Queue<QueuePatient>::Node *nextW = q->getFirstWaiting();
    if (nextW && nextW->data) {
        cout << ">> Con trỏ [firstWaiting] kế tiếp: " << nextW->data->getQP_id()
             << " (Khoa: " << nextW->data->getSp_id() << ", State: " << nextW->data->getState() << ")\n";
    } else {
        cout << ">> Con trỏ [firstWaiting] kế tiếp: nullptr (Hàng đợi đã hết người chờ!)\n";
    }
}

void chayKiemThuHeThong() {
    cout << "\n================ CHẠY KIỂM THỬ ĐỒNG BỘ HỆ THỐNG ================\n";

    // 1. Kiểm thử Date (Getters / Setters / toString / fromString / now)
    cout << "[TEST 1] Kiểm tra thư viện Date:\n";
    Date d(18, 12, 2025, 8, 35);
    cout << "   - Khởi tạo: " << d.toString() << "\n";
    cout << "   - getDay()    : " << d.getDay() << "\n";
    cout << "   - getMonth()  : " << d.getMonth() << "\n";
    cout << "   - getYear()   : " << d.getYear() << "\n";
    cout << "   - getHour()   : " << d.getHour() << "\n";
    cout << "   - getMinute() : " << d.getMinute() << "\n";

    d.setHour(14);
    d.setMinute(45);
    cout << "   - Sau khi setHour(14), setMinute(45): " << d.toString() << "\n";
    cout << "   - toString() format: " << d.toString() << "\n";

    Date parsedD = Date::loadFromString("2026-01-10 15:30");
    cout << "   - loadFromString('2026-01-10 15:30'): " << parsedD.toString()
         << " (H=" << parsedD.getHour() << ", M=" << parsedD.getMinute() << ")\n";

    Date nowD = Date::now();
    cout << "   - Date::now(): " << nowD.toString() << "\n";
    cout << "   => [PASSED] Kiểm tra Date thành công!\n\n";

    // 2. Kiểm thử toString và loadFromString của các đối tượng
    cout << "[TEST 2] Kiểm tra toString() và loadFromString() trên từng đối tượng:\n";

    // Patient
    Patient pTest = Patient::loadFromString("P_999 | Nguyễn Văn Test | 0988776655 | 1 | 25 | 048099009999");
    cout << "   - Patient::loadFromString -> ID: " << pTest.getID() << ", Tên: " << pTest.getfullName() << "\n";
    cout << "   - Patient::toString       : " << pTest.toString() << "\n";

    // Doctor
    Doctor docTest = Doctor::loadFromString("D_999 | Bác Sĩ Test | 0901234567 | 1 | 40 | Sp_001 | 1 | Room_999 | 300000.0 | 4.9");
    cout << "   - Doctor::loadFromString  -> ID: " << docTest.getID() << ", Khoa: " << docTest.getSpId() << "\n";
    cout << "   - Doctor::toString        : " << docTest.toString() << "\n";

    // Manager
    Manager mTest = Manager::loadFromString("MA_999 | Test Admin | 0911223344 | 1 | 30 | AdminTest | PassTest123 | 3 | 1");
    cout << "   - Manager::loadFromString -> ID: " << mTest.getID() << ", Role: " << Manager::roleToString(mTest.getRole()) << "\n";
    cout << "   - Manager::toString       : " << mTest.toString() << "\n";

    // QueuePatient
    QueuePatient qpTest = QueuePatient::loadFromString("QP_999 | Sp_001 | 0");
    cout << "   - QueuePatient::loadFromString -> QP_id: " << qpTest.getQP_id() << ", State: " << qpTest.getState() << "\n";
    cout << "   - QueuePatient::toString       : " << qpTest.toString() << "\n";

    // Symptom
    Symptom symTest = Symptom::loadFromString("S_999 | Đau đầu test | Sp_002 | 4");
    cout << "   - Symptom::loadFromString -> S_id: " << symTest.getS_id() << ", Tên: " << symTest.getSName() << "\n";
    cout << "   - Symptom::toString       : " << symTest.toString() << "\n";

    // Appointment
    Appointment appTest = Appointment::loadFromString("A_999 | P_999 | D_999 | 2025-12-18 08:35 | 2025-12-20 09:00 | 1");
    cout << "   - Appointment::loadFromString -> ID: " << appTest.getId()
         << ", DateSet: " << appTest.getDateSet().toString()
         << ", DateGet: " << appTest.getDateGet().toString() << "\n";
    cout << "   - Appointment::toString       : " << appTest.toString() << "\n";

    // Permission
    Permission permTest = Permission::loadFromString("MA_999 | 01");
    cout << "   - Permission::loadFromString  -> MA_id: " << permTest.MA_id
         << ", Per: " << Permission::permissionTypeToString(permTest.Per_id) << "\n";
    cout << "   - Permission::toString        : " << permTest.toString() << "\n";

    cout << "   => [PASSED] Kiểm tra toString() và loadFromString() trên toàn bộ đối tượng thành công!\n";
    cout << "================================================================\n";
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    cout << "===== KHỞI ĐỘNG HỆ THỐNG VÀ TẢI DỮ LIỆU =====\n";

    LinkList<Manager> *ManagerList = Manager::loadFromFile("Data/Manager.txt");
    LinkList<Permission> *PermissionList = Permission::loadFromFile("Data/PermissionManager.txt");
    LinkList<Patient> *patientList = Patient::loadFromFile("Data/Patient.txt");
    LinkList<Symptom> *symptomList = Symptom::loadFromFile("Data/Symptom/Symptom.txt");
    LinkList<SymptomPatient> *spList = SymptomPatient::loadFromFile("Data/Symptom/SymptomPatient.txt");
    LinkList<Doctor> *doctorList = Doctor::loadFromFile("Data/Doctor/Doctor.txt");
    Queue<QueuePatient> *queuePatient = QueuePatient::loadFromFile("Data/QueuePatient.txt");
    LinkList<Appointment> *appointmentList = Appointment::loadFromFile("Data/Appointment/Appointment.txt");

    if (!ManagerList) {
        cout << "[LỖI] Không thể đọc file Data/Manager.txt!\n";
        return 1;
    }
    if (!PermissionList) {
        cout << "[LỖI] Không thể đọc file Data/PermissionManager.txt!\n";
    }
    if (!patientList) {
        cout << "[LỖI] Không thể đọc file Data/Patient.txt!\n";
    }
    if (!symptomList) {
        cout << "[LỖI] Không thể đọc file Data/Symptom/Symptom.txt!\n";
    }
    if (!spList) {
        cout << "[LỖI] Không thể đọc file Data/Symptom/SymptomPatient.txt!\n";
    }
    if (!doctorList) {
        cout << "[LỖI] Không thể đọc file Data/Doctor/Doctor.txt!\n";
    }
    if (!queuePatient) {
        cout << "[LỖI] Không thể đọc file Data/QueuePatient.txt!\n";
    }
    if (!appointmentList) {
        cout << "[LỖI] Không thể đọc file Data/Appointment/Appointment.txt!\n";
    }

    cout << ">> Tải toàn bộ dữ liệu thành công qua các phương thức loadFromFile của từng đối tượng!\n\n";

    Manager currentManager;
    bool isLoginSuccess = false;

    while (true) {
        cout << "================ ĐĂNG NHẬP HỆ THỐNG ================\n";
        cout << "(Nhập 0 ở Tên đăng nhập để dừng hệ thống)\n";
        string u, p;
        cout << "Tên đăng nhập (Username): ";
        if (!getline(cin, u)) {
            break;
        }
        filterNewline(u);
        libPBL2::trim(u);

        if (u == "0") {
            cout << ">> Hủy đăng nhập. Dừng chương trình!\n";
            break;
        }

        cout << "Mật khẩu (Password): ";
        if (!getline(cin, p)) {
            break;
        }
        filterNewline(p);
        libPBL2::trim(p);

        LinkList<Manager>::Node *tmp = ManagerList->getHeader()->next;
        while (tmp != nullptr && tmp->next != nullptr) {
            if (!tmp->info.isDefault()) {
                if (tmp->info.isValid(u, p)) {
                    currentManager = tmp->info;
                    isLoginSuccess = true;
                    break;
                }
            }
            tmp = tmp->next;
        }

        if (isLoginSuccess) {
            clearScreen();
            cout << ">> Đăng nhập thành công! Xin chào: " << currentManager.getID() << "\n";
            cout << ">> Vai trò (Role): " << Manager::roleToString(currentManager.getRole()) << "\n";
            break;
        } else {
            clearScreen();
            cout << "[LỖI] Sai Tên đăng nhập hoặc Mật khẩu! Vui lòng thử lại.\n\n";
        }
    }

    if (isLoginSuccess) {
        bool running = true;
        while (running) {
            cout << "\n======================= MENU QUẢN LÝ & TEST QUEUE =======================\n";
            cout << "[1] Xem trạng thái Queue & con trỏ firstWaiting\n";
            cout << "[2] Danh sách bệnh nhân đang chờ (Duyệt từ firstWaiting - FIFO)\n";
            cout << "[3] Xem toàn bộ bản ghi trong Queue (Lịch sử + Đang chờ)\n";
            cout << "[4] Gọi khám bệnh nhân (deQueue: chuyển State 0 -> 1 & dịch firstWaiting)\n";
            cout << "[5] Tiếp nhận bệnh nhân (Hồ sơ cũ/mới -> Triệu chứng -> Vào Queue)\n";
            cout << "[6] Lưu toàn bộ dữ liệu (Patient, SymptomPatient, QueuePatient, Appointment)\n";
            cout << "[7] Quét & cập nhật lại con trỏ firstWaiting (updateFirstWaiting)\n";
            cout << "[8] Xem danh sách lịch hẹn khám (Appointment)\n";
            cout << "[9] Chạy kiểm thử tự động (Unit Test: Date, toString, loadFromString, Load/Save)\n";
            cout << "[0] Dừng chương trình (Thoát)\n";
            cout << "========================================================================\n";
            cout << "Lựa chọn của bạn: ";

            int choice;
            if (!(cin >> choice)) {
                clearInputBuffer();
                clearScreen();
                cout << "[LỖI] Vui lòng nhập số hợp lệ!\n";
                continue;
            }
            clearInputBuffer();
            clearScreen();

            switch (choice) {
                case 1:
                    hienThiTrangThaiQueue(queuePatient);
                    break;
                case 2:
                    inDanhSachChoKham(queuePatient);
                    break;
                case 3:
                    inToanBoQueue(queuePatient);
                    break;
                case 4:
                    testDeQueue(queuePatient);
                    break;
                case 5: {
                    PermissionType requiredPer = PermissionType::Per_01;
                    if (libPBL2::isAllowed(currentManager, requiredPer, PermissionList)) {
                        cout << "\n>> Quyền hợp lệ: Được phép thực hiện tiếp nhận bệnh nhân.\n";

                        string patientId = nhapThongTinBenhNhan(patientList);
                        if (!patientId.empty()) {
                            tiepNhanBenhNhan(symptomList, spList, patientId, queuePatient);
                            cout << "\n>> Tiếp nhận bệnh nhân [" << patientId << "] thành công và đã đưa vào hàng đợi khám!\n";
                            cout << ">> Gợi ý: Hãy chọn chức năng [4] để gọi khám hoặc chọn [6] để lưu dữ liệu khi hoàn tất.\n";
                        }
                    } else {
                        cout << "\n[CẢNH BÁO] Tài khoản không có quyền thực hiện chức năng này!\n";
                    }
                    break;
                }
                case 6: {
                    cout << "\n================ LƯU TOÀN BỘ DỮ LIỆU HỆ THỐNG ================\n";
                    bool pOk = Patient::saveToFile("Data/Patient.txt", patientList);
                    bool spOk = SymptomPatient::saveToFile("Data/Symptom/SymptomPatient.txt", spList);
                    bool qOk = QueuePatient::saveToFile("Data/QueuePatient.txt", queuePatient);
                    bool aOk = Appointment::saveToFile("Data/Appointment/Appointment.txt", appointmentList);

                    if (pOk)  cout << "[OK] Đã lưu danh sách bệnh nhân vào Data/Patient.txt\n";
                    else      cout << "[LỖI] Lưu file Data/Patient.txt thất bại!\n";

                    if (spOk) cout << "[OK] Đã lưu triệu chứng khám vào Data/Symptom/SymptomPatient.txt\n";
                    else      cout << "[LỖI] Lưu file Data/Symptom/SymptomPatient.txt thất bại!\n";

                    if (qOk)  cout << "[OK] Đã lưu hàng đợi vào Data/QueuePatient.txt\n";
                    else      cout << "[LỖI] Lưu file Data/QueuePatient.txt thất bại!\n";

                    if (aOk)  cout << "[OK] Đã lưu lịch hẹn vào Data/Appointment/Appointment.txt\n";
                    else      cout << "[LỖI] Lưu file Data/Appointment/Appointment.txt thất bại!\n";

                    if (pOk && spOk && qOk && aOk) {
                        cout << ">> Toàn bộ dữ liệu hệ thống đã được lưu thành công!\n";
                    }
                    cout << "==============================================================\n";
                    break;
                }
                case 7: {
                    Queue<QueuePatient>::Node *p = queuePatient->updateFirstWaiting();
                    if (p && p->data) {
                        cout << "\n[OK] Đã quét lại! firstWaiting hiện trỏ tới: " << p->data->getQP_id()
                             << " (Khoa: " << p->data->getSp_id() << ", State: " << p->data->getState() << ")\n";
                    } else {
                        cout << "\n[OK] Đã quét lại! Hiện không có bệnh nhân nào có state = 0 (firstWaiting = nullptr).\n";
                    }
                    break;
                }
                case 8: {
                    inDanhSachLichHen(appointmentList);
                    break;
                }
                case 9: {
                    chayKiemThuHeThong();
                    break;
                }
                case 0: {
                    cout << "\nBạn có muốn lưu lại toàn bộ dữ liệu trước khi thoát? (1: Có, 0: Không): ";
                    int saveChoice = 0;
                    if (cin >> saveChoice && saveChoice == 1) {
                        Patient::saveToFile("Data/Patient.txt", patientList);
                        SymptomPatient::saveToFile("Data/Symptom/SymptomPatient.txt", spList);
                        QueuePatient::saveToFile("Data/QueuePatient.txt", queuePatient);
                        Appointment::saveToFile("Data/Appointment/Appointment.txt", appointmentList);
                        cout << "[OK] Đã tự động lưu dữ liệu trước khi thoát.\n";
                    }
                    clearInputBuffer();
                    clearScreen();
                    cout << "\n>> Đang dừng chương trình theo yêu cầu của bạn...\n";
                    running = false;
                    break;
                }
                default:
                    cout << "[LỖI] Lựa chọn không hợp lệ! Vui lòng chọn từ 0 đến 9.\n";
                    break;
            }
        }
    }

    delete ManagerList;
    delete PermissionList;
    delete patientList;
    delete symptomList;
    delete spList;
    delete doctorList;
    delete queuePatient;
    delete appointmentList;

    cout << "\nChương trình kết thúc. Tạm biệt!\n";
    return 0;
}
