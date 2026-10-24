#define NOMINMAX
#include <QApplication>
#include <QMainWindow>
#include <QDialog>
#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QMessageBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QLocale>
#include <QStringList>
#include <QGraphicsDropShadowEffect>
#include <QFontDatabase>
#include <QPainter>
#include <QIcon>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <tuple>
#include <set>
#include <utility>

#include "Appointment.h"
#include "Doctor.h"
#include "Manager.h"
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "Permission.h"
#include "libPBL2.h"
#include "MainWindow.h"

using namespace std;

// Tiện ích chuyển đổi chuỗi UTF-8 chuẩn
static inline QString qs(const string &s) { return QString::fromUtf8(s.c_str()); }
static inline string ss(const QString &s) { return s.toUtf8().toStdString(); }

// Đường dẫn tệp dữ liệu chuẩn
static const string patientPath = "Data/Patient.txt";
static const string managerPath = "Data/Manager.txt";
static const string doctorPath = "Data/Doctor/Doctor.txt";
static const string appointmentPath = "Data/Appointment/Appointment.txt";
static const string symptomPatientPath = "Data/Symptom/SymptomPatient.txt";
static const string symptomPath = "Data/Symptom/Symptom.txt";
static const string specializationPath = "Data/Doctor/Specialization.txt";
static const string holidayConfigPath = "Data/Config/Lichnghile.txt";
static const string managerPermissionPath = "Data/Config/ManagerPermission.txt";

static QString roleName(ManagerRole role) {
    switch (role) {
        case ManagerRole::Administrator: return QStringLiteral("Quản trị viên");
        case ManagerRole::Receptionist: return QStringLiteral("Lễ tân");
        case ManagerRole::SpecialReceptionist: return QStringLiteral("Lễ tân chuyên trách");
        default: return QStringLiteral("Chưa phân vai trò");
    }
}

static const vector<pair<string, string>> permissionDefinitions = {
    {"Per_1", "Cấp quyền tạm thời làm admin có thời hạn"},
    {"Per_2", "Chỉnh sửa thông tin bác sĩ"},
    {"Per_3", "Chỉnh sửa thông tin bệnh nhân"},
    {"Per_4", "Chỉnh sửa thông tin lịch hẹn khám bệnh"},
    {"Per_5", "Chỉnh sửa lịch nghỉ"}
};

// Tạo avatar hình tròn bo góc với chữ cái đầu của tên
static QPixmap createAvatar(const QString &fullName, int size = 38, const QColor &bgColor = QColor("#0d9488"), const QColor &textColor = Qt::white) {
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    p.setBrush(QBrush(bgColor));
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, size, size);

    QString initial = "U";
    QStringList parts = fullName.trimmed().split(' ', Qt::SkipEmptyParts);
    if (!parts.isEmpty()) {
        initial = parts.last().left(1).toUpper();
    }
    p.setPen(textColor);
    QFont f("Be Vietnam Pro", size / 2.6, QFont::Bold);
    p.setFont(f);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, initial);
    return pix;
}

class ClinicWindow : public QMainWindow {
private:
    LinkList<Manager> *managers = nullptr;
    LinkList<Patient> *patients = nullptr;
    LinkList<Doctor> *doctors = nullptr;
    LinkList<Symptom> *symptoms = nullptr;
    LinkList<SymptomPatient> *symptomPatients = nullptr;
    LinkList<Appointment> *appointments = nullptr;

    Manager *currentManager = nullptr;
    QWidget *root = nullptr;
    QStackedWidget *pages = nullptr;
    QWidget *sidebar = nullptr;
    QList<QPushButton*> navButtons;
    QTableWidget *managerTable = nullptr;
    int activeNav = 0;
    int dashboardAppPage = 0;
    const int DASHBOARD_PAGE_SIZE = 8;

    // --- KIỂM TRA PHÂN QUYỀN ---
    bool isRootAdmin() const {
        return currentManager && currentManager->getUsername() == "HoangMA001";
    }

    QDateTime permissionExpiry(const string &managerId, const string &permissionId) const {
        ifstream input(managerPermissionPath);
        string line;
        while (getline(input, line)) {
            stringstream row(line);
            string id, permission, expiry;
            if (!getline(row, id, '|') || !getline(row, permission, '|')) continue;
            getline(row, expiry);
            libPBL2::trim(id);
            libPBL2::trim(permission);
            libPBL2::trim(expiry);
            if (id == managerId && permission == permissionId && !expiry.empty()) {
                QDateTime parsed = QDateTime::fromString(qs(expiry), Qt::ISODate);
                if (!parsed.isValid()) parsed = QDateTime::fromString(qs(expiry), "yyyy-MM-dd HH:mm:ss");
                return parsed;
            }
        }
        return QDateTime();
    }

    bool isTemporaryAdmin() const {
        if (!currentManager || isRootAdmin()) return false;
        QDateTime expiry = permissionExpiry(currentManager->getID(), "Per_1");
        return expiry.isValid() && expiry > QDateTime::currentDateTime();
    }

    bool isAdministrator() const {
        return isRootAdmin() || isTemporaryAdmin();
    }

    bool hasPermission(const string &permissionId) const {
        if (!currentManager) return false;
        if (isRootAdmin() || isTemporaryAdmin()) return true;
        if (permissionId == "Per_1") return false;

        ifstream input(managerPermissionPath);
        string line;
        while (getline(input, line)) {
            stringstream row(line);
            string id, permission;
            if (!getline(row, id, '|') || !getline(row, permission, '|')) continue;
            libPBL2::trim(id);
            libPBL2::trim(permission);
            if (id == currentManager->getID() && permission == permissionId) return true;
        }
        return false;
    }

    // --- TIỆN ÍCH DỮ LIỆU ---
    QString specialty(const string &id) const {
        ifstream file(specializationPath);
        string line;
        getline(file, line);
        getline(file, line);
        while (getline(file, line)) {
            string key, value;
            stringstream row(line);
            if (getline(row, key, '|') && getline(row, value)) {
                libPBL2::trim(key);
                libPBL2::trim(value);
                if (key == id) return qs(value);
            }
        }
        return qs(id);
    }

    template<class T>
    int countList(const LinkList<T> *list) const {
        return list ? list->count : 0;
    }

    int countPatients() const { return patients ? patients->count : 0; }
    int countDoctors() const { return doctors ? doctors->count : 0; }
    int countAppointments() const { return appointments ? appointments->count : 0; }

    int pendingAppointments() const {
        int count = 0;
        if (!appointments) return 0;
        auto *n = appointments->getHeader()->next;
        while (n && n->next) {
            if (n->info.getState() == 0) ++count;
            n = n->next;
        }
        return count;
    }

    Patient *findPatient(const string &id) const {
        if (!patients) return nullptr;
        auto *n = patients->getHeader()->next;
        while (n && n->next) {
            if (n->info.getID() == id) return &n->info;
            n = n->next;
        }
        return nullptr;
    }

    Doctor *findDoctor(const string &id) const {
        if (!doctors) return nullptr;
        auto *n = doctors->getHeader()->next;
        while (n && n->next) {
            if (n->info.getID() == id) return &n->info;
            n = n->next;
        }
        return nullptr;
    }

    bool isHoliday(const QDate &date) const {
        ifstream file(holidayConfigPath);
        string line;
        while (getline(file, line)) {
            libPBL2::trim(line);
            if (line.empty()) continue;
            const auto separator = line.find('-');
            string value = line.substr(0, separator);
            libPBL2::trim(value);
            if (value.find('/') == string::npos) continue;
            stringstream parsed(value);
            int day = 0, month = 0;
            char slash = 0;
            if (parsed >> day >> slash >> month && slash == '/' && day == date.day() && month == date.month()) {
                return true;
            }
        }
        return false;
    }

    QString patientSymptoms(const string &patientId) const {
        QStringList names;
        if (!symptomPatients) return "Chưa ghi nhận";
        auto *record = symptomPatients->getHeader()->next;
        while (record && record->next) {
            if (record->info.getP_id() == patientId) {
                stringstream ids(record->info.getS_id());
                string id;
                while (getline(ids, id, ',')) {
                    libPBL2::trim(id);
                    if (symptoms) {
                        auto *s = symptoms->getHeader()->next;
                        while (s && s->next) {
                            if (s->info.getS_id() == id) {
                                names << qs(s->info.getSName());
                                break;
                            }
                            s = s->next;
                        }
                    }
                }
            }
            record = record->next;
        }
        names.removeDuplicates();
        return names.isEmpty() ? QStringLiteral("Chưa ghi nhận") : names.join(QStringLiteral(", "));
    }

    QString unresolvedPatientSymptoms(const string &patientId) const {
        QStringList names;
        if (!symptomPatients) return "Đã khám hết";
        auto *record = symptomPatients->getHeader()->next;
        while (record && record->next) {
            if (record->info.getP_id() == patientId) {
                stringstream ids(record->info.getS_id());
                string id;
                while (getline(ids, id, ',')) {
                    libPBL2::trim(id);
                    bool hasSpecificAppointment = false, specificPending = false;
                    bool hasLegacyAppointment = false, legacyPending = false;

                    if (appointments) {
                        auto *a = appointments->getHeader()->next;
                        while (a && a->next) {
                            if (a->info.getPatientId() == patientId) {
                                if (a->info.getSymptomId() == id) {
                                    hasSpecificAppointment = true;
                                    if (!a->info.getState()) specificPending = true;
                                } else if (a->info.getSymptomId().empty()) {
                                    hasLegacyAppointment = true;
                                    if (!a->info.getState()) legacyPending = true;
                                }
                            }
                            a = a->next;
                        }
                    }

                    if ((hasSpecificAppointment && !specificPending) ||
                        (!hasSpecificAppointment && hasLegacyAppointment && !legacyPending)) {
                        continue;
                    }
                    if (!hasSpecificAppointment && !hasLegacyAppointment && record->info.isResolved()) {
                        continue;
                    }

                    if (symptoms) {
                        auto *s = symptoms->getHeader()->next;
                        while (s && s->next) {
                            if (s->info.getS_id() == id) {
                                names << qs(s->info.getSName());
                                break;
                            }
                            s = s->next;
                        }
                    }
                }
            }
            record = record->next;
        }
        names.removeDuplicates();
        return names.isEmpty() ? QStringLiteral("Đã khám hết") : QStringLiteral("Chưa khám: %1").arg(names.join(QStringLiteral(", ")));
    }

    void updatePatientSymptomStatus(const string &patientId) {
        if (!appointments || !symptomPatients) return;
        bool foundAppointment = false, hasPendingAppointment = false, hasSpecificAppointment = false;
        auto *appointment = appointments->getHeader()->next;
        while (appointment && appointment->next) {
            if (appointment->info.getPatientId() == patientId) {
                foundAppointment = true;
                if (!appointment->info.getSymptomId().empty()) hasSpecificAppointment = true;
                if (!appointment->info.getState()) hasPendingAppointment = true;
            }
            appointment = appointment->next;
        }
        if (hasSpecificAppointment) return;
        const bool resolved = foundAppointment && !hasPendingAppointment;
        auto *record = symptomPatients->getHeader()->next;
        while (record && record->next) {
            if (record->info.getP_id() == patientId) record->info.setResolved(resolved);
            record = record->next;
        }
    }

