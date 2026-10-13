// =========================================================================================
// GIẢI THÍCH CÁC MACRO ĐỊNH NGHĨA Ở ĐẦU FILE:
// 1. _CRT_SECURE_NO_WARNINGS:
//    - Microsoft Visual C++ (MSVC) coi các hàm chuẩn C cổ điển (như localtime, sscanf, strcpy, fopen)
//      là không an toàn (unsafe) vì tiềm ẩn nguy cơ tràn bộ đệm (buffer overflow) và phát sinh mã lỗi C4996.
//    - Định nghĩa macro này giúp vô hiệu hóa các cảnh báo/lỗi ép buộc đó, cho phép biên dịch mượt mà
//      code C/C++ chuẩn trên cả MSVC và GCC/Clang.
//
// 2. NOMINMAX:
//    - Header <windows.h> mặc định định nghĩa 2 macro toàn cục là min(a,b) và max(a,b).
//    - Các macro này xung đột trực tiếp với các hàm template chuẩn trong C++ như std::min, std::max
//      hoặc std::numeric_limits<T>::max().
//    - Định nghĩa NOMINMAX trước khi include <windows.h> để yêu cầu Windows SDK không sinh 2 macro này.
// =========================================================================================
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <cstdio>

#include "libPBL2.h"
#include "Manager.h"
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "Doctor.h"
#include "Date.h"
#include "LinkList.h"

using namespace std;

// =========================================================================================
// BỘ LỌC '\n' VÀ DỌN DẸP BỘ ĐỆM CIN TRÁNH TRÔI LỆNH
// =========================================================================================

// Bộ lọc xóa sạch các ký tự '\r' và '\n' còn sót trong chuỗi
void filterNewline(string &str) {
    libPBL2::filterNewline(str);
}

// Bộ lọc dọn sạch phần còn lại của dòng (bao gồm ký tự '\n') sau khi dùng cin >>
void clearInputBuffer() {
    cin.clear();
    cin.ignore(10000, '\n');
}

// Hàm tiếp nhận bệnh nhân & chọn triệu chứng (HOÀN TOÀN KHÔNG DÙNG std::vector)
void tiepNhanBenhNhan(LinkList<Symptom> *symptomList, LinkList<SymptomPatient> *spList, const string &patientId) {
    if (!symptomList || !symptomList->getHeader() || !spList) {
        cout << "[LỖI] Dữ liệu triệu chứng hoặc lượt khám chưa sẵn sàng!\n";
        return;
    }

    cout << "\n================ TIẾP NHẬN BỆNH NHÂN ================\n";
    cout << "Mã bệnh nhân: " << patientId << "\n";
    cout << "\n--- DANH SÁCH TRIỆU CHỨNG ---\n";

    // Đếm và hiển thị trực tiếp danh sách triệu chứng từ LinkList
    int totalSymptoms = 0;
    LinkList<Symptom>::Node *curSym = symptomList->getHeader()->next;
    while (curSym != nullptr && curSym->next != nullptr) {
        if (!curSym->info.isDefault()) {
            totalSymptoms++;
            cout << "[" << totalSymptoms << "] " 
                 << left << setw(25) << curSym->info.getSName()
                 << " (Khoa: " << curSym->info.getSp_id() 
                 << " | Điểm: " << curSym->info.getSScore() << ")\n";
        }
        curSym = curSym->next;
    }

    if (totalSymptoms == 0) {
        cout << "[THÔNG BÁO] Danh sách triệu chứng trống!\n";
        return;
    }
    cout << "[0] >> XÁC NHẬN (CONFIRM) VÀ KẾT THÚC CHỌN <<\n\n";

    // Sử dụng LinkList<Symptom> để lưu danh sách các triệu chứng đã chọn (thay thế std::vector)
    LinkList<Symptom> trieuChungDaChon;
    int countChosen = 0;
    int choice;

    while (true) {
        cout << "Chọn mã triệu chứng (nhập 0 để kết thúc): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Vui lòng nhập số hợp lệ!\n";
            continue;
        }

        if (choice == 0) {
            if (countChosen == 0) {
                cout << "Chưa chọn triệu chứng nào, vui lòng chọn ít nhất 1 triệu chứng!\n";
                continue;
            }
            break;
        }

        if (choice >= 1 && choice <= totalSymptoms) {
            // Tìm triệu chứng thứ 'choice' trong symptomList
            LinkList<Symptom>::Node *findNode = symptomList->getHeader()->next;
            int curIdx = 1;
            Symptom selected;
            while (findNode != nullptr && findNode->next != nullptr) {
                if (!findNode->info.isDefault()) {
                    if (curIdx == choice) {
                        selected = findNode->info;
                        break;
                    }
                    curIdx++;
                }
                findNode = findNode->next;
            }

            // Kiểm tra xem triệu chứng này đã được chọn trước đó chưa
            bool daTonTai = false;
            LinkList<Symptom>::Node *chkNode = trieuChungDaChon.getHeader()->next;
            while (chkNode != nullptr && chkNode->next != nullptr) {
                if (chkNode->info.getS_id() == selected.getS_id()) {
                    daTonTai = true;
                    break;
                }
                chkNode = chkNode->next;
            }

            if (daTonTai) {
                cout << "-> Triệu chứng [" << selected.getSName() << "] đã được chọn trước đó!\n";
            } else {
                trieuChungDaChon.insert(selected);
                countChosen++;
                cout << "-> Đã thêm: " << selected.getSName() << "\n";
            }
        } else {
            cout << "Lựa chọn không hợp lệ, vui lòng chọn lại!\n";
        }
    }

    // Lấy ngày hiện tại thông qua class Date
    Date currentDate = Date::now();
    string spId = libPBL2::generateNext_id(spList);

    // Chèn từng triệu chứng đã chọn vào danh sách SymptomPatient
    LinkList<Symptom>::Node *pNode = trieuChungDaChon.getHeader()->next;
    while (pNode != nullptr && pNode->next != nullptr) {
        if (!pNode->info.isDefault()) {
            spList->insert(SymptomPatient(spId, pNode->info.getS_id(), patientId, currentDate));
        }
        pNode = pNode->next;
    }

    cout << "\n================ XÁC NHẬN THÀNH CÔNG ================\n";
    cout << "Mã lượt khám (SP_id)  : " << spId << "\n";
    cout << "Mã bệnh nhân          : " << patientId << "\n";
    cout << "Ngày tiếp nhận (Date) : " << currentDate.toDisplayString() 
         << " (Định dạng file: " << currentDate.toString() << ")\n";
    cout << "Số triệu chứng đã thêm: " << countChosen << "\n";
}

