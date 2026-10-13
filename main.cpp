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
#include "Date.h"
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
            return foundPatient->getID();
        } else {
            cout << "\n[THÔNG BÁO] Không tìm thấy bệnh nhân với thông tin '" << keyword << "'!\n";
            cout << "Bạn có muốn tạo hồ sơ bệnh nhân MỚI không? (1: Có, 0: Không): ";
            int tiepTuc;
            if (!(cin >> tiepTuc) || tiepTuc != 1) {
                clearInputBuffer();
                clearScreen();
                return "";
            }
            clearInputBuffer();
            clearScreen();
            loai = 2;
        }
    }

    if (loai == 2) {

        string newId = libPBL2::generateNext_id<Patient>(patientList);
        cout << "\n--- ĐĂNG KÝ HỒ SƠ BỆNH NHÂN MỚI ---\n";
        cout << ">> Mã bệnh nhân được cấp mới: " << newId << "\n";

        string fullName, phone, cccd;
        int genderInput = 1, age = 0;

        cout << "Nhập Họ và tên: ";
        getline(cin, fullName);
        filterNewline(fullName);
        libPBL2::trim(fullName);

        cout << "Nhập Số điện thoại: ";
        getline(cin, phone);
        filterNewline(phone);
        libPBL2::trim(phone);

        cout << "Nhập Giới tính (1: Nam, 0: Nữ) [Mặc định 1]: ";
        string genderStr;
        getline(cin, genderStr);
        filterNewline(genderStr);
        libPBL2::trim(genderStr);
        if (!genderStr.empty()) {
            try { genderInput = stoi(genderStr); } catch (...) { genderInput = 1; }
        }

        cout << "Nhập Tuổi: ";
        string ageStr;
        getline(cin, ageStr);
        filterNewline(ageStr);
        libPBL2::trim(ageStr);
        if (!ageStr.empty()) {
            try { age = stoi(ageStr); } catch (...) { age = 0; }
        }

        cout << "Nhập Căn cước công dân (CCCD): ";
        getline(cin, cccd);
        filterNewline(cccd);
        libPBL2::trim(cccd);

        Patient newPatient(newId, fullName, phone, (genderInput == 1), age, cccd);
        patientList->insert(newPatient);

        cout << "\n[OK] Đã đăng ký thành công hồ sơ bệnh nhân mới: " << newId << " - " << fullName << "\n";
        return newId;
    }

    return "";
}