    string nextAppointmentId() const {
        int maxId = 0;
        if (appointments) {
            auto *n = appointments->getHeader()->next;
            while (n && n->next) {
                string id = n->info.getId();
                if (id.rfind("A_", 0) == 0) {
                    try { maxId = max(maxId, stoi(id.substr(2))); } catch (...) {}
                }
                n = n->next;
            }
        }
        ostringstream out;
        out << "A_" << setfill('0') << setw(3) << maxId + 1;
        return out.str();
    }

    // --- STYLESHEET ---
    QString style() const {
        return QStringLiteral(R"(
            QWidget { font-family: 'Be Vietnam Pro', 'Segoe UI', sans-serif; font-size: 14px; color: #183044; }
            QMainWindow, QWidget#appRoot { background: #f4f7fb; }
            QLabel#muted { color: #8091a4; }
            QLabel#pageTitle { font-size: 26px; font-weight: 750; color: #17334a; }
            QLabel#pageSubtitle { font-size: 13.5px; color: #64748b; }
            QFrame#sidebar { background: #102f43; border: 0; }
            QLabel#brand { color: white; font-size: 20px; font-weight: 750; letter-spacing: 0.5px; }
            QLabel#brandSub { color: #9db7c6; font-size: 11px; font-weight: 600; letter-spacing: 1px; }
            QPushButton#nav { color: #bfd0d9; text-align: left; padding: 12px 16px; border: 0; border-radius: 9px; background: transparent; font-size: 14px; font-weight: 500; }
            QPushButton#nav:hover { color: white; background: #1b465d; }
            QPushButton#nav[active="true"] { color: white; background: #167d79; font-weight: 650; }
            QPushButton#primary { background: #167d79; color: white; border: 0; border-radius: 9px; padding: 11px 20px; font-weight: 650; font-size: 14px; }
            QPushButton#primary:hover { background: #106d69; }
            QPushButton#secondary { background: white; color: #355166; border: 1px solid #dce5eb; border-radius: 9px; padding: 10px 16px; font-weight: 500; }
            QPushButton#danger { background: #fff0ef; color: #b94b45; border: 0; border-radius: 8px; padding: 9px 14px; font-weight: 600; }
            QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateTimeEdit, QDateEdit { background: white; border: 1px solid #d8e2e9; border-radius: 8px; padding: 9px 12px; min-height: 20px; selection-background-color: #167d79; }
            QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateTimeEdit:focus, QDateEdit:focus { border: 1px solid #167d79; }
            QTableWidget { background: white; border: 1px solid #e2e8f0; border-radius: 12px; gridline-color: transparent; selection-background-color: #f1f5f9; selection-color: #17334a; }
            QHeaderView::section { background: #f8fafc; color: #475569; border: 0; border-bottom: 2px solid #e2e8f0; padding: 12px 20px; font-size: 12px; font-weight: 700; letter-spacing: 0.5px; }
            QTableWidget::item { padding: 4px 8px; font-size: 13.5px; color: #1e293b; border-bottom: 1px solid #f1f5f9; }
            QTableWidget::item:hover { background-color: #f8fafc; }
            QFrame#card, QFrame#panel { background: white; border: 1px solid #e2e8f0; border-radius: 14px; }
            QLabel#metricValue { font-size: 32px; font-weight: 800; }
            QLabel#metricLabel { color: #64748b; font-size: 12px; font-weight: 700; letter-spacing: 0.5px; }
            QLabel#pill { background: #e6f5f2; color: #14756e; border-radius: 10px; padding: 5px 9px; font-size: 11px; font-weight: 700; }
            QCheckBox { spacing: 9px; }
        )");
    }

    // --- UI FACTORY ---
    QWidget *page(const QString &title, const QString &subtitle, QWidget *parent = nullptr) {
        QWidget *container = new QWidget(parent);
        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->setContentsMargins(32, 26, 32, 26);
        layout->setSpacing(20);

        QLabel *heading = new QLabel(title);
        heading->setObjectName("pageTitle");
        QLabel *description = new QLabel(subtitle);
        description->setObjectName("pageSubtitle");

        layout->addWidget(heading);
        layout->addWidget(description);
        return container;
    }

    QFrame *panel() {
        QFrame *frame = new QFrame;
        frame->setObjectName("panel");
        auto *shadow = new QGraphicsDropShadowEffect(frame);
        shadow->setBlurRadius(18);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(20, 48, 65, 12));
        frame->setGraphicsEffect(shadow);
        return frame;
    }

    QPushButton *button(const QString &text, const QString &kind = "primary") {
        auto *b = new QPushButton(text);
        b->setObjectName(kind);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    }

    QTableWidget *table(const QStringList &headers) {
        auto *t = new QTableWidget;
        t->setColumnCount(headers.size());
        t->setHorizontalHeaderLabels(headers);
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        t->verticalHeader()->hide();
        t->setAlternatingRowColors(false);
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setShowGrid(false);
        return t;
    }

    void cell(QTableWidget *t, int row, int column, const QString &text, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter) {
        auto *item = new QTableWidgetItem(text);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setTextAlignment(align);
        t->setItem(row, column, item);
    }

    void setPage(int index) {
        activeNav = index;
        if (pages) pages->setCurrentIndex(index);
        updateNavigation();
    }

    void updateNavigation() {
        for (int i = 0; i < navButtons.size(); ++i) {
            navButtons[i]->setProperty("active", i == activeNav);
            navButtons[i]->style()->unpolish(navButtons[i]);
            navButtons[i]->style()->polish(navButtons[i]);
        }
    }

    // --- SHELL GIAO DIỆN CHÍNH ---
    void setupShell() {
        delete root;
        root = new QWidget;
        root->setObjectName("appRoot");
        setCentralWidget(root);

        auto *outer = new QHBoxLayout(root);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->setSpacing(0);

        sidebar = new QFrame;
        sidebar->setObjectName("sidebar");
        sidebar->setFixedWidth(238);

        auto *side = new QVBoxLayout(sidebar);
        side->setContentsMargins(18, 24, 18, 20);
        side->setSpacing(8);

        // Giữ nguyên tiêu đề thương hiệu gốc ✦ AN TÂM
        auto *brand = new QLabel("✦  AN TÂM");
        brand->setObjectName("brand");
        auto *brandSub = new QLabel("PHÒNG KHÁM ĐA KHOA");
        brandSub->setObjectName("brandSub");
        side->addWidget(brand);
        side->addWidget(brandSub);
        side->addSpacing(27);

        navButtons.clear();
        const QStringList labels = {
            "⌂   Tổng quan",
            "＋   Đặt lịch khám",
            "♡   Bệnh nhân",
            "✚   Bác sĩ",
            "▤   Quản lý lịch khám",
            "▧   Lịch nghỉ"
        };
        for (int i = 0; i < labels.size(); ++i) {
            auto *b = button(labels[i], "nav");
            navButtons.push_back(b);
            connect(b, &QPushButton::clicked, this, [this, i]{
                refreshPage(i);
                setPage(i);
            });
            side->addWidget(b);
        }

        if (isAdministrator()) {
            auto *b = button("♙   Quản lý tài khoản", "nav");
            navButtons.push_back(b);
            connect(b, &QPushButton::clicked, this, [this]{
                refreshManagers();
                setPage(6);
            });
            side->addWidget(b);
        }

        side->addStretch();

        // Khung tài khoản góc dưới bên trái (Khóa login: duy trì phiên làm việc Quản trị viên)
        QFrame *userCard = new QFrame;
        userCard->setObjectName("userCard");
        userCard->setStyleSheet(R"(
            QFrame#userCard {
                background: #14384e;
                border: 1px solid #1e4b66;
                border-radius: 12px;
            }
        )");
        auto *userLayout = new QVBoxLayout(userCard);
        userLayout->setContentsMargins(12, 12, 12, 12);
        userLayout->setSpacing(10);

        auto *profileRow = new QHBoxLayout;
        profileRow->setSpacing(10);

        QLabel *avatarLabel = new QLabel;
        avatarLabel->setFixedSize(38, 38);
        QString currentName = currentManager ? qs(currentManager->getfullName()) : "Quản trị viên";
        avatarLabel->setPixmap(createAvatar(currentName, 38));
        profileRow->addWidget(avatarLabel);

        auto *nameCol = new QVBoxLayout;
        nameCol->setSpacing(2);
        QLabel *nameLabel = new QLabel(currentName);
        nameLabel->setStyleSheet("color: white; font-weight: 700; font-size: 13px;");
        nameLabel->setWordWrap(true);

        QLabel *roleLabel = new QLabel(currentManager ? roleName(currentManager->getRole()) : "Quản trị hệ thống");
        roleLabel->setStyleSheet("color: #8faec0; font-size: 11px; font-weight: 500;");

        nameCol->addWidget(nameLabel);
        nameCol->addWidget(roleLabel);
        profileRow->addLayout(nameCol, 1);
        userLayout->addLayout(profileRow);

        // Nút Đăng xuất / Lưu dữ liệu
        QPushButton *logout = new QPushButton("↪   Đăng xuất");
        logout->setCursor(Qt::PointingHandCursor);
        logout->setStyleSheet(R"(
            QPushButton {
                background: rgba(239, 68, 68, 0.12);
                color: #fca5a5;
                border: 1px solid rgba(239, 68, 68, 0.35);
                border-radius: 8px;
                padding: 7px 12px;
                font-weight: 600;
                font-size: 12.5px;
            }
            QPushButton:hover {
                background: #dc2626;
                color: white;
                border-color: #dc2626;
            }
        )");
        connect(logout, &QPushButton::clicked, this, [this]{
            saveAll();
            QMessageBox::information(this, "Đăng xuất", "Chức năng đăng nhập đang được khóa. Hệ thống tự động lưu và duy trì phiên làm việc.");
        });
        userLayout->addWidget(logout);
        side->addWidget(userCard);

        pages = new QStackedWidget;
        outer->addWidget(sidebar);
        outer->addWidget(pages, 1);

        pages->addWidget(dashboardPage(pages));
        pages->addWidget(bookingPage(pages));
        pages->addWidget(patientPage(pages));
        pages->addWidget(doctorPage(pages));
        pages->addWidget(appointmentPage(pages));
        pages->addWidget(holidayPage(pages));
        if (isAdministrator()) pages->addWidget(managerPage(pages));

        updateNavigation();
        setPage(0);
    }

    // --- TRANG 0: TỔNG QUAN (DASHBOARD) ---
    QWidget *dashboardPage(QWidget *parent = nullptr) {
        int hour = QTime::currentTime().hour();
        QString timeGreeting = (hour >= 5 && hour < 12) ? "Chào buổi sáng" :
                               (hour >= 12 && hour < 18) ? "Chào buổi chiều" : "Chào buổi tối";
        QString managerName = currentManager ? qs(currentManager->getfullName()) : "";
        QString greetingTitle = managerName.isEmpty() ? QStringLiteral("%1 👋").arg(timeGreeting)
                                                      : QStringLiteral("%1, %2 👋").arg(timeGreeting, managerName);
        QString greetingSub = QStringLiteral("Hôm nay là %1 · Chúc bạn một ngày làm việc hiệu quả tại Phòng khám An Tâm.")
                                  .arg(QDate::currentDate().toString("dd/MM/yyyy"));

        QWidget *w = page(greetingTitle, greetingSub, parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        // 4 Thẻ chỉ số nổi bật
        auto *metrics = new QGridLayout;
        metrics->setSpacing(16);
        const QString labels[] = {"BỆNH NHÂN", "BÁC SĨ", "LỊCH KHÁM", "CHƯA KHÁM"};
        const QString values[] = {
            QString::number(countPatients()),
            QString::number(countDoctors()),
            QString::number(countAppointments()),
            QString::number(pendingAppointments())
        };
        const QString colors[] = {"#0d9488", "#0284c7", "#6366f1", "#ea580c"};

        for (int i = 0; i < 4; ++i) {
            auto *card = panel();
            card->setObjectName("card");
            card->setStyleSheet(QStringLiteral("QFrame#card { border-left: 4px solid %1; background: white; border-radius: 12px; }").arg(colors[i]));
            auto *v = new QVBoxLayout(card);
            v->setContentsMargins(20, 18, 20, 18);
            v->setSpacing(6);

            auto *number = new QLabel(values[i]);
            number->setObjectName("metricValue");
            number->setStyleSheet(QStringLiteral("color: %1; font-size: 32px; font-weight: 800;").arg(colors[i]));

            auto *caption = new QLabel(labels[i]);
            caption->setObjectName("metricLabel");

            v->addWidget(number);
            v->addWidget(caption);
            metrics->addWidget(card, 0, i);
        }
        layout->addLayout(metrics);

        // Banner đặt lịch khám
        auto *welcome = panel();
        welcome->setStyleSheet("QFrame#panel { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:1 #f0fdfa); border: 1px solid #ccfbf1; border-radius: 14px; }");
        auto *box = new QHBoxLayout(welcome);
        box->setContentsMargins(24, 22, 24, 22);

        auto *copy = new QVBoxLayout;
        auto *title = new QLabel("Chăm sóc chu đáo, bắt đầu từ một lịch hẹn tốt.");
        title->setStyleSheet("font-size: 19px; font-weight: 700; color: #133b47;");
        auto *sub = new QLabel("Tạo lịch khám mới và chọn bác sĩ phù hợp với triệu chứng của bệnh nhân.");
        sub->setObjectName("muted");
        copy->addWidget(title);
        copy->addWidget(sub);

        auto *book = button("Đặt lịch khám  →");
        connect(book, &QPushButton::clicked, this, [this]{ setPage(1); });
        box->addLayout(copy, 1);
        box->addWidget(book);
        layout->addWidget(welcome);

        // Khung Lịch hẹn sắp tới
        auto *recent = panel();
        auto *recentLayout = new QVBoxLayout(recent);
        recentLayout->setContentsMargins(22, 20, 22, 20);
        recentLayout->setSpacing(14);

        auto *recentHeader = new QHBoxLayout;
        auto *titleRecent = new QLabel("Lịch hẹn sắp tới");
        titleRecent->setStyleSheet("font-size: 18px; font-weight: 750; color: #102f43;");
        auto *subRecent = new QLabel("Lịch khám trong ngày hôm nay và ngày mai");
        subRecent->setStyleSheet("font-size: 13px; color: #64748b; padding-left: 8px;");

        recentHeader->addWidget(titleRecent);
        recentHeader->addWidget(subRecent);
        recentHeader->addStretch();
        recentLayout->addLayout(recentHeader);

        // Bảng 4 cột: BỆNH NHÂN, BÁC SĨ, THỜI GIAN, TRẠNG THÁI
        auto *t = table({"BỆNH NHÂN", "  BÁC SĨ", "THỜI GIAN", "TRẠNG THÁI"});
        if (t->horizontalHeaderItem(0)) t->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        if (t->horizontalHeaderItem(1)) t->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        if (t->horizontalHeaderItem(2)) t->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
        if (t->horizontalHeaderItem(3)) t->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);

        t->verticalHeader()->setDefaultSectionSize(54);
        t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        t->setColumnWidth(2, 170);
        t->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
        t->setColumnWidth(3, 145);

        // Thanh điều hướng phân trang (Pagination Bar)
        auto *pagerLayout = new QHBoxLayout;
        pagerLayout->setContentsMargins(4, 8, 4, 2);

        QLabel *pageInfo = new QLabel;
        pageInfo->setStyleSheet("color: #64748b; font-size: 12.5px; font-weight: 500;");
        pagerLayout->addWidget(pageInfo);
        pagerLayout->addStretch();

        QPushButton *prevBtn = new QPushButton("‹   Trước");
        prevBtn->setCursor(Qt::PointingHandCursor);
        prevBtn->setStyleSheet(R"(
            QPushButton {
                background: white;
                border: 1px solid #dce5eb;
                border-radius: 8px;
                padding: 6px 14px;
                font-size: 12px;
                font-weight: 600;
                color: #334155;
            }
            QPushButton:hover:!disabled {
                background: #f1f5f9;
                border-color: #cbd5e1;
                color: #0f172a;
            }
            QPushButton:disabled {
                background: #f8fafc;
                border-color: #e2e8f0;
                color: #94a3b8;
            }
        )");

        QLabel *pageStatus = new QLabel;
        pageStatus->setStyleSheet("color: #0f172a; font-size: 12.5px; font-weight: 700; padding: 0 8px;");

        QPushButton *nextBtn = new QPushButton("Sau   ›");
        nextBtn->setCursor(Qt::PointingHandCursor);
        nextBtn->setStyleSheet(R"(
            QPushButton {
                background: white;
                border: 1px solid #dce5eb;
                border-radius: 8px;
                padding: 6px 14px;
                font-size: 12px;
                font-weight: 600;
                color: #334155;
            }
            QPushButton:hover:!disabled {
                background: #f1f5f9;
                border-color: #cbd5e1;
                color: #0f172a;
            }
            QPushButton:disabled {
                background: #f8fafc;
                border-color: #e2e8f0;
                color: #94a3b8;
            }
        )");

        pagerLayout->addWidget(prevBtn);
        pagerLayout->addWidget(pageStatus);
        pagerLayout->addWidget(nextBtn);

        connect(prevBtn, &QPushButton::clicked, this, [=, this]() mutable {
            if (dashboardAppPage > 0) {
                fillAppointmentTablePage(t, dashboardAppPage - 1, DASHBOARD_PAGE_SIZE, pageInfo, pageStatus, prevBtn, nextBtn);
            }
        });
        connect(nextBtn, &QPushButton::clicked, this, [=, this]() mutable {
            fillAppointmentTablePage(t, dashboardAppPage + 1, DASHBOARD_PAGE_SIZE, pageInfo, pageStatus, prevBtn, nextBtn);
        });

        fillAppointmentTablePage(t, 0, DASHBOARD_PAGE_SIZE, pageInfo, pageStatus, prevBtn, nextBtn);

        recentLayout->addWidget(t);
        recentLayout->addLayout(pagerLayout);
        layout->addWidget(recent, 1);
        return w;
    }

    void fillAppointmentTablePage(QTableWidget *t, int pageNumber = 0, int pageSize = 8,
                                  QLabel *pageInfo = nullptr, QLabel *pageStatus = nullptr,
                                  QPushButton *prevBtn = nullptr, QPushButton *nextBtn = nullptr) {
        t->setRowCount(0);
        if (!appointments) return;

        vector<Appointment *> all;
        auto *n = appointments->getHeader()->next;
        const QDate today = QDate::currentDate();

        while (n && n->next) {
            if (!n->info.isDefault()) {
                Date d = n->info.getDateGet();
                QDate appointmentDay(d.getYear(), d.getMonth(), d.getDay());
                if (appointmentDay >= today && appointmentDay <= today.addDays(1)) {
                    all.push_back(&n->info);
                }
            }
            n = n->next;
        }

        sort(all.begin(), all.end(), [](Appointment *a, Appointment *b){
            return a->getDateGet().toString() < b->getDateGet().toString();
        });

        int totalItems = static_cast<int>(all.size());
        int totalPages = totalItems > 0 ? (totalItems + pageSize - 1) / pageSize : 1;

        if (pageNumber < 0) pageNumber = 0;
        if (pageNumber >= totalPages) pageNumber = totalPages - 1;
        dashboardAppPage = pageNumber;

        int startIndex = pageNumber * pageSize;
        int endIndex = std::min(startIndex + pageSize, totalItems);

        for (int i = startIndex; i < endIndex; ++i) {
            Appointment *a = all[i];
            int row = t->rowCount();
            t->insertRow(row);
            Patient *p = findPatient(a->getPatientId());
            Doctor *d = findDoctor(a->getDoctorId());

            // --- CỘT 0: BỆNH NHÂN (Tên đậm + Tuổi & Giới tính) ---
            auto *pBox = new QWidget;
            pBox->setStyleSheet("background: transparent;");
            auto *pCol = new QVBoxLayout(pBox);
            pCol->setContentsMargins(13, 7, 14, 7);
            pCol->setSpacing(3);
            pCol->setAlignment(Qt::AlignVCenter);

            QString pName = p ? qs(p->getfullName()) : qs(a->getPatientId());
            QLabel *pNameLbl = new QLabel(pName);
            pNameLbl->setStyleSheet("font-weight: 700; font-size: 13.5px; color: #0f172a;");

            QString pSub;
            if (p) {
                QString gender = p->getGender() ? "Nam" : "Nữ";
                pSub = QStringLiteral("%1 tuổi (%2)").arg(p->getAge()).arg(gender);
            } else {
                pSub = "Bệnh nhân";
            }
            QLabel *pSubLbl = new QLabel(pSub);
            pSubLbl->setStyleSheet("font-size: 11.5px; color: #64748b; font-weight: 500;");
            pCol->addWidget(pNameLbl);
            pCol->addWidget(pSubLbl);
            t->setCellWidget(row, 0, pBox);

            // --- CỘT 1: BÁC SĨ ("BS. Họ Tên" + Chuyên khoa · Room Số) ---
            auto *dBox = new QWidget;
            dBox->setStyleSheet("background: transparent;");
            auto *dCol = new QVBoxLayout(dBox);
            dCol->setContentsMargins(13, 7, 14, 7);
            dCol->setSpacing(3);
            dCol->setAlignment(Qt::AlignVCenter);

            QString dRawName = d ? qs(d->getfullName()) : qs(a->getDoctorId());
            QString dTitle = d ? QStringLiteral("BS. %1").arg(dRawName) : QStringLiteral("Bác sĩ: %1").arg(dRawName);
            QLabel *dNameLbl = new QLabel(dTitle);
            dNameLbl->setStyleSheet("font-weight: 700; font-size: 13.5px; color: #0f172a;");

            QString dSub;
            if (d) {
                QString roomStr = qs(d->getRoomId()).replace('_', ' ').trimmed();
                if (!roomStr.startsWith("Room", Qt::CaseInsensitive)) {
                    roomStr = "Room " + roomStr;
                }
                dSub = QStringLiteral("%1 · %2").arg(specialty(d->getSpId()), roomStr);
            } else {
                dSub = "Bác sĩ phụ trách";
            }
            QLabel *dSubLbl = new QLabel(dSub);
            dSubLbl->setStyleSheet("font-size: 11.5px; color: #0284c7; font-weight: 600;");
            dCol->addWidget(dNameLbl);
            dCol->addWidget(dSubLbl);
            t->setCellWidget(row, 1, dBox);

            // --- CỘT 2: THỜI GIAN (Giờ to đậm + Ngày định dạng Hôm nay/Ngày mai) ---
            auto *tBox = new QWidget;
            tBox->setStyleSheet("background: transparent;");
            auto *tLayout = new QVBoxLayout(tBox);
            tLayout->setContentsMargins(10, 6, 10, 6);
            tLayout->setSpacing(2);
            tLayout->setAlignment(Qt::AlignCenter);

            Date ad = a->getDateGet();
            QString timeStr = QStringLiteral("%1:%2")
                .arg(ad.getHour(), 2, 10, QChar('0'))
                .arg(ad.getMinute(), 2, 10, QChar('0'));
            QLabel *timeLbl = new QLabel(timeStr);
            timeLbl->setAlignment(Qt::AlignCenter);
            timeLbl->setStyleSheet("font-weight: 700; font-size: 14px; color: #0f172a;");

            QDate appDay(ad.getYear(), ad.getMonth(), ad.getDay());
            QString dateDesc;
            if (appDay == today) {
                dateDesc = "Hôm nay, " + appDay.toString("dd/MM");
            } else if (appDay == today.addDays(1)) {
                dateDesc = "Ngày mai, " + appDay.toString("dd/MM");
            } else {
                dateDesc = appDay.toString("dd/MM/yyyy");
            }
            QLabel *dateLbl = new QLabel(dateDesc);
            dateLbl->setAlignment(Qt::AlignCenter);
            dateLbl->setStyleSheet("font-size: 11.5px; color: #64748b; font-weight: 500;");
            tLayout->addWidget(timeLbl);
            tLayout->addWidget(dateLbl);
            t->setCellWidget(row, 2, tBox);

            // --- CỘT 3: TRẠNG THÁI (Pill Badge hiện đại có chấm tròn chỉ báo) ---
            auto *badgeBox = new QWidget;
            badgeBox->setStyleSheet("background: transparent;");
            auto *bl = new QHBoxLayout(badgeBox);
            bl->setContentsMargins(0, 0, 0, 0);
            bl->setAlignment(Qt::AlignCenter);

            auto *pill = new QLabel(a->getState() ? "●  Đã khám" : "●  Chưa khám");
            if (a->getState()) {
                pill->setStyleSheet("background: #ecfdf5; color: #047857; border: 1px solid #a7f3d0; font-weight: 600; font-size: 12px; border-radius: 12px; padding: 5px 14px;");
            } else {
                pill->setStyleSheet("background: #fffbeb; color: #b45309; border: 1px solid #fde68a; font-weight: 600; font-size: 12px; border-radius: 12px; padding: 5px 14px;");
            }
            bl->addWidget(pill);
            t->setCellWidget(row, 3, badgeBox);
        }

        if (pageInfo) {
            if (totalItems == 0) {
                pageInfo->setText("Chưa có lịch hẹn nào sắp tới.");
            } else {
                pageInfo->setText(QStringLiteral("Hiển thị %1 - %2 trên tổng số %3 lịch hẹn")
                                      .arg(startIndex + 1).arg(endIndex).arg(totalItems));
            }
        }
        if (pageStatus) {
            pageStatus->setText(QStringLiteral("Trang %1 / %2").arg(pageNumber + 1).arg(totalPages));
        }
        if (prevBtn) {
            prevBtn->setEnabled(pageNumber > 0);
        }
        if (nextBtn) {
            nextBtn->setEnabled(pageNumber < totalPages - 1);
        }
    }

    void fillAppointmentTable(QTableWidget *t, bool upcomingOnly = true) {
        Q_UNUSED(upcomingOnly);
        fillAppointmentTablePage(t, 0, DASHBOARD_PAGE_SIZE);
    }

    // --- TRANG 1: ĐẶT LỊCH KHÁM ---
    QWidget *bookingPage(QWidget *parent = nullptr) {
        QWidget *w = page("Đặt lịch khám", "Chọn hồ sơ bệnh nhân, triệu chứng và bác sĩ phù hợp.", parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        auto *card = panel();
        auto *form = new QFormLayout(card);
        form->setContentsMargins(25, 23, 25, 23);
        form->setSpacing(14);

        auto *patient = new QComboBox;
        if (patients) {
            auto *p = patients->getHeader()->next;
            while (p && p->next) {
                patient->addItem(QStringLiteral("%1  ·  %2").arg(qs(p->info.getID()), qs(p->info.getfullName())), qs(p->info.getID()));
                p = p->next;
            }
        }

        auto *symptom = new QComboBox;
        symptom->addItem("Chọn triệu chứng chính…", "");
        if (symptoms) {
            auto *s = symptoms->getHeader()->next;
            while (s && s->next) {
                symptom->addItem(QStringLiteral("%1  ·  %2").arg(qs(s->info.getSName()), specialty(s->info.getSp_id())), qs(s->info.getS_id()));
                s = s->next;
            }
        }

        auto *doctor = new QComboBox;
        doctor->addItem("Chọn bác sĩ…", "");

        auto *fee = new QLabel("Chọn bác sĩ để xem phí khám");
        fee->setObjectName("muted");

        auto *date = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1));
        date->setDisplayFormat("dd/MM/yyyy  HH:mm");
        date->setCalendarPopup(true);

        form->addRow("Bệnh nhân", patient);
        form->addRow("Triệu chứng", symptom);
        form->addRow("Bác sĩ khám", doctor);
        form->addRow("Phí khám", fee);
        form->addRow("Ngày giờ hẹn", date);

        connect(symptom, qOverload<int>(&QComboBox::currentIndexChanged), w, [this, symptom, doctor, fee]{
            doctor->clear();
            doctor->addItem("Chọn bác sĩ…", "");
            fee->setText("Chọn bác sĩ để xem phí khám");

            string spid;
            if (symptoms) {
                auto *s = symptoms->getHeader()->next;
                while (s && s->next) {
                    if (qs(s->info.getS_id()) == symptom->currentData().toString()) {
                        spid = s->info.getSp_id();
                        break;
                    }
                    s = s->next;
                }
            }

            if (doctors) {
                auto *d = doctors->getHeader()->next;
                while (d && d->next) {
                    if (d->info.getState() && (spid.empty() || d->info.getSpId() == spid)) {
                        doctor->addItem(QStringLiteral("%1  ·  %2  ·  %3").arg(qs(d->info.getID()), qs(d->info.getfullName()), specialty(d->info.getSpId())), qs(d->info.getID()));
                    }
                    d = d->next;
                }
            }
        });

        connect(doctor, qOverload<int>(&QComboBox::currentIndexChanged), w, [this, doctor, fee]{
            Doctor *selected = findDoctor(ss(doctor->currentData().toString()));
            fee->setText(selected ? QLocale(QLocale::Vietnamese).toString(selected->getFee(), 'f', 0) + " ₫" : "Chọn bác sĩ để xem phí khám");
        });

        auto *save = button("Xác nhận đặt lịch");
        if (!hasPermission("Per_4")) save->hide();

        connect(save, &QPushButton::clicked, w, [this, patient, symptom, doctor, date]{
            if (patient->currentIndex() < 0 || doctor->currentData().toString().isEmpty() || symptom->currentData().toString().isEmpty()) {
                QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng chọn bệnh nhân, triệu chứng và bác sĩ.");
                return;
            }
            if (date->dateTime() <= QDateTime::currentDateTime()) {
                QMessageBox::warning(this, "Thời gian không hợp lệ", "Vui lòng chọn thời gian trong tương lai.");
                return;
            }
            if (isHoliday(date->date())) {
                QMessageBox::warning(this, "Ngày nghỉ lễ", "Phòng khám nghỉ lễ vào ngày này. Vui lòng chọn ngày khác.");
                return;
            }

            string pId = ss(patient->currentData().toString());
            string dId = ss(doctor->currentData().toString());
            string symptomId = ss(symptom->currentData().toString());

            if (appointments) {
                appointments->insert(Appointment(nextAppointmentId(), pId, dId, Date::now(), Date::loadFromString(ss(date->dateTime().toString("yyyy-MM-dd HH:mm"))), 0, symptomId));
            }
            if (symptomPatients) {
                symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients), symptomId, pId, Date::now()));
            }

            saveAll();
            QMessageBox::information(this, "Đã đặt lịch", "Lịch khám đã được lưu thành công.");
            refreshPage(4);
            refreshPage(0);
        });

        layout->addWidget(card);
        layout->addWidget(save, 0, Qt::AlignLeft);
        layout->addStretch();
        return w;
    }

    // --- TRANG 2: BỆNH NHÂN ---
    QWidget *patientPage(QWidget *parent = nullptr) {
        QWidget *w = page("Bệnh nhân", "Quản lý hồ sơ và thông tin triệu chứng của người bệnh.", parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        auto *actions = new QHBoxLayout;
        auto *search = new QLineEdit;
        search->setPlaceholderText("Tìm theo tên, mã, điện thoại hoặc CCCD…");

        auto *add = button("＋  Thêm bệnh nhân");
        auto *edit = button("Sửa hồ sơ", "secondary");
        auto *remove = button("Xóa đã khám hết", "danger");

        actions->addWidget(search, 1);
        actions->addWidget(add);
        actions->addWidget(edit);
        actions->addWidget(remove);
        layout->addLayout(actions);

        if (!hasPermission("Per_3")) {
            add->hide();
            edit->hide();
            remove->hide();
        }

        auto *t = table({"MÃ HỒ SƠ", "HỌ VÀ TÊN", "ĐIỆN THOẠI", "CCCD", "TUỔI", "GIỚI TÍNH", "TRIỆU CHỨNG"});

        auto fill = [this, t, search]{
            t->setRowCount(0);
            if (!patients) return;
            QString q = search->text().trimmed().toLower();
            auto *n = patients->getHeader()->next;
            while (n && n->next) {
                QString name = qs(n->info.getfullName());
                QString id = qs(n->info.getID());
                QString phone = qs(n->info.getPhone());
                QString cccd = qs(n->info.getCCCD());

                if (q.isEmpty() || name.toLower().contains(q) || id.toLower().contains(q) || phone.contains(q) || cccd.contains(q)) {
                    int r = t->rowCount();
                    t->insertRow(r);
                    cell(t, r, 0, id);
                    cell(t, r, 1, name);
                    cell(t, r, 2, phone);
                    cell(t, r, 3, cccd);
                    cell(t, r, 4, QString::number(n->info.getAge()), Qt::AlignCenter);
                    cell(t, r, 5, n->info.getGender() ? "Nam" : "Nữ", Qt::AlignCenter);
                    cell(t, r, 6, unresolvedPatientSymptoms(n->info.getID()));
                }
                n = n->next;
            }
        };

        connect(search, &QLineEdit::textChanged, w, [fill]{ fill(); });
        fill();
        layout->addWidget(t, 1);

        connect(add, &QPushButton::clicked, w, [this, fill]{
            addPatientDialog();
            fill();
        });

        connect(edit, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row < 0) return;
            editPatientDialog(ss(t->item(row, 0)->text()));
            fill();
        });

        connect(remove, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row < 0) return;
            string id = ss(t->item(row, 0)->text());

            if (unresolvedPatientSymptoms(id) != QStringLiteral("Đã khám hết")) {
                QMessageBox::warning(this, "Chưa thể xóa", "Chỉ có thể xóa hồ sơ đã khám hết các triệu chứng.");
                return;
            }
            if (QMessageBox::question(this, "Xóa bệnh nhân", "Xóa hồ sơ bệnh nhân đã khám hết?") != QMessageBox::Yes) {
                return;
            }

            if (patients) {
                auto *n = patients->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getID() == id) {
                        patients->remove(n);
                        break;
                    }
                    n = n->next;
                }
            }
            if (symptomPatients) {
                auto *s = symptomPatients->getHeader()->next;
                while (s && s->next) {
                    auto *next = s->next;
                    if (s->info.getP_id() == id) symptomPatients->remove(s);
                    s = next;
                }
            }
            if (appointments) {
                auto *a = appointments->getHeader()->next;
                while (a && a->next) {
                    auto *next = a->next;
                    if (a->info.getPatientId() == id) appointments->remove(a);
                    a = next;
                }
            }
            saveAll();
            fill();
        });

        return w;
    }

    void addPatientDialog() {
        QDialog dialog(this);
        dialog.setWindowTitle("Thêm bệnh nhân mới");
        dialog.setMinimumWidth(440);

        auto *v = new QVBoxLayout(&dialog);
        auto *form = new QFormLayout;

        auto *name = new QLineEdit;
        auto *phone = new QLineEdit;
        auto *cccd = new QLineEdit;
        auto *age = new QSpinBox;
        age->setRange(1, 130);
        age->setValue(30);

        auto *gender = new QComboBox;
        gender->addItems({"Nam", "Nữ"});

        auto *symptomChecks = new QWidget;
        auto *checks = new QVBoxLayout(symptomChecks);
        vector<QCheckBox *> boxes;

        if (symptoms) {
            auto *s = symptoms->getHeader()->next;
            while (s && s->next) {
                auto *check = new QCheckBox(QStringLiteral("%1  ·  %2").arg(qs(s->info.getSName()), specialty(s->info.getSp_id())));
                check->setProperty("sid", qs(s->info.getS_id()));
                checks->addWidget(check);
                boxes.push_back(check);
                s = s->next;
            }
        }

        auto *scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setWidget(symptomChecks);
        scroll->setFixedHeight(150);

        form->addRow("Họ và tên", name);
        form->addRow("Điện thoại", phone);
        form->addRow("CCCD", cccd);
        form->addRow("Tuổi", age);
        form->addRow("Giới tính", gender);
        form->addRow("Triệu chứng ban đầu", scroll);
        v->addLayout(form);

        auto *buttons = new QHBoxLayout;
        buttons->addStretch();
        auto *cancel = button("Hủy", "secondary");
        auto *save = button("Tạo hồ sơ");
        buttons->addWidget(cancel);
        buttons->addWidget(save);
        v->addLayout(buttons);

        connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(save, &QPushButton::clicked, &dialog, [this, &dialog, name, phone, cccd, age, gender, boxes]{
            if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() || cccd->text().trimmed().isEmpty()) {
                QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đủ họ tên, điện thoại và CCCD.");
                return;
            }
            if (patients) {
                auto *n = patients->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getPhone() == ss(phone->text().trimmed()) || n->info.getCCCD() == ss(cccd->text().trimmed())) {
                        QMessageBox::warning(&dialog, "Trùng hồ sơ", "Số điện thoại hoặc CCCD đã tồn tại.");
                        return;
                    }
                    n = n->next;
                }
            }

            string id = Patient::generateNextId(patients);
            if (patients) {
                patients->insert(Patient(id, ss(name->text().trimmed()), ss(phone->text().trimmed()), gender->currentIndex() == 0, age->value(), ss(cccd->text().trimmed())));
            }

            string ids;
            for (auto *box : boxes) {
                if (box->isChecked()) {
                    if (!ids.empty()) ids += ", ";
                    ids += ss(box->property("sid").toString());
                }
            }
            if (!ids.empty() && symptomPatients) {
                symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients), ids, id, Date::now()));
            }

            saveAll();
            dialog.accept();
        });
        dialog.exec();
    }

    void editPatientDialog(const string &id) {
        Patient *patient = findPatient(id);
        if (!patient || !hasPermission("Per_3")) return;

        QDialog dialog(this);
        dialog.setWindowTitle("Chỉnh sửa bệnh nhân");

        auto *layout = new QVBoxLayout(&dialog);
        auto *form = new QFormLayout;

        auto *name = new QLineEdit(qs(patient->getfullName()));
        auto *phone = new QLineEdit(qs(patient->getPhone()));
        auto *cccd = new QLineEdit(qs(patient->getCCCD()));
        auto *age = new QSpinBox;
        age->setRange(1, 130);
        age->setValue(patient->getAge());

        auto *gender = new QComboBox;
        gender->addItems({"Nam", "Nữ"});
        gender->setCurrentIndex(patient->getGender() ? 0 : 1);

        form->addRow("Họ và tên", name);
        form->addRow("Điện thoại", phone);
        form->addRow("CCCD", cccd);
        form->addRow("Tuổi", age);
        form->addRow("Giới tính", gender);
        layout->addLayout(form);

        auto *save = button("Lưu thay đổi");
        layout->addWidget(save);

        connect(save, &QPushButton::clicked, &dialog, [this, &dialog, patient, name, phone, cccd, age, gender]{
            if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() || cccd->text().trimmed().isEmpty()) {
                QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đủ họ tên, điện thoại và CCCD.");
                return;
            }
            if (patients) {
                auto *n = patients->getHeader()->next;
                while (n && n->next) {
                    if (&n->info != patient && (n->info.getPhone() == ss(phone->text().trimmed()) || n->info.getCCCD() == ss(cccd->text().trimmed()))) {
                        QMessageBox::warning(&dialog, "Trùng hồ sơ", "Số điện thoại hoặc CCCD đã tồn tại.");
                        return;
                    }
                    n = n->next;
                }
            }

            patient->setfullName(ss(name->text().trimmed()));
            patient->setPhone(ss(phone->text().trimmed()));
            patient->setCCCD(ss(cccd->text().trimmed()));
            patient->setAge(age->value());
            patient->setGender(gender->currentIndex() == 0);

            saveAll();
            dialog.accept();
        });
        dialog.exec();
    }

    // --- TRANG 3: BÁC SĨ ---
    QWidget *doctorPage(QWidget *parent = nullptr) {
        QWidget *w = page("Đội ngũ bác sĩ", "Danh sách bác sĩ và chuyên khoa đang hoạt động tại phòng khám.", parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        auto *actions = new QHBoxLayout;
        auto *search = new QLineEdit;
        search->setPlaceholderText("Tìm bác sĩ hoặc chuyên khoa…");

        auto *edit = button("Sửa thông tin", "secondary");
        auto *remove = button("Xóa bác sĩ đã nghỉ", "danger");
        if (!hasPermission("Per_2")) {
            edit->hide();
            remove->hide();
        }

        actions->addWidget(search, 1);
        actions->addWidget(edit);
        actions->addWidget(remove);
        layout->addLayout(actions);

        auto *t = table({"MÃ BÁC SĨ", "BÁC SĨ", "CHUYÊN KHOA", "PHÒNG", "PHÍ KHÁM", "ĐÁNH GIÁ", "TRẠNG THÁI"});

        auto fill = [this, t, search]{
            t->setRowCount(0);
            if (!doctors) return;
            QString q = search->text().trimmed().toLower();
            auto *n = doctors->getHeader()->next;
            while (n && n->next) {
                QString name = qs(n->info.getfullName());
                QString spec = specialty(n->info.getSpId());
                if (q.isEmpty() || name.toLower().contains(q) || spec.toLower().contains(q)) {
                    int r = t->rowCount();
                    t->insertRow(r);
                    cell(t, r, 0, qs(n->info.getID()));
                    cell(t, r, 1, name);
                    cell(t, r, 2, spec);
                    cell(t, r, 3, qs(n->info.getRoomId()), Qt::AlignCenter);
                    cell(t, r, 4, QLocale(QLocale::Vietnamese).toString(n->info.getFee(), 'f', 0) + " ₫", Qt::AlignRight | Qt::AlignVCenter);
                    cell(t, r, 5, QString::number(n->info.getRating(), 'f', 1) + " ★", Qt::AlignCenter);
                    cell(t, r, 6, n->info.getState() ? "Đang nhận khám" : "Tạm nghỉ", Qt::AlignCenter);
                }
                n = n->next;
            }
        };

        connect(search, &QLineEdit::textChanged, w, [fill]{ fill(); });
        fill();
        layout->addWidget(t, 1);

        connect(edit, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row < 0) return;
            editDoctorDialog(ss(t->item(row, 0)->text()));
            fill();
        });

        connect(remove, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row < 0) return;
            string id = ss(t->item(row, 0)->text());
            Doctor *d = findDoctor(id);
            if (!d || d->getState()) {
                QMessageBox::warning(this, "Chưa thể xóa", "Chỉ có thể xóa bác sĩ đã ở trạng thái nghỉ việc.");
                return;
            }
            if (QMessageBox::question(this, "Xóa bác sĩ", "Xóa hồ sơ bác sĩ đã nghỉ việc này?") != QMessageBox::Yes) {
                return;
            }
            if (doctors) {
                auto *n = doctors->getHeader()->next;
                while (n && n->next) {
                    auto *next = n->next;
                    if (n->info.getID() == id) doctors->remove(n);
                    n = next;
                }
            }
            saveAll();
            fill();
        });

        return w;
    }

    void editDoctorDialog(const string &id) {
        Doctor *doctor = findDoctor(id);
        if (!doctor || !hasPermission("Per_2")) return;

        QDialog dialog(this);
        dialog.setWindowTitle("Chỉnh sửa thông tin bác sĩ");

        auto *layout = new QVBoxLayout(&dialog);
        auto *form = new QFormLayout;

        auto *name = new QLineEdit(qs(doctor->getfullName()));
        auto *phone = new QLineEdit(qs(doctor->getPhone()));
        auto *room = new QLineEdit(qs(doctor->getRoomId()));

        auto *fee = new QDoubleSpinBox;
        fee->setRange(0, 100000000);
        fee->setDecimals(0);
        fee->setValue(doctor->getFee());

        auto *rating = new QDoubleSpinBox;
        rating->setRange(0, 5);
        rating->setSingleStep(0.1);
        rating->setValue(doctor->getRating());

        auto *age = new QSpinBox;
        age->setRange(18, 120);
        age->setValue(doctor->getAge());

        auto *gender = new QComboBox;
        gender->addItems({"Nam", "Nữ"});
        gender->setCurrentIndex(doctor->getGender() ? 0 : 1);

        auto *specialtyBox = new QComboBox;
        ifstream specs(specializationPath);
        string line;
        getline(specs, line);
        getline(specs, line);
        while (getline(specs, line)) {
            auto split = line.find('|');
            if (split == string::npos) continue;
            string sid = line.substr(0, split);
            string label = line.substr(split + 1);
            libPBL2::trim(sid);
            libPBL2::trim(label);
            specialtyBox->addItem(qs(label), qs(sid));
        }
        specialtyBox->setCurrentIndex(specialtyBox->findData(qs(doctor->getSpId())));

        auto *state = new QComboBox;
        state->addItems({"Đang làm việc", "Đã nghỉ việc"});
        state->setCurrentIndex(doctor->getState() ? 0 : 1);

        form->addRow("Họ và tên", name);
        form->addRow("Điện thoại", phone);
        form->addRow("Tuổi", age);
        form->addRow("Giới tính", gender);
        form->addRow("Chuyên khoa", specialtyBox);
        form->addRow("Phòng khám", room);
        form->addRow("Phí khám", fee);
        form->addRow("Đánh giá sao", rating);
        form->addRow("Trạng thái", state);
        layout->addLayout(form);

        auto *save = button("Lưu thay đổi");
        layout->addWidget(save);

        connect(save, &QPushButton::clicked, &dialog, [this, &dialog, doctor, name, phone, room, fee, rating, age, gender, specialtyBox, state]{
            if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty()) {
                QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập họ tên và điện thoại.");
                return;
            }
            doctor->setfullName(ss(name->text().trimmed()));
            doctor->setPhone(ss(phone->text().trimmed()));
            doctor->setAge(age->value());
            doctor->setGender(gender->currentIndex() == 0);
            doctor->setSpId(ss(specialtyBox->currentData().toString()));
            doctor->setRoomId(ss(room->text().trimmed()));
            doctor->setFee(fee->value());
            doctor->setRating(rating->value());
            doctor->setState(state->currentIndex() == 0);

            saveAll();
            dialog.accept();
        });
        dialog.exec();
    }

    // --- TRANG 4: QUẢN LÝ LỊCH KHÁM ---
    QWidget *appointmentPage(QWidget *parent = nullptr) {
        QWidget *w = page("Quản lý lịch khám", "Theo dõi lịch theo bác sĩ, ngày hẹn và trạng thái khám.", parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        auto *filters = new QHBoxLayout;
        auto *doctor = new QComboBox;
        doctor->addItem("Tất cả bác sĩ", "");
        if (doctors) {
            auto *d = doctors->getHeader()->next;
            while (d && d->next) {
                doctor->addItem(QStringLiteral("%1 · %2").arg(qs(d->info.getID()), qs(d->info.getfullName())), qs(d->info.getID()));
                d = d->next;
            }
        }

        auto *date = new QDateEdit(QDate::currentDate());
        date->setCalendarPopup(true);
        date->setDisplayFormat("dd/MM/yyyy");

        auto *allDates = new QCheckBox("Tất cả ngày");
        allDates->setChecked(true);

        auto *status = new QComboBox;
        status->addItems({"Tất cả trạng thái", "Chưa khám", "Đã khám"});

        filters->addWidget(new QLabel("Bác sĩ:"));
        filters->addWidget(doctor, 1);
        filters->addWidget(new QLabel("Ngày:"));
        filters->addWidget(date);
        filters->addWidget(allDates);
        filters->addWidget(status);
        layout->addLayout(filters);

        auto *t = table({"MÃ LỊCH", "BỆNH NHÂN", "TRIỆU CHỨNG", "BÁC SĨ / CHUYÊN KHOA", "NGÀY GIỜ HẸN", "TRẠNG THÁI"});

        auto fill = [this, t, doctor, date, allDates, status]{
            t->setRowCount(0);
            if (!appointments) return;

            vector<Appointment*> ordered;
            auto *n = appointments->getHeader()->next;
            while (n && n->next) {
                if (!n->info.isDefault()) ordered.push_back(&n->info);
                n = n->next;
            }

            sort(ordered.begin(), ordered.end(), [](Appointment *a, Appointment *b){
                Date x = a->getDateGet(), y = b->getDateGet();
                return make_tuple(x.getYear(), x.getMonth(), x.getDay(), x.getHour(), x.getMinute()) <
                       make_tuple(y.getYear(), y.getMonth(), y.getDay(), y.getHour(), y.getMinute());
            });

            for (auto *item : ordered) {
                auto &a = *item;
                Doctor *d = findDoctor(a.getDoctorId());
                Patient *p = findPatient(a.getPatientId());
                Date ad = a.getDateGet();

                bool matchesDoctor = doctor->currentData().toString().isEmpty() || qs(a.getDoctorId()) == doctor->currentData().toString();
                bool matchesDate = allDates->isChecked() || (ad.getYear() == date->date().year() && ad.getMonth() == date->date().month() && ad.getDay() == date->date().day());
                bool matchesStatus = status->currentIndex() == 0 ||
                                     (status->currentIndex() == 1 && a.getState() == 0) ||
                                     (status->currentIndex() == 2 && a.getState() == 1);

                if (matchesDoctor && matchesDate && matchesStatus) {
                    int r = t->rowCount();
                    t->insertRow(r);
                    cell(t, r, 0, qs(a.getId()));
                    cell(t, r, 1, p ? qs(p->getfullName()) : qs(a.getPatientId()));
                    cell(t, r, 2, patientSymptoms(a.getPatientId()));
                    cell(t, r, 3, d ? QStringLiteral("%1 · %2").arg(qs(d->getfullName()), specialty(d->getSpId())) : qs(a.getDoctorId()));
                    cell(t, r, 4, qs(a.getDateGet().toString()), Qt::AlignCenter);
                    cell(t, r, 5, a.getState() ? "Đã khám" : "Chưa khám", Qt::AlignCenter);
                }
            }
        };

        connect(doctor, qOverload<int>(&QComboBox::currentIndexChanged), w, [fill]{ fill(); });
        connect(date, &QDateEdit::dateChanged, w, [fill]{ fill(); });
        connect(allDates, &QCheckBox::toggled, w, [fill]{ fill(); });
        connect(status, qOverload<int>(&QComboBox::currentIndexChanged), w, [fill]{ fill(); });

        auto *actions = new QHBoxLayout;
        auto *invite = button("Mời bệnh nhân đến khám");
        auto *edit = button("Sửa lịch hẹn", "secondary");
        auto *remove = button("Xóa lịch đã khám", "danger");

        if (!hasPermission("Per_4")) {
            invite->hide();
            edit->hide();
            remove->hide();
        }

        actions->addWidget(invite);
        actions->addWidget(edit);
        actions->addWidget(remove);
        actions->addStretch();
        layout->addLayout(actions);
        layout->addWidget(t, 1);
        fill();

        connect(invite, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row < 0) {
                QMessageBox::information(this, "Chọn lịch", "Hãy chọn lịch hẹn cần mời.");
                return;
            }
            string id = ss(t->item(row, 0)->text());
            if (appointments) {
                auto *n = appointments->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getId() == id) {
                        if (n->info.getState()) {
                            QMessageBox::information(this, "Lịch đã hoàn tất", "Lịch này đã ở trạng thái đã khám.");
                            return;
                        }
                        Patient *p = findPatient(n->info.getPatientId());
                        Doctor *d = findDoctor(n->info.getDoctorId());
                        QString patientName = p ? qs(p->getfullName()) : qs(n->info.getPatientId());
                        QString room = d ? qs(d->getRoomId()) : QStringLiteral("chưa xác định");

                        QMessageBox::information(this, "Mời bệnh nhân đến khám", QStringLiteral("Mời bệnh nhân %1 đến phòng %2 để khám bệnh.").arg(patientName, room));
                        n->info.setState(1);
                        updatePatientSymptomStatus(n->info.getPatientId());
                        saveAll();
                        fill();
                        refreshPage(2);
                        return;
                    }
                    n = n->next;
                }
            }
        });

        connect(edit, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row >= 0) {
                editAppointmentDialog(ss(t->item(row, 0)->text()));
                fill();
            }
        });

        connect(remove, &QPushButton::clicked, w, [this, t, fill]{
            int row = t->currentRow();
            if (row < 0) return;
            string id = ss(t->item(row, 0)->text());
            if (appointments) {
                auto *n = appointments->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getId() == id) {
                        if (!n->info.getState()) {
                            QMessageBox::warning(this, "Chưa thể xóa", "Chỉ có thể xóa lịch hẹn đã khám.");
                            return;
                        }
                        if (QMessageBox::question(this, "Xóa lịch", "Xóa lịch hẹn đã khám này?") != QMessageBox::Yes) {
                            return;
                        }
                        appointments->remove(n);
                        saveAll();
                        fill();
                        return;
                    }
                    n = n->next;
                }
            }
        });

        return w;
    }

    void editAppointmentDialog(const string &id) {
        if (!hasPermission("Per_4") || !appointments) return;
        Appointment *appointment = nullptr;
        auto *n = appointments->getHeader()->next;
        while (n && n->next) {
            if (n->info.getId() == id) {
                appointment = &n->info;
                break;
            }
            n = n->next;
        }
        if (!appointment) return;

        QDialog dialog(this);
        dialog.setWindowTitle("Chỉnh sửa lịch hẹn");

        auto *layout = new QVBoxLayout(&dialog);
        auto *form = new QFormLayout;

        auto *patient = new QComboBox;
        if (patients) {
            auto *p = patients->getHeader()->next;
            while (p && p->next) {
                patient->addItem(QStringLiteral("%1 · %2").arg(qs(p->info.getID()), qs(p->info.getfullName())), qs(p->info.getID()));
                p = p->next;
            }
        }
        patient->setCurrentIndex(patient->findData(qs(appointment->getPatientId())));

        auto *doctor = new QComboBox;
        if (doctors) {
            auto *d = doctors->getHeader()->next;
            while (d && d->next) {
                doctor->addItem(QStringLiteral("%1 · %2").arg(qs(d->info.getID()), qs(d->info.getfullName())), qs(d->info.getID()));
                d = d->next;
            }
        }
        doctor->setCurrentIndex(doctor->findData(qs(appointment->getDoctorId())));

        auto *symptom = new QComboBox;
        if (symptoms) {
            auto *s = symptoms->getHeader()->next;
            while (s && s->next) {
                symptom->addItem(QStringLiteral("%1 · %2").arg(qs(s->info.getS_id()), qs(s->info.getSName())), qs(s->info.getS_id()));
                s = s->next;
            }
        }
        symptom->setCurrentIndex(symptom->findData(qs(appointment->getSymptomId())));

        Date old = appointment->getDateGet();
        auto *date = new QDateTimeEdit(QDateTime(QDate(old.getYear(), old.getMonth(), old.getDay()), QTime(old.getHour(), old.getMinute())));
        date->setDisplayFormat("dd/MM/yyyy HH:mm");
        date->setCalendarPopup(true);

        auto *state = new QComboBox;
        state->addItems({"Chưa khám", "Đã khám"});
        state->setCurrentIndex(appointment->getState() ? 1 : 0);

        form->addRow("Bệnh nhân", patient);
        form->addRow("Bác sĩ", doctor);
        form->addRow("Triệu chứng", symptom);
        form->addRow("Ngày giờ", date);
        form->addRow("Trạng thái", state);
        layout->addLayout(form);

        auto *save = button("Lưu thay đổi");
        layout->addWidget(save);

        connect(save, &QPushButton::clicked, &dialog, [this, &dialog, appointment, patient, doctor, symptom, date, state]{
            if (patient->currentIndex() < 0 || doctor->currentIndex() < 0) {
                QMessageBox::warning(&dialog, "Thiếu thông tin", "Hãy chọn bệnh nhân và bác sĩ.");
                return;
            }
            if (isHoliday(date->date())) {
                QMessageBox::warning(&dialog, "Ngày nghỉ lễ", "Không thể xếp lịch vào ngày nghỉ lễ.");
                return;
            }

            string oldPatientId = appointment->getPatientId();
            appointment->setPatientId(ss(patient->currentData().toString()));
            appointment->setDoctorId(ss(doctor->currentData().toString()));

            string symptomId = ss(symptom->currentData().toString());
            appointment->setSymptomId(symptomId);
            appointment->setDateGet(Date::loadFromString(ss(date->dateTime().toString("yyyy-MM-dd HH:mm"))));
            appointment->setState(state->currentIndex());

            if (!symptomId.empty() && symptomPatients) {
                bool found = false;
                auto *r = symptomPatients->getHeader()->next;
                while (r && r->next) {
                    if (r->info.getP_id() == appointment->getPatientId() && r->info.getS_id().find(symptomId) != string::npos) {
                        found = true;
                    }
                    r = r->next;
                }
                if (!found) {
                    symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients), symptomId, appointment->getPatientId(), Date::now()));
                }
            }

            updatePatientSymptomStatus(oldPatientId);
            updatePatientSymptomStatus(appointment->getPatientId());
            saveAll();
            dialog.accept();
        });
        dialog.exec();
    }

    // --- TRANG 5: LỊCH NGHỈ LỄ ---
    QWidget *holidayPage(QWidget *parent = nullptr) {
        QWidget *w = page("Lịch nghỉ", "Quản lý ngày nghỉ lễ áp dụng cho lịch đặt khám.", parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        auto *actions = new QHBoxLayout;
        auto *add = button("＋ Thêm ngày nghỉ");
        auto *edit = button("Sửa lịch nghỉ", "secondary");
        auto *remove = button("Xóa ngày nghỉ", "danger");

        if (!hasPermission("Per_5")) {
            add->hide();
            edit->hide();
            remove->hide();
        }

        actions->addWidget(add);
        actions->addWidget(edit);
        actions->addWidget(remove);
        actions->addStretch();
        layout->addLayout(actions);

        auto *t = table({"NGÀY NGHỈ", "GHI CHÚ"});
        layout->addWidget(t, 1);

        auto fill = [this, t]{
            t->setRowCount(0);
            ifstream file(holidayConfigPath);
            string line;
            while (getline(file, line)) {
                libPBL2::trim(line);
                if (line.empty()) continue;
                size_t split = line.find('-');
                string date = split == string::npos ? line : line.substr(0, split);
                string note = split == string::npos ? "Nghỉ lễ" : line.substr(split + 1);
                libPBL2::trim(date);
                libPBL2::trim(note);
                int row = t->rowCount();
                t->insertRow(row);
                cell(t, row, 0, qs(date), Qt::AlignCenter);
                cell(t, row, 1, qs(note));
            }
        };

        auto persist = [t]{
            ofstream file(holidayConfigPath);
            for (int r = 0; r < t->rowCount(); ++r) {
                file << ss(t->item(r, 0)->text()) << " - " << ss(t->item(r, 1)->text()) << "\n";
            }
        };

        auto editHoliday = [this, t, fill, persist](int row, bool isNew){
            QString oldDate = isNew ? QString() : t->item(row, 0)->text();
            QString oldNote = isNew ? QString() : t->item(row, 1)->text();

            QDialog dialog(this);
            dialog.setWindowTitle(isNew ? "Thêm ngày nghỉ" : "Chỉnh sửa ngày nghỉ");

            auto *v = new QVBoxLayout(&dialog);
            auto *f = new QFormLayout;

            auto *date = new QDateEdit(QDate::currentDate());
            date->setDisplayFormat("d/M");
            date->setCalendarPopup(true);

            if (!isNew) {
                auto parts = oldDate.split('/');
                if (parts.size() == 2) {
                    date->setDate(QDate(QDate::currentDate().year(), parts[1].toInt(), parts[0].toInt()));
                }
            }
            auto *note = new QLineEdit(oldNote);
            f->addRow("Ngày nghỉ (ngày/tháng)", date);
            f->addRow("Ghi chú", note);
            v->addLayout(f);

            auto *save = button("Lưu");
            v->addWidget(save);

            connect(save, &QPushButton::clicked, &dialog, [&dialog, t, date, note, row, isNew, fill, persist]{
                if (note->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dialog, "Thiếu ghi chú", "Vui lòng nhập ghi chú ngày nghỉ.");
                    return;
                }
                QString value = date->date().toString("d/M");
                if (isNew) {
                    int r = t->rowCount();
                    t->insertRow(r);
                    auto *item0 = new QTableWidgetItem(value);
                    item0->setTextAlignment(Qt::AlignCenter);
                    t->setItem(r, 0, item0);
                    t->setItem(r, 1, new QTableWidgetItem(note->text().trimmed()));
                } else {
                    t->item(row, 0)->setText(value);
                    t->item(row, 1)->setText(note->text().trimmed());
                }
                persist();
                fill();
                dialog.accept();
            });
            dialog.exec();
        };

        connect(add, &QPushButton::clicked, w, [editHoliday]{ editHoliday(0, true); });
        connect(edit, &QPushButton::clicked, w, [t, editHoliday]{
            if (t->currentRow() >= 0) editHoliday(t->currentRow(), false);
        });
        connect(remove, &QPushButton::clicked, w, [this, t, fill, persist]{
            int r = t->currentRow();
            if (r < 0) return;
            if (QMessageBox::question(this, "Xóa ngày nghỉ", "Xóa ngày nghỉ đã chọn này?") != QMessageBox::Yes) {
                return;
            }
            t->removeRow(r);
            persist();
            fill();
        });

        fill();
        return w;
    }

    // --- TRANG 6: QUẢN LÝ TÀI KHOẢN (ADMIN) ---
    QWidget *managerPage(QWidget *parent = nullptr) {
        QWidget *w = page("Tài khoản & cấu hình", "Quản trị viên HoangMA001 quản lý tài khoản, quyền hệ thống và lịch nghỉ lễ.", parent);
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());

        auto *actions = new QHBoxLayout;
        auto *add = button("＋  Thêm quản lý");
        auto *toggle = button("Khóa / mở khóa tài khoản", "secondary");
        actions->addWidget(add);
        actions->addWidget(toggle);
        actions->addStretch();
        layout->addLayout(actions);

        auto *t = table({"MÃ", "HỌ VÀ TÊN", "TÀI KHOẢN", "VAI TRÒ", "TRẠNG THÁI"});
        layout->addWidget(t, 1);
        managerTable = t;

        connect(add, &QPushButton::clicked, w, [this]{
            addManagerDialog();
            refreshManagers();
        });

        connect(toggle, &QPushButton::clicked, w, [this, t]{
            int row = t->currentRow();
            if (row < 0) {
                QMessageBox::information(this, "Chọn tài khoản", "Hãy chọn tài khoản cần khóa hoặc mở khóa.");
                return;
            }
            const string id = ss(t->item(row, 0)->text());
            if (id == "MA_001") {
                QMessageBox::warning(this, "Không thể thay đổi", "Không thể khóa tài khoản quản trị viên gốc.");
                return;
            }
            if (currentManager && id == currentManager->getID()) {
                QMessageBox::warning(this, "Không thể thay đổi", "Không thể khóa tài khoản đang đăng nhập.");
                return;
            }
            if (managers) {
                auto *n = managers->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getID() == id) {
                        bool lock = n->info.isActive();
                        auto answer = QMessageBox::question(this, lock ? "Khóa tài khoản" : "Mở khóa tài khoản",
                            QString("Bạn có muốn %1 tài khoản %2?").arg(lock ? "khóa" : "mở khóa", qs(id)));
                        if (answer == QMessageBox::Yes) {
                            n->info.setState(lock ? 0 : 1);
                            saveAll();
                            refreshManagerTable();
                        }
                        return;
                    }
                    n = n->next;
                }
            }
        });

        auto *grant = button("Cấp quyền cho tài khoản đã chọn", "secondary");
        if (!isRootAdmin()) grant->hide();
        layout->addWidget(grant, 0, Qt::AlignLeft);

        connect(grant, &QPushButton::clicked, w, [this, t]{
            int row = t->currentRow();
            if (row < 0) {
                QMessageBox::information(this, "Chọn tài khoản", "Hãy chọn tài khoản cần cấp quyền.");
                return;
            }
            grantPermissions(ss(t->item(row, 0)->text()));
        });

        auto *note = new QLabel("Danh sách quyền quản lý phân quyền thời hạn chỉ áp dụng khi tài khoản root cấp cho lễ tân.");
        note->setObjectName("muted");
        layout->addWidget(note);

        refreshManagerTable();
        return w;
    }

    void grantPermissions(const string &managerId) {
        if (!isRootAdmin()) {
            QMessageBox::warning(this, "Không có quyền", "Chỉ tài khoản HoangMA001 được cấp quyền.");
            return;
        }
        if (managerId == currentManager->getID()) return;

        set<string> granted;
        ifstream input(managerPermissionPath);
        string line;
        while (getline(input, line)) {
            stringstream row(line);
            string id, permission;
            if (!getline(row, id, '|') || !getline(row, permission, '|')) continue;
            libPBL2::trim(id);
            libPBL2::trim(permission);
            if (id == managerId) granted.insert(permission);
        }

        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("Cấp quyền · %1").arg(qs(managerId)));

        auto *layout = new QVBoxLayout(&dialog);
        auto *checks = new vector<pair<QCheckBox*, QSpinBox*>>;

        for (const auto &entry : permissionDefinitions) {
            auto *check = new QCheckBox(qs(entry.second));
            check->setChecked(granted.count(entry.first) > 0);
            check->setProperty("permissionId", qs(entry.first));
            layout->addWidget(check);

            QSpinBox *duration = nullptr;
            if (entry.first == "Per_1") {
                auto *row = new QHBoxLayout;
                row->addWidget(new QLabel("Thời hạn (ngày):"));
                duration = new QSpinBox;
                duration->setRange(1, 3650);
                duration->setValue(1);
                row->addWidget(duration);
                layout->addLayout(row);
                duration->setVisible(check->isChecked());
                connect(check, &QCheckBox::toggled, duration, &QWidget::setVisible);
            }
            checks->push_back({check, duration});
        }

        auto *save = button("Lưu quyền");
        layout->addWidget(save);

        connect(save, &QPushButton::clicked, &dialog, [this, &dialog, managerId, checks]{
            struct PermissionGrant { string id, permission, expiry; };
            vector<PermissionGrant> all;

            ifstream existing(managerPermissionPath);
            string line;
            while (getline(existing, line)) {
                stringstream row(line);
                string id, permission, expiry;
                if (!getline(row, id, '|') || !getline(row, permission, '|')) continue;
                getline(row, expiry);
                libPBL2::trim(id);
                libPBL2::trim(permission);
                libPBL2::trim(expiry);
                if (id.empty() || id == "MA_ID" || id.find('-') != string::npos || id == managerId) continue;
                all.push_back({id, permission, expiry});
            }

            for (const auto &entry : *checks) {
                if (entry.first->isChecked()) {
                    string expiry;
                    if (entry.second) {
                        expiry = ss(QDateTime::currentDateTime().addDays(entry.second->value()).toString(Qt::ISODate));
                    }
                    all.push_back({managerId, ss(entry.first->property("permissionId").toString()), expiry});
                }
            }

            ofstream output(managerPermissionPath);
            if (!output) {
                QMessageBox::warning(&dialog, "Lỗi lưu", "Không thể lưu danh sách quyền.");
                return;
            }
            output << "MA_ID | Per_ID | Expires_At\n-------+--------+--------------------\n";
            for (const auto &entry : all) {
                output << entry.id << " | " << entry.permission << " | " << entry.expiry << "\n";
            }

            dialog.accept();
            QMessageBox::information(this, "Đã cấp quyền", QStringLiteral("HoangMA001 đã cập nhật quyền cho tài khoản %1.").arg(qs(managerId)));
        });
        dialog.exec();
    }

    void refreshManagerTable() {
        if (!managerTable || !managers) return;
        managerTable->setRowCount(0);
        auto *n = managers->getHeader()->next;
        while (n && n->next) {
            int r = managerTable->rowCount();
            managerTable->insertRow(r);
            cell(managerTable, r, 0, qs(n->info.getID()));
            cell(managerTable, r, 1, qs(n->info.getfullName()));
            cell(managerTable, r, 2, qs(n->info.getUsername()));
            cell(managerTable, r, 3, roleName(n->info.getRole()));
            cell(managerTable, r, 4, n->info.isActive() ? "Hoạt động" : "Đã khóa", Qt::AlignCenter);
            n = n->next;
        }
    }

    void refreshManagers() { refreshManagerTable(); }

    void addManagerDialog() {
        QDialog dialog(this);
        dialog.setWindowTitle("Thêm tài khoản quản lý");

        auto *v = new QVBoxLayout(&dialog);
        auto *f = new QFormLayout;

        auto *name = new QLineEdit;
        auto *phone = new QLineEdit;
        auto *username = new QLineEdit;
        auto *password = new QLineEdit;
        password->setEchoMode(QLineEdit::Password);

        auto *age = new QSpinBox;
        age->setRange(18, 120);
        age->setValue(28);

        auto *gender = new QComboBox;
        gender->addItems({"Nam", "Nữ"});

        auto *role = new QComboBox;
        role->addItems({"Lễ tân", "Lễ tân chuyên trách", "Quản trị viên"});

        f->addRow("Họ tên", name);
        f->addRow("Điện thoại", phone);
        f->addRow("Tuổi", age);
        f->addRow("Giới tính", gender);
        f->addRow("Tên đăng nhập", username);
        f->addRow("Mật khẩu", password);
        f->addRow("Vai trò", role);
        v->addLayout(f);

        auto *save = button("Tạo tài khoản");
        v->addWidget(save);

        connect(save, &QPushButton::clicked, &dialog, [this, &dialog, name, phone, username, password, age, gender, role]{
            if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() ||
                username->text().trimmed().isEmpty() || password->text().isEmpty()) {
                QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đủ thông tin.");
                return;
            }
            if (managers) {
                auto *n = managers->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getUsername() == ss(username->text().trimmed())) {
                        QMessageBox::warning(&dialog, "Trùng tài khoản", "Tên đăng nhập đã được sử dụng.");
                        return;
                    }
                    n = n->next;
                }
            }

            int maximum = 0;
            if (managers) {
                auto *n = managers->getHeader()->next;
                while (n && n->next) {
                    string id = n->info.getID();
                    if (id.rfind("MA_", 0) == 0) {
                        try { maximum = max(maximum, stoi(id.substr(3))); } catch (...) {}
                    }
                    n = n->next;
                }
            }

            ostringstream id;
            id << "MA_" << setfill('0') << setw(3) << maximum + 1;
            ManagerRole r = role->currentIndex() == 2 ? ManagerRole::Administrator :
                           (role->currentIndex() == 1 ? ManagerRole::SpecialReceptionist : ManagerRole::Receptionist);

            if (managers) {
                managers->insert(Manager(id.str(), ss(name->text()), ss(phone->text()), gender->currentIndex() == 0, age->value(), ss(username->text()), ss(password->text()), r, 1));
            }
            saveAll();
            dialog.accept();
        });
        dialog.exec();
    }

    void refreshPage(int index) {
        if (!pages) return;
        if (index >= 0 && index <= 5) {
            QWidget *old = pages->widget(index);
            pages->removeWidget(old);
            delete old;

            QWidget *updated = nullptr;
            switch (index) {
                case 0: updated = dashboardPage(pages); break;
                case 1: updated = bookingPage(pages); break;
                case 2: updated = patientPage(pages); break;
                case 3: updated = doctorPage(pages); break;
                case 4: updated = appointmentPage(pages); break;
                case 5: updated = holidayPage(pages); break;
            }
            pages->insertWidget(index, updated);
        } else if (index == 6) {
            refreshManagers();
        }
    }

    void saveAll() {
        bool ok = Patient::saveToFile(patientPath, patients) &&
                  Manager::saveToFile(managerPath, managers) &&
                  Doctor::saveToFile(doctorPath, doctors) &&
                  SymptomPatient::saveToFile(symptomPatientPath, symptomPatients) &&
                  Appointment::saveToFile(appointmentPath, appointments);
        if (!ok) {
            QMessageBox::warning(this, "Lỗi lưu dữ liệu", "Không thể lưu một hoặc nhiều tệp dữ liệu.");
        }
    }

