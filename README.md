# HỆ THỐNG TIẾP NHẬN BỆNH NHÂN & QUẢN LÝ HÀNG ĐỢI PHÒNG KHÁM (PBL2)

Dự án môn học Lập trình hướng đối tượng / Đồ án cơ sở (PBL2) xây dựng trên nền tảng **C++23**, áp dụng kiến trúc hướng đối tượng (OOP), các cấu trúc dữ liệu tự định nghĩa (`LinkList`, `Queue`), và các kỹ thuật C++ hiện đại (`if constexpr`, `requires`, Template Specialization, Template Template Parameters).

---

## MỤC LỤC
1. [Cấu trúc thư mục dự án](#1-cấu-trúc-thư-mục-dự-án)
2. [Danh sách các đối tượng (Models & Data Structures)](#2-danh-sách-các-đối-tượng-models--data-structures)
3. [Danh sách các hàm và chức năng chính](#3-danh-sách-các-hàm-và-chức-năng-chính)
4. [Các kỹ thuật lập trình nâng cao](#4-các-kỹ-thuật-lập-trình-nâng-cao)
   - [Cơ chế `if constexpr` và C++20 `requires`](#41-cơ-chế-if-constexpr-và-c20-requires)
   - [Template Specialization & Template Template Parameter](#42-template-specialization--template-template-parameter)
   - [Hàng đợi vòng liên kết đôi bảo toàn lịch sử](#43-hàng-đợi-vòng-liên-kết-đôi-bảo-toàn-lịch-sử)
   - [Xử lý căn lề chính xác cho chuỗi đa byte UTF-8](#44-xử-lý-căn-lề-chính-xác-cho-chuỗi-đa-byte-utf-8)
5. [Quy trình hoạt động chính trong hệ thống](#5-quy-trình-hoạt-động-chính-trong-hệ-thống)
6. [Hướng dẫn biên dịch và chạy chương trình](#6-hướng-dẫn-biên-dịch-và-chạy-chương-trình)

---

## 1. CẤU TRÚC THƯ MỤC DỰ ÁN

```
pbl_2test/
├── Controllers/              # Cài đặt logic xử lý (.cpp)
│   ├── Appointment.cpp
│   ├── Date.cpp
│   ├── Doctor.cpp
│   ├── human.cpp
│   ├── libPBL2.cpp          # Thư viện xử lý File I/O, chuỗi, sinh ID, phân quyền
│   ├── Manager.cpp
│   ├── Patient.cpp
│   ├── QueuePatient.cpp
│   ├── Symptom.cpp
│   └── SymptomPatient.cpp
├── Data/                     # Dữ liệu dạng tệp văn bản (.txt)
│   ├── Appointment/
│   ├── Doctor/
│   ├── Symptom/
│   │   ├── Symptom.txt
│   │   └── SymptomPatient.txt
│   ├── Manager.txt
│   ├── Patient.txt
│   ├── PermissionManager.txt
│   └── QueuePatient.txt
├── Data_structures/          # Các cấu trúc dữ liệu tự cài đặt
│   ├── LinkList.h            # Danh sách liên kết đơn (có dummy header, footer)
│   └── Queue.h               # Hàng đợi liên kết đôi (firstWaiting, deQueue giữ lịch sử)
├── Models/                   # Định nghĩa các lớp và kiểu dữ liệu (.h)
│   ├── Appointment.h
│   ├── Date.h
│   ├── Doctor.h
│   ├── human.h
│   ├── libPBL2.h
│   ├── Manager.h
│   ├── Patient.h
│   ├── QueuePatient.h
│   ├── Symptom.h
│   └── SymptomPatient.h
├── main.cpp                  # Chương trình chính & menu tương tác console
└── README.md
```

---

## 2. DANH SÁCH CÁC ĐỐI TƯỢNG (MODELS & DATA STRUCTURES)

### 2.1. Các Lớp Thực Thể (Models)

| Lớp (Class) | Lớp cha (Kế thừa) | Các thuộc tính chính | Mục đích / Tác dụng |
| :--- | :--- | :--- | :--- |
| **`Human`** | *Gốc* | `id`, `fullName`, `phone`, `gender`, `age` | Lớp trừu tượng cơ sở quản lý thông tin nhân thân cơ bản. Cung cấp các getter/setter chuẩn. |
| **`Patient`** | `Human` | `CCCD` (Căn cước công dân) | Quản lý hồ sơ bệnh nhân. Hỗ trợ đọc/ghi file (`save`, `LoadFromFile`, `SaveToFile`), tự động sinh mã `P_xxx`. |
| **`Doctor`** | `Human` | `spId` (Chuyên khoa), `state`, `roomId`, `fee`, `rating` | Quản lý thông tin bác sĩ điều trị, chuyên khoa, phòng khám và trạng thái làm việc. |
| **`Manager`** | `Human` | `username`, `password`, `role` (`ManagerRole`) | Quản lý tài khoản đăng nhập hệ thống và phân cấp vai trò (`Admin`, `Staff`...). |
| **`Symptom`** | *Độc lập* | `S_id`, `sName`, `Sp_id` (Chuyên khoa), `sScore` (Điểm nặng) | Quản lý danh mục triệu chứng bệnh. Dùng để tra cứu (chỉ có getter và nạp từ file, không cho phép sửa đổi tùy tiện). |
| **`SymptomPatient`** | *Độc lập* | `QP_id` (Tham chiếu `QueuePatient`), `S_id` (Danh sách triệu chứng: `S_001, S_002...`), `P_id`, `date` (`Date`) | Lưu thông tin triệu chứng khám của bệnh nhân theo định dạng khối Java-like (`{ QP_id, S_id, P_id, Date },`). `QP_id` tham chiếu đến hàng đợi. |
| **`QueuePatient`** | *Độc lập* | `QP_id` (Khóa chính), `Sp_id` (Chuyên khoa ưu tiên), `state` (0: Chờ, 1: Đã khám) | Đại diện cho một bệnh nhân trong hàng đợi khám. `QP_id` đóng vai trò khóa chính (Primary Key). |
| **`Appointment`** | *Độc lập* | `Ap_id`, `P_id`, `D_id`, `Date`, `shift` | Quản lý thông tin lịch hẹn khám trước. |
| **`Date`** | *Độc lập* | `day`, `month`, `year` | Xử lý ngày tháng năm, ép kiểu từ chuỗi `YYYY-MM-DD`, xuất định dạng hiển thị `DD/MM/YYYY`, lấy ngày giờ hiện tại hệ thống (`Date::now()`). |

### 2.2. Các Cấu Trúc Dữ Liệu Tự Định Nghĩa (Data Structures)

#### 1. `LinkList<T>` (Danh sách liên kết đơn có nút giả)
- **Cấu trúc:** Sử dụng nút canh gác `header` và `footer` giúp loại bỏ hoàn toàn các trường hợp biên (danh sách rỗng, thêm ở đầu, thêm ở cuối).
- **Phương thức chính:**
  - `insert(T info)`: Thêm một phần tử vào cuối danh sách trước `footer`.
  - `getHeader()`, `getFooter()`: Truy xuất nút đầu và nút cuối để duyệt danh sách tuần tự.

#### 2. `Queue<T>` (Hàng đợi liên kết đôi với con trỏ `firstWaiting`)
- **Cấu trúc:**
  - Nút đôi `Node { Node *next; Node *prev; T *data; }`.
  - Có nút canh gác `header` và `footer`.
  - Con trỏ đặc biệt **`firstWaiting`**: Trỏ trực tiếp đến nút đầu tiên trong hàng đợi có bệnh nhân đang ở trạng thái chờ khám (`State == 0`).
- **Phương thức chính:**
  - `enQueue(T val)`: Thêm phần tử mới vào cuối hàng đợi (ngay trước `footer`). Tự động cập nhật `firstWaiting` nếu trước đó chưa có ai chờ.
  - `deQueue()`: Gọi khám bệnh nhân theo thứ tự FIFO. Với kiểu dữ liệu có `getState()`, hàm **không giải phóng node** mà chỉ chuyển `State: 0 -> 1` và dịch `firstWaiting` sang bệnh nhân kế tiếp, trả về con trỏ bản sao dữ liệu.
  - `updateFirstWaiting()`: Quét lại từ đầu hàng đợi để đồng bộ chính xác con trỏ `firstWaiting`.
  - `isEmpty()`: Kiểm tra hàng đợi rỗng.

---

## 3. DANH SÁCH CÁC HÀM VÀ CHỨC NĂNG CHÍNH

### 3.1. Thư viện hệ thống `libPBL2` (`Controllers/libPBL2.cpp`)

| Tên hàm | Kiểu trả về | Tham số | Chức năng |
| :--- | :--- | :--- | :--- |
| `trim` | `void` | `string &str` | Cắt bỏ khoảng trắng thừa ở cả hai đầu chuỗi ký tự. |
| `filterNewline` | `void` | `string &str` | Loại bỏ các ký tự điều khiển xuống dòng `\r`, `\n` còn sót lại. |
| `isAllowed` | `bool` | `const Manager &M, PermissionType Per, const LinkList<Permission> *list` | Kiểm tra tài khoản người dùng có quyền thực hiện thao tác hay không. |
| `LoadFromFile<T, U>` | `U<T>*` | `const string &path` | Đọc dữ liệu từ file văn bản và nạp vào cấu trúc dữ liệu (`LinkList` hoặc `Queue`). |
| `SaveToFile<T, U>` | `bool` | `const string &path, const U<T> *container` | Lưu toàn bộ nội dung từ cấu trúc dữ liệu ra file văn bản chuẩn hóa. |
| `generateNext_id<T>` | `string` | `const LinkList<T> *list` | Tự động quét ID lớn nhất hiện có và sinh mã tiếp theo (`P_%03d` cho `Patient`, `QP_%03d` cho `QueuePatient`/`SymptomPatient`). |
| `utf8Length` | `size_t` | `const string &str` | Đếm số lượng ký tự hiển thị thực tế trong chuỗi UTF-8 (bỏ qua các continuation byte, đếm theo Unicode code point). |
| `padRightUtf8` | `string` | `const string &str, size_t targetWidth` | Căn lề trái và đệm khoảng trắng chính xác theo số ký tự hiển thị UTF-8, khắc phục hoàn toàn lỗi lệch cột bảng do ký tự tiếng Việt có dấu. |

### 3.2. Các hàm nghiệp vụ trong `main.cpp`

| Tên hàm | Chức năng chi tiết |
| :--- | :--- |
| **`nhapThongTinBenhNhan`** | Hỗ trợ 2 chế độ: **(1)** Tìm kiếm bệnh nhân cũ theo Mã BN (`P_xxx`), SĐT, hoặc CCCD; **(2)** Tiếp nhận bệnh nhân mới: tự động sinh mã `P_xxx`, nhập thông tin nhân thân và thêm vào `patientList`. |
| **`tiepNhanBenhNhan`** | Hiển thị bảng triệu chứng có thang điểm và mã khoa; cho phép chọn nhiều triệu chứng; tính toán khoa có điểm số cao nhất; ghi nhận vào `spList`; tự động đóng gói đối tượng `QueuePatient` từ `SymptomPatient` và đưa vào hàng đợi `queuePatient`. |
| **`hienThiTrangThaiQueue`** | Báo cáo tổng số bản ghi trong Queue, số bệnh nhân đang chờ (`State = 0`), và thông tin chi tiết tại vị trí con trỏ `firstWaiting`. |
| **`inDanhSachChoKham`** | Duyệt hàng đợi bắt đầu từ `firstWaiting` theo thứ tự FIFO chỉ hiển thị những bệnh nhân đang chờ khám. |
| **`inToanBoQueue`** | Hiển thị toàn bộ lịch sử hàng đợi gồm cả người đã khám (`State = 1`) và người đang chờ (`State = 0`). |
| **`testDeQueue`** | Thực hiện gọi khám: Chuyển `State` của bệnh nhân từ `0` sang `1`, giữ nguyên vị trí trong Queue để lưu vết lịch sử, và dịch chuyển `firstWaiting` đến bệnh nhân chờ kế tiếp. |
| **`testEnQueue`** | Thêm thủ công một lượt khám vào hàng đợi để kiểm tra tính linh hoạt của Queue. |

---

## 4. CÁC KỸ THUẬT LẬP TRÌNH NÂNG CAO

### 4.1. Cơ chế `if constexpr` và C++20 `requires`

#### Vấn đề đặt ra:
Template class `Queue<T>` được thiết kế tổng quát cho mọi kiểu dữ liệu `T`. Tuy nhiên, với bài toán phòng khám:
- Nếu `T` là `QueuePatient`: Khi `deQueue()`, ta muốn **giữ lại node** trong hàng đợi để làm lịch sử, chỉ đổi `State` từ `0` thành `1` (thông qua phương thức `getState()` và `setState(1)`).
- Nếu `T` là một kiểu dữ liệu thông thường (ví dụ: `int`, `string`... không có hàm `getState()`): Mã nguồn không được phép gọi `obj.getState()`, nếu không trình biên dịch sẽ báo lỗi cú pháp.

#### Giải pháp C++23: Kết hợp `if constexpr` và biểu thức `requires`:
`if constexpr` thực hiện đánh giá điều kiện tại **thời điểm biên dịch** (Compile-time) thay vì lúc chạy (Runtime). Trình biên dịch sẽ **loại bỏ hoàn toàn** nhánh không thỏa mãn, không sinh mã máy cho nhánh đó.

```cpp
template <class T>
T* Queue<T>::deQueue() {
    if (isEmpty()) return nullptr;

    // Kiểm tra xem kiểu T có cung cấp phương thức getState() hay không tại thời điểm biên dịch
    if constexpr (requires(T obj) { { obj.getState() } -> std::convertible_to<int>; }) {
        // NHÁNH 1: Dành riêng cho QueuePatient (hoặc bất kỳ kiểu nào có State)
        Node *target = firstWaiting;
        if (target == nullptr) {
            return nullptr;
        }

        // Cập nhật State thành 1 (đã khám), giữ nguyên node trong hàng đợi
        target->data->setState(1);
        T *result = new T(*(target->data));

        // Dịch chuyển firstWaiting đến node kế tiếp có State == 0
        Node *nextWait = target->next;
        while (nextWait != footer && nextWait != nullptr) {
            if (nextWait->data && nextWait->data->getState() == 0) {
                break;
            }
            nextWait = nextWait->next;
        }

        if (nextWait != footer && nextWait != nullptr) {
            firstWaiting = nextWait;
        } else {
            firstWaiting = nullptr;
        }

        return result;
    } else {
        // NHÁNH 2: Dành cho các kiểu dữ liệu T thông thường không có getState()
        // Hủy node đầu tiên theo chuẩn FIFO cổ điển
        Node *target = header->next;
        header->next = target->next;
        target->next->prev = header;

        T *result = target->data;
        delete target;
        return result;
    }
}
```

* **Lợi ích:**
  1. **An toàn kiểu tuyệt đối (Type-safety):** Không xảy ra lỗi biên dịch khi tái sử dụng `Queue<int>` hay `Queue<string>`.
  2. **Hiệu năng tối đa (Zero-cost abstraction):** Không tốn chi phí kiểm tra điều kiện lúc chương trình đang chạy.

---

### 4.2. Template Specialization & Template Template Parameter

#### 1. Template Template Parameter:
Khai báo hàm đọc/ghi file có thể hoạt động linh hoạt với bất kỳ cấu trúc dữ liệu nào (`LinkList` hoặc `Queue`) nhận cùng kiểu dữ liệu `T`:
```cpp
template <typename T, template <typename> class U = LinkList>
U<T>* LoadFromFile(const std::string &path);

template <typename T, template <typename> class U = LinkList>
bool SaveToFile(const std::string &path, const U<T> *container);
```

#### 2. Template Specialization (Chuyên biệt hóa mẫu):
Đối với mỗi kiểu dữ liệu cụ thể (`Patient`, `SymptomPatient`, `QueuePatient`), cách định dạng tệp và phân tách chuỗi là khác nhau:
```cpp
template <>
LinkList<Patient>* LoadFromFile<Patient, LinkList>(const std::string &path);

template <>
bool SaveToFile<Patient, LinkList>(const std::string &path, const LinkList<Patient> *list);

template <>
Queue<QueuePatient>* LoadFromFile<QueuePatient, Queue>(const std::string &path);

template <>
bool SaveToFile<QueuePatient, Queue>(const std::string &path, const Queue<QueuePatient> *queue);
```

Mỗi lớp thực thể cung cấp một phương thức `save(std::ofstream &f) const`, và `SaveToFile` chỉ cần duyệt qua các node để gọi phương thức này, đảm bảo tính đóng gói (encapsulation) chuẩn OOP.

---

### 4.3. Hàng đợi vòng liên kết đôi bảo toàn lịch sử

- **Khởi tạo:** `header` và `footer` trỏ lẫn nhau.
- **`firstWaiting`:** Luôn theo dõi vị trí người tiếp theo cần phục vụ.
- **Lợi ích:**
  - Không cần tạo thêm một danh sách riêng để lưu lịch sử khám bệnh.
  - Vẫn duyệt được danh sách chờ với độ phức tạp $O(1)$ để lấy người tiếp theo, và duyệt lịch sử theo thứ tự tiếp nhận ban đầu.

---

### 4.4. Xử lý căn lề chính xác cho chuỗi đa byte UTF-8 (Tiếng Việt có dấu)

#### Vấn đề:
Khi định dạng xuất bảng trong C++ bằng `std::setw(width)`, hàm này chỉ đếm **số lượng byte** (`char`), chứ **không đếm số lượng ký tự hiển thị thực tế**.
- Trong bảng mã UTF-8, các ký tự tiếng Việt có dấu (`á`, `à`, `ễ`, `Đ`...) chiếm từ **2 đến 3 byte**, nhưng trên màn hình console hoặc tệp văn bản chỉ chiếm **1 độ rộng ký tự** (1 display column).
- Ví dụ: Họ tên `"Nguyễn Văn An"` có 13 ký tự hiển thị nhưng lại chiếm tới 16 byte. Nếu dùng `std::setw(23)`, trình biên dịch tính chuỗi đã dài 16 byte nên chỉ chèn thêm $23 - 16 = 7$ khoảng trắng thay vì đúng $23 - 13 = 10$ khoảng trắng.
- Hậu quả: Các cột dữ liệu chứa ký tự tiếng Việt (như `FullName` trong `Data/Patient.txt` hoặc tên triệu chứng trong console) bị thụt thò, xô lệch dấu `|`.

#### Giải pháp trong `libPBL2`:
Xây dựng 2 hàm chuyên biệt `utf8Length` và `padRightUtf8`:

1. **`libPBL2::utf8Length(const string &str)`**:
   Dựa trên nguyên lý mã hóa UTF-8, mỗi ký tự Unicode bắt đầu bằng một byte không phải là *continuation byte*. Một byte là continuation byte khi và chỉ khi 2 bit đầu là `10` (tức `(c & 0xC0) == 0x80`).
   Hàm chỉ cần đếm các byte **không phải** continuation byte để tính chính xác số ký tự hiển thị:
   ```cpp
   size_t utf8Length(const string &str) {
       size_t len = 0;
       for (unsigned char c : str) {
           if ((c & 0xC0) != 0x80) {
               len++;
           }
       }
       return len;
   }
   ```

2. **`libPBL2::padRightUtf8(const string &str, size_t targetWidth)`**:
   Tính số lượng khoảng trắng cần bù dựa trên `utf8Length(str)`:
   ```cpp
   string padRightUtf8(const string &str, size_t targetWidth) {
       size_t ulen = utf8Length(str);
       if (ulen >= targetWidth) return str;
       return str + string(targetWidth - ulen, ' ');
   }
   ```

Khi gọi hàm `Patient::save(ofstream &f)`:
```cpp
void Patient::save(std::ofstream &f) const {
    f << libPBL2::padRightUtf8(id, 5) << " | "
      << libPBL2::padRightUtf8(fullName, 23) << " | "
      << libPBL2::padRightUtf8(phone, 10) << " | "
      << libPBL2::padRightUtf8(gender ? "1" : "0", 8) << " | "
      << libPBL2::padRightUtf8(to_string(age), 5) << " | "
      << CCCD << "\n";
}
```
Mọi dòng trong `Data/Patient.txt` và hiển thị console đều được căn chỉnh thẳng tắp 100%, độc lập với số lượng dấu thanh tiếng Việt trong chuỗi.

---

## 5. QUY TRÌNH HOẠT ĐỘNG CHÍNH TRONG HỆ THỐNG

```
[1. Khởi động] ──> Tải dữ liệu từ các file .txt vào LinkList & Queue
       │
[2. Đăng nhập] ──> Xác thực Manager (Admin: HoangMA001 / HoangMA001)
       │
[3. Tiếp nhận BN & Triệu chứng] (Menu 5)
       ├─ Hồ sơ: BN Cũ (tìm theo Mã BN / SĐT / CCCD) HOẶC BN Mới (tự sinh P_xxx)
       ├─ Triệu chứng & Khám: Tự sinh QP_xxx ──> Chọn triệu chứng ──> Lưu vào spList
       └─ Tự động vào Queue: Phân tích chuyên khoa ưu tiên (Sp_id) ──> Khởi tạo QueuePatient (QP_id) ──> enQueue(State = 0)
       │
[4. Quản lý Queue & Khám bệnh]
       ├─ [1] Xem trạng thái Queue & firstWaiting
       ├─ [2] Danh sách bệnh nhân đang chờ (FIFO từ firstWaiting)
       ├─ [3] Xem toàn bộ bản ghi Queue (Lịch sử + Đang chờ)
       ├─ [4] Gọi khám bệnh nhân (deQueue: chuyển State 0 -> 1, dịch firstWaiting)
       └─ [7] Quét và cập nhật lại con trỏ firstWaiting
       │
[5. Lưu dữ liệu] (Menu 6 hoặc trước khi Thoát Menu 0)
       ├─ Ghi Data/Patient.txt
       ├─ Ghi Data/Symptom/SymptomPatient.txt
       └─ Ghi Data/QueuePatient.txt
```

---

## 6. HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY CHƯƠNG TRÌNH

### Yêu cầu môi trường:
- Trình biên dịch: GCC / MinGW-w64 hỗ trợ chuẩn **C++23**.
- Hệ điều hành: Windows (chạy console hỗ trợ UTF-8 `SetConsoleOutputCP(65001)`).

### Lệnh biên dịch (Compile & Link):
Mở terminal tại thư mục gốc của dự án (`pbl_2test/`) và thực hiện:

```bash
# 1. Biên dịch các module controller thành file đối tượng (.o)
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/Appointment.cpp -o Controllers/Appointment.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/Date.cpp -o Controllers/Date.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/Doctor.cpp -o Controllers/Doctor.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/human.cpp -o Controllers/human.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/libPBL2.cpp -o Controllers/libPBL2.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/Manager.cpp -o Controllers/Manager.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/Patient.cpp -o Controllers/Patient.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/QueuePatient.cpp -o Controllers/QueuePatient.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/Symptom.cpp -o Controllers/Symptom.o
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c Controllers/SymptomPatient.cpp -o Controllers/SymptomPatient.o

# 2. Biên dịch hàm main
g++ -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures -c main.cpp -o main.o

# 3. Liên kết tạo chương trình thực thi (main.exe)
g++ main.o Controllers/Appointment.o Controllers/Date.o Controllers/Doctor.o Controllers/human.o Controllers/libPBL2.o Controllers/Manager.o Controllers/Patient.o Controllers/QueuePatient.o Controllers/Symptom.o Controllers/SymptomPatient.o -o main.exe -mconsole
```

### Chạy chương trình:
```bash
./main.exe
```

* **Tài khoản đăng nhập quản trị mẫu:**
  - **Tên đăng nhập (Username):** `HoangMA001`
  - **Mật khẩu (Password):** `HoangMA001`
  - *(Nhập `0` tại màn hình đăng nhập nếu muốn dừng hệ thống)*.