int main() {
    // Thiết lập font console UTF-8 để hiển thị tiếng Việt
    SetConsoleOutputCP(65001);  
    SetConsoleCP(65001);

    cout << "===== KHỞI ĐỘNG HỆ THỐNG VÀ TẢI DỮ LIỆU =====\n";

    // 1. Tải dữ liệu từ file
    LinkList<Manager> *ManagerList = libPBL2::LoadFromFile<Manager>("Data/Manager.txt");
    LinkList<libPBL2::Permission> *PermissionList = libPBL2::LoadFromFile<libPBL2::Permission>("Data/PermissionManager.txt");
    LinkList<Symptom> *symptomList = libPBL2::LoadFromFile<Symptom>("Data/Symptom/Symptom.txt");
    LinkList<SymptomPatient> *spList = libPBL2::LoadFromFile<SymptomPatient>("Data/Symptom/SymptomPatient.txt");
    LinkList<Doctor> *doctorList = libPBL2::LoadFromFile<Doctor>("Data/Doctor/Doctor.txt");

    // Kiểm tra an toàn xem các danh sách có tải thành công không
    if (!ManagerList) {
        cout << "[LỖI] Không thể đọc file Data/Manager.txt!\n";
        return 1;
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

    cout << "Tải dữ liệu hoàn tất.\n\n";

    // 2. Đăng nhập tài khoản
    cout << "===== ĐĂNG NHẬP HỆ THỐNG =====\n";
    string username, password;
    cout << "Username: "; 
    cin >> username; 
    filterNewline(username);

    cout << "Password: "; 
    cin >> password; 
    filterNewline(password);

    clearInputBuffer(); // Bộ lọc xóa '\n' sau khi nhập

    bool isLoginSuccess = false;
    Manager currentManager;

    LinkList<Manager>::Node *tmpNode = ManagerList->getHeader()->next;
    while (tmpNode != nullptr && tmpNode->next != nullptr) {
        if (tmpNode->info.isValid(username, password)) {
            isLoginSuccess = true;
            currentManager = tmpNode->info;
            break;
        }
        tmpNode = tmpNode->next;
    }

    if (isLoginSuccess) {
        cout << "\n>> Đăng nhập thành công! Xin chào: " << currentManager.getID() << "\n";
        cout << ">> Vai trò (Role): " << libPBL2::toStr(currentManager.getRole()) << "\n";

        // 3. Kiểm tra phân quyền (Test hàm isAllowed)
        libPBL2::PermissionType requiredPer = libPBL2::PermissionType::Per_01;
        if (libPBL2::isAllowed(currentManager, requiredPer, PermissionList)) {
            cout << ">> Quyền hợp lệ: Được phép thực hiện tiếp nhận bệnh nhân.\n";

            string patientId;
            cout << "\nNhập mã bệnh nhân cần tiếp nhận (ví dụ P_001): ";
            cin >> patientId;
            filterNewline(patientId);
            clearInputBuffer(); // Dọn sạch buffer sau khi nhập mã bệnh nhân

            tiepNhanBenhNhan(symptomList, spList, patientId);

            // 4. Lưu lại dữ liệu sau khi tiếp nhận
            if (libPBL2::SaveToFile<SymptomPatient>("Data/Symptom/SymptomPatient.txt", spList)) {
                cout << "\n[OK] Đã lưu dữ liệu SymptomPatient vào file thành công.\n";
            } else {
                cout << "\n[LỖI] Lưu file Data/Symptom/SymptomPatient.txt thất bại!\n";
            }
        } else {
            cout << "\n[CẢNH BÁO] Tài khoản không có quyền thực hiện chức năng này!\n";
        }
    } else {
        cout << "\n[LỖI] Tài khoản hoặc mật khẩu không chính xác!\n";
    }

    // 5. Giải phóng bộ nhớ động
    delete ManagerList;
    delete PermissionList;
    delete symptomList;
    delete spList;
    delete doctorList;

    cout << "\nChương trình kết thúc.\n";
    return 0;
}