public:
    ClinicWindow() {
        setStyleSheet(style());
        setMinimumSize(1120, 720);
        resize(1360, 850);
        setWindowTitle("An Tâm · Quản lý phòng khám");

        QIcon appIcon("assets/textures/icon_app.png");
        if (!appIcon.isNull()) setWindowIcon(appIcon);
    }

    ~ClinicWindow() override {
        delete managers;
        delete patients;
        delete doctors;
        delete symptoms;
        delete symptomPatients;
        delete appointments;
    }

    bool initApp() {
        // Nạp dữ liệu từ các file
        managers = Manager::loadFromFile(managerPath);
        patients = Patient::loadFromFile(patientPath);
        doctors = Doctor::loadFromFile(doctorPath);
        symptoms = Symptom::loadFromFile(symptomPath);
        symptomPatients = SymptomPatient::loadFromFile(symptomPatientPath);
        appointments = Appointment::loadFromFile(appointmentPath);

        bool ok = managers && patients && doctors && symptoms && symptomPatients && appointments;
        if (!ok) return false;

        // Cập nhật trạng thái triệu chứng
        auto *p = patients->getHeader()->next;
        while (p && p->next) {
            updatePatientSymptomStatus(p->info.getID());
            p = p->next;
        }

        // Khóa màn hình Login: Tự động gán quyền Quản trị viên mặc định (HoangMA001)
        currentManager = nullptr;
        if (managers) {
            auto *node = managers->getHeader()->next;
            while (node && node->next) {
                if (node->info.getUsername() == "HoangMA001" || node->info.getID() == "MA_001") {
                    currentManager = &node->info;
                    break;
                }
                node = node->next;
            }
            if (!currentManager && managers->getHeader()->next != managers->getFooter()) {
                currentManager = &(managers->getHeader()->next->info);
            }
        }

        // Vào thẳng giao diện chính
        setupShell();
        return true;
    }
};

int runClinicApp(int argc, char **argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Phòng khám An Tâm");
    QApplication::setStyle("Fusion");

    // Nạp bộ font chữ đẹp Be Vietnam Pro
    QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Regular.ttf");
    QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Medium.ttf");
    QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-SemiBold.ttf");
    QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Bold.ttf");
    QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-ExtraBold.ttf");

    QFont appFont("Be Vietnam Pro", 10);
    appFont.setStyleHint(QFont::SansSerif);
    appFont.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(appFont);

    // Gán logo icon app
    QIcon appIcon("assets/textures/icon_app.png");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    ClinicWindow window;
    if (!window.initApp()) {
        QMessageBox::critical(nullptr, "Không tải được dữ liệu", "Không tìm thấy dữ liệu phòng khám. Hãy chạy ứng dụng từ thư mục gốc dự án.");
        return 1;
    }
    window.show();
    return app.exec();
}