void tiepNhanBenhNhan(LinkList<Symptom> *symptomList, LinkList<SymptomPatient> *spList, const string &patientId, Queue<QueuePatient> *queuePatient = nullptr) {
    if (!symptomList || !symptomList->getHeader() || !spList) {
        cout << "[LỖI] Dữ liệu triệu chứng hoặc lượt khám chưa sẵn sàng!\n";
        return;
    }

    clearScreen();
    cout << "\n================ TIẾP NHẬN BỆNH NHÂN ================\n";
    cout << "Mã bệnh nhân: " << patientId << "\n";
    cout << "\n--- DANH SÁCH TRIỆU CHỨNG ---\n";

    int totalSymptoms = 0;
    LinkList<Symptom>::Node *curSym = symptomList->getHeader()->next;
    while (curSym != nullptr && curSym->next != nullptr) {
        if (!curSym->info.isDefault()) {
            totalSymptoms++;
            cout << "[" << totalSymptoms << "] "
                 << libPBL2::padRightUtf8(curSym->info.getSName(), 25)
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
            clearInputBuffer();
            clearScreen();
            break;
        }

        if (choice >= 1 && choice <= totalSymptoms) {

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

    Date currentDate = Date::now();
    string spId = libPBL2::generateNext_id(spList);

    string primarySpId = "";
    int highestScore = -1;
    string allS_ids = "";
    LinkList<Symptom>::Node *pNode = trieuChungDaChon.getHeader()->next;
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

    SymptomPatient latestSP(spId, allS_ids, patientId, currentDate);
    spList->insert(latestSP);

    cout << "\n================ XÁC NHẬN THÀNH CÔNG ================\n";
    cout << "Mã lượt khám (SP_id)  : " << spId << "\n";
    cout << "Mã bệnh nhân          : " << patientId << "\n";
    cout << "Ngày tiếp nhận (Date) : " << currentDate.toDisplayString()
         << " (Định dạng file: " << currentDate.toString() << ")\n";
    cout << "Số triệu chứng đã thêm: " << countChosen << "\n";
    if (queuePatient && !primarySpId.empty()) {
        queuePatient->enQueue(QueuePatient(latestSP, primarySpId, 0));
        cout << "\n>> ĐÃ TỰ ĐỘNG ĐƯA BỆNH NHÂN VÀO HÀNG ĐỢI (QUEUE):\n";
        cout << "   - Mã lượt khám (SP_id)  : " << latestSP.getSP_id() << " (Nhận từ SymptomPatient)\n";
        cout << "   - Khoa tiếp nhận (Sp_id): " << primarySpId << " (Khoa có độ ưu tiên cao nhất)\n";
        cout << "   - Trạng thái trong Queue: 0 (Đang chờ khám)\n";
        Queue<QueuePatient>::Node *firstW = queuePatient->getFirstWaiting();
        if (firstW && firstW->data) {
            cout << ">> Con trỏ [firstWaiting] hiện tại: " << firstW->data->getSP_id() << "\n";
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
        cout << "  + Mã lượt khám (SP_id)  : " << firstW->data->getSP_id() << "\n";
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
    cout << left << setw(8) << "STT" << setw(15) << "SP_id" << setw(15) << "Sp_id" << setw(15) << "Trạng thái" << "\n";
    cout << "--------------------------------------------------------------------\n";

    int stt = 1;
    Queue<QueuePatient>::Node *cur = startNode;
    while (cur != q->getFooter() && cur != nullptr) {
        if (cur->data && cur->data->getState() == 0) {
            cout << left << setw(8) << stt++
                 << setw(15) << cur->data->getSP_id()
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
    cout << left << setw(8) << "STT" << setw(15) << "SP_id" << setw(15) << "Sp_id" << setw(15) << "Trạng thái" << "\n";
    cout << "--------------------------------------------------------------\n";

    int stt = 1;
    Queue<QueuePatient>::Node *cur = q->getHeader()->next;
    while (cur != q->getFooter() && cur != nullptr) {
        if (cur->data) {
            string stateStr = (cur->data->getState() == 0) ? "Đang chờ (0)" : "Đã xử lý (1)";
            cout << left << setw(8) << stt++
                 << setw(15) << cur->data->getSP_id()
                 << setw(15) << cur->data->getSp_id()
                 << setw(15) << stateStr << "\n";
        }
        cur = cur->next;
    }
    cout << "==============================================================\n";
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
    cout << "   - Lượt khám (SP_id)  : " << qp->getSP_id() << "\n";
    cout << "   - Chuyên khoa (Sp_id): " << qp->getSp_id() << "\n";
    cout << "   - Trạng thái mới     : " << qp->getState() << " (Đã đổi State: 0 -> 1, vẫn giữ lại trong Queue)\n";

    delete qp;

    Queue<QueuePatient>::Node *nextW = q->getFirstWaiting();
    if (nextW && nextW->data) {
        cout << ">> Con trỏ [firstWaiting] kế tiếp: " << nextW->data->getSP_id()
             << " (Khoa: " << nextW->data->getSp_id() << ", State: " << nextW->data->getState() << ")\n";
    } else {
        cout << ">> Con trỏ [firstWaiting] kế tiếp: nullptr (Hàng đợi đã hết người chờ!)\n";
    }
}

int main() {

    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    cout << "===== KHỞI ĐỘNG HỆ THỐNG VÀ TẢI DỮ LIỆU =====\n";

    LinkList<Manager> *ManagerList = libPBL2::LoadFromFile<Manager>("Data/Manager.txt");
    LinkList<libPBL2::Permission> *PermissionList = libPBL2::LoadFromFile<libPBL2::Permission>("Data/PermissionManager.txt");
    LinkList<Patient> *patientList = libPBL2::LoadFromFile<Patient>("Data/Patient.txt");
    LinkList<Symptom> *symptomList = libPBL2::LoadFromFile<Symptom>("Data/Symptom/Symptom.txt");
    LinkList<SymptomPatient> *spList = libPBL2::LoadFromFile<SymptomPatient>("Data/Symptom/SymptomPatient.txt");
    LinkList<Doctor> *doctorList = libPBL2::LoadFromFile<Doctor>("Data/Doctor/Doctor.txt");
    Queue<QueuePatient> *queuePatient = libPBL2::LoadFromFile<QueuePatient, Queue>("Data/QueuePatient.txt");

    if (!ManagerList) {
        cout << "[LỖI] Không thể đọc file Data/Manager.txt!\n";
        return 1;
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

    cout << "Tải dữ liệu hoàn tất.\n\n";

    bool isLoginSuccess = false;
    Manager currentManager;

    while (!isLoginSuccess) {
        cout << "===== ĐĂNG NHẬP HỆ THỐNG =====\n";
        cout << "(Gợi ý tài khoản admin: HoangMA001 / HoangMA001 | Nhập '0' để dừng)\n";
        string username, password;
        cout << "Username: ";
        if (!(cin >> username)) break;
        filterNewline(username);
        libPBL2::trim(username);
        if (username == "0") break;

        cout << "Password: ";
        if (!(cin >> password)) break;
        filterNewline(password);
        libPBL2::trim(password);

        clearInputBuffer();

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
            clearScreen();
            cout << ">> Đăng nhập thành công! Xin chào: " << currentManager.getID() << "\n";
            cout << ">> Vai trò (Role): " << libPBL2::toStr(currentManager.getRole()) << "\n";
            break;
        } else {
            clearScreen();
            cout << "[LỖI] Tài khoản hoặc mật khẩu không chính xác! Vui lòng thử lại.\n\n";
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
            cout << "[6] Lưu toàn bộ dữ liệu (Patient, SymptomPatient, QueuePatient)\n";
            cout << "[7] Quét & cập nhật lại con trỏ firstWaiting (updateFirstWaiting)\n";
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
                    libPBL2::PermissionType requiredPer = libPBL2::PermissionType::Per_01;
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
                    bool pOk = libPBL2::SaveToFile<Patient>("Data/Patient.txt", patientList);
                    bool spOk = libPBL2::SaveToFile<SymptomPatient>("Data/Symptom/SymptomPatient.txt", spList);
                    bool qOk = libPBL2::SaveToFile<QueuePatient, Queue>("Data/QueuePatient.txt", queuePatient);

                    if (pOk)  cout << "[OK] Đã lưu danh sách bệnh nhân vào Data/Patient.txt\n";
                    else      cout << "[LỖI] Lưu file Data/Patient.txt thất bại!\n";

                    if (spOk) cout << "[OK] Đã lưu triệu chứng khám vào Data/Symptom/SymptomPatient.txt\n";
                    else      cout << "[LỖI] Lưu file Data/Symptom/SymptomPatient.txt thất bại!\n";

                    if (qOk)  cout << "[OK] Đã lưu hàng đợi vào Data/QueuePatient.txt\n";
                    else      cout << "[LỖI] Lưu file Data/QueuePatient.txt thất bại!\n";

                    if (pOk && spOk && qOk) {
                        cout << ">> Toàn bộ dữ liệu (Patient, SymptomPatient, QueuePatient) đã được lưu thành công!\n";
                    }
                    cout << "==============================================================\n";
                    break;
                }
                case 7: {
                    Queue<QueuePatient>::Node *p = queuePatient->updateFirstWaiting();
                    if (p && p->data) {
                        cout << "\n[OK] Đã quét lại! firstWaiting hiện trỏ tới: " << p->data->getSP_id()
                             << " (Khoa: " << p->data->getSp_id() << ", State: " << p->data->getState() << ")\n";
                    } else {
                        cout << "\n[OK] Đã quét lại! Hiện không có bệnh nhân nào có state = 0 (firstWaiting = nullptr).\n";
                    }
                    break;
                }
                case 0: {
                    cout << "\nBạn có muốn lưu lại toàn bộ dữ liệu trước khi thoát? (1: Có, 0: Không): ";
                    int saveChoice = 0;
                    if (cin >> saveChoice && saveChoice == 1) {
                        libPBL2::SaveToFile<Patient>("Data/Patient.txt", patientList);
                        libPBL2::SaveToFile<SymptomPatient>("Data/Symptom/SymptomPatient.txt", spList);
                        libPBL2::SaveToFile<QueuePatient, Queue>("Data/QueuePatient.txt", queuePatient);
                        cout << "[OK] Đã tự động lưu dữ liệu trước khi thoát.\n";
                    }
                    clearInputBuffer();
                    clearScreen();
                    cout << "\n>> Đang dừng chương trình theo yêu cầu của bạn...\n";
                    running = false;
                    break;
                }
                default:
                    cout << "[LỖI] Lựa chọn không hợp lệ! Vui lòng chọn từ 0 đến 7.\n";
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

    cout << "\nChương trình kết thúc. Tạm biệt!\n";
    return 0;
}
