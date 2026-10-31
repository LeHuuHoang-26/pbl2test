# 🏥 AnTamMng - HỆ THỐNG QUẢN LÝ PHÒNG KHÁM ĐA KHOA AN TÂM

> **Hệ thống phần mềm chuyên nghiệp phục vụ quản lý lịch khám, điều phối bệnh nhân, quản lý bác sĩ, phân quyền nhân sự (RBAC) và báo cáo thống kê trực quan dành cho phòng khám đa khoa.**

[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com)
[![Framework](https://img.shields.io/badge/GUI-Qt%206.8%2B-green.svg)](https://www.qt.io/)
[![Standard](https://img.shields.io/badge/C%2B%2B-20%2F2b-red.svg)](https://isocpp.org/)
[![Compiler](https://img.shields.io/badge/Compiler-MinGW%20UCRT64%20GCC-purple.svg)](https://www.msys2.org/)
[![Interactive Diagram](https://img.shields.io/badge/Archify-Interactive%20Diagram-0d9488.svg)](pbl2-architecture.html)
[![License](https://img.shields.io/badge/Project-PBL%202-orange.svg)](#)

---

## 🌐 SƠ ĐỒ KIẾN TRÚC TƯƠNG TÁC (ARCHIFY)

Dự án đã được tích hợp sơ đồ kiến trúc động chuẩn **Archify**, cho phép quan sát trực quan luồng dữ liệu, phân tầng MVC, cơ chế phân quyền và cấu trúc dữ liệu tự xây dựng:
* 📊 **Mở sơ đồ kiến trúc tương tác (HTML):** 👉 [**`pbl2-architecture.html`**](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/pbl2-architecture.html)
* ⚙️ **Đặc tả cấu hình kiến trúc (JSON IR):** [`.archify/architecture-pbl2-overview-20261031-234500/candidate.json`](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/.archify/architecture-pbl2-overview-20261031-234500/candidate.json)
* 💡 Hỗ trợ chế độ Sáng / Tối, phóng to thu nhỏ, tìm kiếm node, animation motion và xuất ảnh PNG/SVG/WebM phục vụ báo cáo.

---

## 📖 MỤC LỤC
1. [Giới thiệu Tổng quan](#1-giới-thiệu-tổng-quan)
2. [Điểm nổi bật Kỹ thuật & Ràng buộc Thiết kế](#2-điểm-nổi-bật-kỹ-thuật--ràng-buộc-thiết-kế)
3. [Kiến trúc Hệ thống (System Architecture)](#3-kiến-trúc-hệ-thống-system-architecture)
4. [Cấu trúc Thư mục Dự án](#4-cấu-trúc-thư-mục-dự-án)
5. [Các Phân hệ Chức năng Chính](#5-các-phân-hệ-chức-năng-chính)
   - [5.1. Xác thực & Phân quyền Tài khoản Động (RBAC)](#51-xác-thực--phân-quyền-tài-khoản-động-rbac)
   - [5.2. Quản lý Đặt lịch Khám & Sàng lọc Triệu chứng (Triage)](#52-quản-lý-đặt-lịch-khám--sàng-lọc-triệu-chứng-triage)
   - [5.3. Quản lý Đội ngũ Bác sĩ (Doctor Management)](#53-quản-lý-đội-ngũ-bác-sĩ-doctor-management)
   - [5.4. Quản lý Bệnh nhân & Hồ sơ Bệnh án (Patient Management)](#54-quản-lý-bệnh-nhân--hồ-sơ-bệnh-án-patient-management)
   - [5.5. Thống kê & Báo cáo Trực quan (Analytics & Charts)](#55-thống-kê--báo-cáo-trực-quan-analytics--charts)
   - [5.6. Cấu hình Hệ thống & Quản lý Nghỉ Lễ (Config & Holidays)](#56-cấu-hình-hệ-thống--quản-lý-nghỉ-lễ-config--holidays)
6. [Quy chuẩn Ma trận Phân quyền](#6-quy-chuẩn-ma-trận-phân-quyền)
7. [Yêu cầu Môi trường & Hướng dẫn Cài đặt](#7-yêu-cầu-môi-trường--hướng-dẫn-cài-đặt)
8. [Hướng dẫn Khởi chạy Ứng dụng](#8-hướng-dẫn-khởi-chạy-ứng-dụng)
9. [Tài khoản Mẫu Trải nghiệm](#9-tài-khoản-mẫu-trải-nghiệm)
10. [Tài liệu Thuật toán Chuyên sâu](#10-tài-liệu-thuật-toán-chuyên-sâu)

---

## 1. GIỚI THIỆU TỔNG QUAN

**AnTamMng** (Phòng Khám Đa Khoa An Tâm) là giải pháp máy tính để bàn (Desktop Application) toàn diện hỗ trợ tối ưu hóa quy trình tiếp đón, sàng lọc bệnh nhân ban đầu, đặt lịch hẹn khám bệnh chính xác theo khung giờ, điều phối bác sĩ và báo cáo tài chính - chuyên môn cho cơ sở y tế đa khoa.

Ứng dụng được thiết kế theo phong cách giao diện hiện đại **Deep Dark Teal** (`#0F2A3A` - `#0A1F2C` - `#0D9488`), mang lại trải nghiệm êm mắt, chuyên nghiệp và tối ưu cho nhân viên y tế làm việc liên tục trong thời gian dài.

```mermaid
flowchart LR
    A[Bệnh nhân đến khám / Đặt hẹn] --> B[Lễ tân tiếp đón & Sàng lọc triệu chứng]
    B --> C{Đánh giá Triage Score}
    C -->|Gợi ý chuyên khoa| D[Phân bổ Bác sĩ & Khung giờ Slot]
    D --> E[Lưu lịch hẹn & Ngăn xung đột]
    E --> F[Bác sĩ khám & Hoàn thành bệnh án]
    F --> G[Cập nhật Doanh thu & Thống kê KPI]
```

---

## 2. ĐIỂM NỔI BẬT KỸ THUẬT & RÀNG BUỘC THIẾT KẾ

* **100% Thuần C++ & Cấu trúc Dữ liệu Tự xây dựng (No Forbidden STL Containers):**
  - Tuyệt đối tuân thủ quy chuẩn học phần PBL 2: Không sử dụng các container và giải thuật có sẵn của thư viện chuẩn STL (`<vector>`, `<algorithm>`, `<map>`, `<set>`, `<functional>`, `<list>`,...).
  - Tự hiện thực cấu trúc dữ liệu danh sách liên kết đôi có nút canh giữ (`LinkList<T>`) và hàng đợi FIFO (`Queue<T>`).
* **Kiến trúc Phần mềm MVC phân tách mạch lạc:**
  - **Models:** Đại diện cho các thực thể nghiệp vụ (`Human`, `Doctor`, `Patient`, `Manager`, `Appointment`, `Symptom`, `SymptomPatient`, `Permission`).
  - **Controllers:** Chứa toàn bộ logic nghiệp vụ, thuật toán sắp xếp, lọc, tính toán doanh thu và phân quyền.
  - **Views:** Giao diện đồ họa Qt 6 tương tác phong phú, tự động co giãn thích ứng từ độ phân giải 1024x768 đến 4K.
* **Portable Windows Launcher chuyên dụng:**
  - Tích hợp launcher nhị phân viết bằng C thuần (`AnTamMng.exe` ở thư mục gốc), tự động định vị thư mục làm việc, tải môi trường và khởi chạy ứng dụng lõi `bin/AnTamMng.exe` mà không gây nhấp nháy cửa sổ console đen.
* **Hệ thống Ghi Log Đồng bộ & An toàn Đa luồng (Thread-safe Logging):**
  - Sử dụng `QMutex` và bộ lọc thông điệp tùy biến (`customMessageHandler`), ghi nhận toàn bộ hoạt động vào `logs/app.log` với timestamp chuẩn tới mili-giây.
* **Tìm kiếm Tiếng Việt thông minh không phân biệt dấu:**
  - Thuật toán bóc tách dấu Unicode tiếng Việt giúp người dùng tìm kiếm tên bệnh nhân, bác sĩ, triệu chứng dễ dàng bằng cả chữ thường, chữ hoa hoặc gõ không dấu.

---

## 3. KIẾN TRÚC HỆ THỐNG (SYSTEM ARCHITECTURE)

Hệ thống được tổ chức thành 5 phân tầng rõ ràng:

```mermaid
graph TD
    subgraph UI_Layer["1. Presentation Tier (Qt5 / Qt6 GUI Views)"]
        MW["MainWindow (QStackedWidget)"]
        LoginV["Login Dialog"]
        ApptV["AppointmentView"]
        PatV["PatientView"]
        DocV["DoctorView"]
        StatV["StatisticView"]
        AccV["AccountView"]
        UIH["UIHelper Utility"]
    end

    subgraph Controller_Layer["2. Business Logic Tier (Controllers)"]
        AuthCtrl["AuthController"]
        ApptCtrl["AppointmentController"]
        PatCtrl["PatientController"]
        DocCtrl["DoctorController"]
        StatCtrl["StatisticController"]
        MngCtrl["ManagerController"]
    end

    subgraph Model_Layer["3. Domain Model Tier (Entities)"]
        H["Human (Base Class)"]
        Pat["Patient"]
        Doc["Doctor"]
        Mng["Manager"]
        Appt["Appointment"]
        Sym["Symptom"]
        SymPat["SymptomPatient"]
        Perm["Permission"]
    end

    subgraph DS_Layer["4. Custom Data Structures & Utils"]
        LL["LinkList&lt;T&gt; (Doubly Linked List)"]
        Q["Queue&lt;T&gt; (FIFO Queue)"]
        DT["Date (Custom DateTime)"]
        LP["libPBL2 (String/Validation Helpers)"]
    end

    subgraph Storage_Layer["5. Data Persistence Tier (Flat-File Storage)"]
        FAppt["data/Appointment.txt"]
        FPat["data/Patient.txt"]
        FDoc["data/Doctor/Doctor.txt"]
        FSpec["data/Doctor/Specialization.txt"]
        FSym["data/Symptom/Symptom.txt"]
        FSymPat["data/Symptom/SymptomPatient.txt"]
        FMng["data/Manager.txt"]
        FPerm["data/PermissionManager.txt"]
        FHol["Config/Lichnghile.txt"]
    end

    UI_Layer --> Controller_Layer
    Controller_Layer --> Model_Layer
    Model_Layer --> DS_Layer
    Model_Layer --> Storage_Layer
    Controller_Layer --> Storage_Layer
```

---

## 4. CẤU TRÚC THƯ MỤC DỰ ÁN

```text
pbl2_v1.1/
├── AnTamMng.exe                 # Trình khởi chạy portable gốc (C Launcher không console)
├── create_portable.bat          # Script tự động build & đóng gói bản portable độc lập
├── PBL_2.pro                    # File cấu hình dự án qmake cho Qt 6 / MinGW
├── resource.qrc                 # File tài nguyên nhúng (Fonts, Icons, Textures)
├── README.md                    # Tài liệu tổng quan chức năng ứng dụng
├── funtionAlgorithms.md         # Phân tích kỹ thuật chuyên sâu về source code & giải thuật
├── pbl2-architecture.html       # Sơ đồ kiến trúc tương tác Archify (Mở trên trình duyệt)
├── Config/                      # Cấu hình hệ sinh thái
│   ├── Lichnghile.txt           # Danh mục các ngày nghỉ lễ, Tết của phòng khám
│   └── Permission.txt           # Danh mục quyền hệ thống chuẩn (Per_1 -> Per_5)
├── data/                        # Cơ sở dữ liệu văn bản thuần (File-based Storage)
│   ├── Appointment.txt          # Dữ liệu các cuộc hẹn khám bệnh
│   ├── Manager.txt              # Danh sách tài khoản người dùng hệ thống
│   ├── Patient.txt              # Hồ sơ nhân khẩu học bệnh nhân
│   ├── PermissionManager.txt    # Bảng phân quyền động cho từng tài khoản
│   ├── Doctor/
│   │   ├── Doctor.txt           # Danh sách hồ sơ bác sĩ
│   │   └── Specialization.txt   # Danh mục chuyên khoa & phòng khám
│   └── Symptom/
│       ├── Symptom.txt          # Danh mục triệu chứng lâm sàng & điểm Triage
│       └── SymptomPatient.txt   # Bệnh án triệu chứng gắn với từng bệnh nhân
├── assets/                      # Tài nguyên giao diện
│   ├── fonts/                   # Bộ font Be Vietnam Pro & Playfair Display
│   └── textures/                # Icon app, biểu tượng mũi tên, đồ họa y tế
├── bin/                         # Thư mục chứa file nhị phân & DLL runtime Qt 6
│   ├── AnTamMng.exe             # Ứng dụng thực thi chính
│   └── *.dll                    # Các thư viện liên kết động Qt 6 & MinGW
├── logs/                        # Thư mục lưu nhật ký hoạt động hệ thống
│   └── app.log                  # File log an toàn đa luồng
└── src/                         # Mã nguồn C++ dự án
    ├── main.cpp                 # Điểm khởi đầu ứng dụng & bộ lọc log QMutex
    ├── Controllers/             # Các bộ điều khiển nghiệp vụ (6 Controllers)
    ├── DataStructures/          # Cấu trúc dữ liệu tự định nghĩa (LinkList, Queue)
    ├── Launcher/                # Mã nguồn trình khởi chạy C & Resource Icon
    ├── Models/                  # Các lớp thực thể đối tượng (8 Models)
    ├── Utils/                   # Tiện ích ngày tháng (Date) và chuỗi (libPBL2)
    └── Views/                   # Giao diện người dùng Qt 6 (6 Phân hệ View mô-đun)
```

---

## 5. CÁC PHÂN HỆ CHỨC NĂNG CHÍNH

### 5.1. Xác thực & Phân quyền Tài khoản Động (RBAC)
* **Đăng nhập an toàn:**
  - Giao diện đăng nhập hiện đại với hình họa y tế (sóng ECG, dấu thập y khoa, hiệu ứng ánh sáng ambient).
  - Kiểm tra tính hợp lệ của tài khoản: tên đăng nhập, mật khẩu, trạng thái tài khoản (`Active` / `Locked`).
  - Hỗ trợ nút bật/tắt ẩn hiện mật khẩu bảo mật.
* **Phân cấp vai trò người dùng (User Roles):**
  - **Administrator:** Quyền quản trị tối cao, kiểm tra toàn bộ thông tin hệ thống, quản lý tài khoản nhân viên, cấp/thu hồi quyền và cấu hình ngày nghỉ lễ.
  - **Receptionist (Lễ tân):** Chuyên trách tiếp đón, tra cứu hồ sơ bệnh nhân, tạo và điều phối lịch hẹn khám bệnh.
  - **Special Receptionist:** Lễ tân nâng cao được ủy quyền thêm một số tính năng quản trị thông qua cơ chế cấp quyền động.
* **An toàn dữ liệu người dùng:**
  - Ngăn chặn người dùng tự khóa tài khoản của chính mình.
  - Ngăn chặn chỉnh sửa hoặc thao tác nguy hiểm lên các tài khoản đồng cấp hoặc cấp cao hơn.

### 5.2. Quản lý Đặt lịch Khám & Sàng lọc Triệu chứng (Triage)
* **Dashboard Lịch khám Tương tác:**
  - Lịch tháng thông minh (`QCalendarWidget`) được tùy biến giao diện: tự động làm nổi bật các ngày có lịch hẹn khám, đổi màu đặc biệt cho các ngày nghỉ lễ/Tết.
  - Xem nhanh danh sách các cuộc hẹn trong ngày được chọn hoặc toàn bộ lịch sắp tới.
* **Quy trình Đặt lịch Khám 3 bước:**
  1. *Bước 1: Chọn / Thêm mới Bệnh nhân:* Tìm kiếm bệnh nhân theo tên, CCCD hoặc số điện thoại.
  2. *Bước 2: Sàng lọc Triệu chứng (Triage):* Chọn các triệu chứng bệnh nhân đang gặp phải. Hệ thống tự động tính điểm nguy cơ (`sScore`) và gợi ý chuyên khoa phù hợp nhất.
  3. *Bước 3: Chọn Bác sĩ & Khung giờ (Time Slot):* Lọc danh sách bác sĩ theo chuyên khoa, gợi ý bác sĩ có đánh giá cao nhất và ít lịch bận nhất.
* **Thuật toán Chống Trùng lịch (Conflict Detection):**
  - Tự động kiểm tra xung đột thời gian giữa các lịch khám của cùng một bác sĩ.
  - Khung giờ khám chuẩn: 30 phút/ca, trong khung giờ hành chính (07:30 - 11:30 và 13:30 - 17:30).
  - Tự động chặn đặt lịch vào các ngày nghỉ lễ, tết theo cấu hình phòng khám.
* **Quản lý Trạng thái Lịch khám Thời gian thực:**
  - `Pending (0)` - Chờ khám.
  - `Completed (1)` - Đã hoàn thành khám bệnh.
  - `Delayed (-1)` - Trễ hẹn (hệ thống tự động tính toán dựa trên thời gian thực tế so với giờ hẹn).
* **Bộ lọc Đa tiêu chí:** Lọc tức thì theo ngày hẹn, trạng thái, bác sĩ phụ trách, chuyên khoa và thanh tìm kiếm từ khóa không dấu.

### 5.3. Quản lý Đội ngũ Bác sĩ (Doctor Management)
* **Hồ sơ Bác sĩ Chi tiết:**
  - Mã bác sĩ (`D_ID`), Họ và tên, Giới tính, Tuổi, Số điện thoại.
  - Chuyên khoa trực thuộc (Nội khoa, Ngoại khoa, Nhi khoa, Tai Mũi Họng, Răng Hàm Mặt, Mắt, Da liễu, Tim mạch,...).
  - Phòng khám chỉ định (`P101`, `P202`,...).
  - Mức phí khám chuyên khoa niêm yết (VNĐ).
  - Điểm đánh giá chất lượng (Rating từ 1.0 đến 5.0 sao).
  - Trạng thái làm việc (Đang làm việc / Nghỉ phép).
* **Nghiệp vụ Quản lý:**
  - Thêm mới bác sĩ với mã tự động tăng (`DOC001`, `DOC002`,...).
  - Chỉnh sửa thông tin hồ sơ, phòng làm việc, biểu phí.
  - Khóa/mở trạng thái hoạt động của bác sĩ.
  - Theo dõi danh sách các ca khám được phân công theo bác sĩ.

### 5.4. Quản lý Bệnh nhân & Hồ sơ Bệnh án (Patient Management)
* **Hồ sơ Nhân khẩu học:**
  - Mã bệnh nhân (`PAT001`, `PAT002`,...), Số CCCD/CMND định danh y tế.
  - Họ và tên, Tuổi, Giới tính, Số điện thoại liên hệ.
* **Liên kết Bệnh án Triệu chứng:**
  - Ghi nhận lịch sử triệu chứng của từng lần khám (`SymptomPatient`).
  - Phân loại trạng thái triệu chứng: Chưa xử lý (đang chờ khám) và Đã xử lý (sau khi hoàn thành cuộc hẹn).
  - Thống kê lịch sử khám bệnh và các lần tái khám của bệnh nhân.

### 5.5. Thống kê & Báo cáo Trực quan (Analytics & Charts)
* **Hệ thống Thẻ KPI Tổng quan:**
  - Tổng số bệnh nhân đăng ký trong hệ thống.
  - Tổng số bác sĩ đang công tác.
  - Tổng số lượt đặt lịch và số ca đang chờ khám.
  - Doanh thu dự kiến / thực tế (tổng hợp theo ngày, tháng).
  - Tỷ lệ hoàn thành lịch hẹn khám (`Completion Rate %`).
* **Đồ họa Trực quan với QtCharts:**
  - **Biểu đồ Cột (Bar Chart):** Xu hướng số lượng lịch hẹn khám trong 7 ngày gần nhất.
  - **Biểu đồ Tròn (Donut / Pie Chart):** Cơ cấu tỷ lệ bệnh nhân theo giới tính và các nhóm độ tuổi (Dưới 18, 18-45, 46-60, Trên 60).
  - **Bảng Xếp hạng (Leaderboard):** Top các bác sĩ tiêu biểu có số ca khám nhiều nhất và đánh giá cao nhất.
  - **Phân tích Nhu cầu Chuyên khoa:** Thống kê chuyên khoa có số lượng bệnh nhân quan tâm cao nhất để tối ưu hóa điều phối nhân lực.

### 5.6. Cấu hình Hệ thống & Quản lý Nghỉ Lễ (Config & Holidays)
* **Quản lý Danh mục Lịch Nghỉ Lễ:**
  - Quản lý danh sách ngày nghỉ theo lịch nhà nước (Tết Dương lịch, Giỗ tổ Hùng Vương, 30/4 - 1/5, Quốc khánh) và các ngày nghỉ đột xuất của phòng khám.
  - Thêm, sửa, xóa ngày nghỉ kèm ghi chú giải thích.
  - Hiển thị trực tiếp cảnh báo màu sắc trên giao diện lịch đặt hẹn.

---

## 6. QUY CHUẨN MA TRẬN PHÂN QUYỀN

Hệ thống triển khai cơ chế kiểm soát truy cập dựa trên vai trò kết hợp phân quyền động theo thời gian:

| Mã Quyền | Tên Quyền Hạn | Administrator | Receptionist | Special Receptionist |
| :---: | :--- | :---: | :---: | :---: |
| **-** | **Đăng nhập & Xem Dashboard** |  Có |  Có |  Có |
| **-** | **Đặt lịch khám & Tra cứu bệnh nhân** |  Có |  Có |  Có |
| **Per_1** | **Chỉnh sửa / Cập nhật Bác sĩ** |  Có | ❌ Không |  Khi được cấp |
| **Per_2** | **Chỉnh sửa / Xóa Bệnh nhân** |  Có | ❌ Không |  Khi được cấp |
| **Per_3** | **Xem Thống kê & Báo cáo Doanh thu** |  Có | ❌ Không |  Khi được cấp |
| **Per_4** | **Cấu hình Ngày nghỉ lễ & Quy định** |  Có | ❌ Không |  Khi được cấp |
| **Per_5** | **Quản lý Tài khoản & Phân quyền (RBAC)**|  Có | ❌ Không | ❌ Không |

> [!NOTE]
> Administrator có thể cấp các quyền phụ (`Per_1` đến `Per_4`) cho tài khoản Receptionist kèm **thời hạn hiệu lực (Expiration Date)**. Khi quá thời hạn quy định, quyền sẽ tự động thu hồi.

---

## 7. YÊU CẦU MÔI TRƯỜNG & HƯỚNG DẪN CÀI ĐẶT

### Yêu cầu phần mềm
* **Hệ điều hành:** Windows 10 / Windows 11 (64-bit).
* **Trình biên dịch:** MinGW-w64 GCC hỗ trợ chuẩn C++20/C++2b (khuyến nghị môi trường MSYS2 UCRT64).
* **Bộ thư viện:** Qt 6.8+ (các module: `Core`, `Gui`, `Widgets`, `Charts`, `OpenGL`, `OpenGLWidgets`).

### Các bước biên dịch từ mã nguồn

1. **Mở terminal MSYS2 UCRT64** hoặc Command Prompt đã thiết lập đường dẫn `PATH` tới Qt 6:
   ```cmd
   set PATH=C:\msys64\ucrt64\bin;%PATH%
   ```

2. **Di chuyển vào thư mục dự án:**
   ```cmd
   cd /d "d:\Documents\Project\pbl_2\pbl2_v1.1"
   ```

3. **Tạo Makefile bằng qmake:**
   ```cmd
   qmake PBL_2.pro
   ```

4. **Biên dịch mã nguồn song song (8 luồng):**
   ```cmd
   mingw32-make -j8
   ```
   *File thực thi lõi sẽ được tạo ra tại: `bin\AnTamMng.exe`.*

5. **(Khuyên dùng) Tự động build và đóng gói Portable hoàn chỉnh:**
   Nhấp đúp hoặc chạy script:
   ```cmd
   create_portable.bat
   ```
   *Script sẽ tự động biên dịch dự án, đồng bộ plugins/DLLs và biên dịch C Launcher độc lập `AnTamMng.exe` ở root.*

---

## 8. HƯỚNG DẪN KHỞI CHẠY ỨNG DỤNG

Người dùng có thể khởi chạy ứng dụng bằng một trong các phương thức tiện lợi sau:

1. **Cách 1 (Khuyên dùng):** Nhấp đúp chuột trực tiếp vào file **[`AnTamMng.exe`](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/AnTamMng.exe)** ở thư mục gốc. Đây là portable launcher C thuần (static runtime) không nhấp nháy console đen, tự động thiết lập môi trường cô lập, nạp Qt plugins và mở giao diện ứng dụng mượt mà trên **mọi máy tính Windows khác**.
2. **Cách 2:** Chạy script **[`create_portable.bat`](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/create_portable.bat)** nếu muốn tự động kiểm tra, đồng bộ thư viện và tái tạo launcher.
3. **Cách 3:** Chạy trực tiếp từ file nhị phân chính **[`bin/AnTamMng.exe`](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/bin/AnTamMng.exe)**.

---

## 9. TÀI KHOẢN MẪU TRẢI NGHIỆM

| Tài khoản (Username) | Mật khẩu (Password) | Vai trò (Role) | Mô tả quyền hạn |
| :--- | :--- | :--- | :--- |
| `admin` | `admin123` | **Administrator** | Toàn quyền quản trị hệ thống, quản lý tài khoản, báo cáo và cấu hình |
| `receptionist1` | `rec123` | **Receptionist** | Nhân viên tiếp đón, tra cứu bệnh nhân, sàng lọc triệu chứng & đặt lịch |

---

## 10. TÀI LIỆU THUẬT TOÁN CHUYÊN SÂU

Để xem phân tích kỹ thuật toàn diện về từng file mã nguồn, cấu trúc dữ liệu tự định nghĩa, độ phức tạp thuật toán và chi tiết hiện thực từng phương thức:
👉 Tham khảo file: [**`funtionAlgorithms.md`**](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/funtionAlgorithms.md) *(hoặc [`functionalAlgorithms.md`](file:///d:/Documents/Project/pbl_2/pbl2_v1.1/functionalAlgorithms.md))*.

---

* **Đơn vị phát triển:** Dự án PBL 2 - Đồ án Cơ sở Ngành Công nghệ Thông tin.
* **Bản quyền:** © 2026 Phòng Khám Đa Khoa An Tâm.
