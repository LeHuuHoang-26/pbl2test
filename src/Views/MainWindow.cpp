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
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QMessageBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QDateTime>
#include <QDateEdit>
#include <QLocale>
#include <QStringList>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QFontDatabase>
#include <QDate>

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
#include "../Controllers/AuthController.h"

using namespace std;

static QString qs(const string &s) { return QString::fromUtf8(s.c_str()); }
static string ss(const QString &s) { return s.toUtf8().toStdString(); }
static const string patientPath = "Data/Patient.txt";
static const string managerPath = "Data/Manager.txt";
static const string appointmentPath = "Data/Appointment/Appointment.txt";
static const string symptomPatientPath = "Data/Symptom/SymptomPatient.txt";
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

class ClinicWindow : public QMainWindow {
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
    QLabel *userLabel = nullptr;
    int activeNav = 0;

    bool isRootAdmin() const {
        return currentManager && currentManager->getUsername() == "HoangMA001";
    }
    QDateTime permissionExpiry(const string &managerId, const string &permissionId) const {
        ifstream input(managerPermissionPath);
        string line;
        while (getline(input, line)) {
            stringstream row(line); string id, permission, expiry;
            if (!getline(row, id, '|') || !getline(row, permission, '|')) continue;
            getline(row, expiry);
            libPBL2::trim(id); libPBL2::trim(permission); libPBL2::trim(expiry);
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
    bool isAdministrator() const { return isRootAdmin() || isTemporaryAdmin(); }
    bool hasPermission(const string &permissionId) const {
        if (!currentManager) return false;
        if (isRootAdmin()) return true;
        if (isTemporaryAdmin()) return true;
        if (permissionId == "Per_1") return false;
        ifstream input(managerPermissionPath);
        string line;
        while (getline(input, line)) {
            stringstream row(line); string id, permission;
            if (!getline(row, id, '|') || !getline(row, permission, '|')) continue;
            libPBL2::trim(id); libPBL2::trim(permission);
            if (id == currentManager->getID() && permission == permissionId) return true;
        }
        return false;
    }

    QString specialty(const string &id) const {
        ifstream file("Data/Doctor/Specialization.txt");
        string line; getline(file, line); getline(file, line);
        while (getline(file, line)) {
            string key, value; stringstream row(line);
            if (getline(row, key, '|') && getline(row, value)) {
                libPBL2::trim(key); libPBL2::trim(value);
                if (key == id) return qs(value);
            }
        }
        return qs(id);
    }
    int countPatients() const { return countList(patients); }
    int countDoctors() const { return countList(doctors); }
    int countAppointments() const { return countList(appointments); }
    template<class T> int countList(const LinkList<T> *list) const {
        int count = 0; if (!list) return 0;
        auto *n = list->getHeader()->next;
        while (n && n->next) { if (!n->info.isDefault()) ++count; n = n->next; }
        return count;
    }
    Patient *findPatient(const string &id) const {
        auto *n = patients->getHeader()->next;
        while (n && n->next) { if (n->info.getID() == id) return &n->info; n = n->next; }
        return nullptr;
    }
    Doctor *findDoctor(const string &id) const {
        auto *n = doctors->getHeader()->next;
        while (n && n->next) { if (n->info.getID() == id) return &n->info; n = n->next; }
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
            if (parsed >> day >> slash >> month && slash == '/' && day == date.day() && month == date.month()) return true;
        }
        return false;
    }
    QString patientSymptoms(const string &patientId) const {
        QStringList names;
        auto *record = symptomPatients->getHeader()->next;
        while (record && record->next) {
            if (record->info.getP_id() == patientId) {
                stringstream ids(record->info.getS_id()); string id;
                while (getline(ids, id, ',')) {
                    libPBL2::trim(id); auto *s = symptoms->getHeader()->next;
                    while (s && s->next) { if (s->info.getS_id() == id) { names << qs(s->info.getSName()); break; } s = s->next; }
                }
            }
            record = record->next;
        }
        names.removeDuplicates();
        return names.isEmpty() ? QStringLiteral("Chưa ghi nhận") : names.join(QStringLiteral(", "));
    }
    QString unresolvedPatientSymptoms(const string &patientId) const {
        QStringList names;
        auto *record = symptomPatients->getHeader()->next;
        while (record && record->next) {
            if (record->info.getP_id() == patientId) {
                stringstream ids(record->info.getS_id()); string id;
                while (getline(ids, id, ',')) {
                    libPBL2::trim(id);
                    bool hasSpecificAppointment=false, specificPending=false, hasLegacyAppointment=false, legacyPending=false;
                    auto *a=appointments->getHeader()->next;while(a&&a->next){if(a->info.getPatientId()==patientId){if(a->info.getSymptomId()==id){hasSpecificAppointment=true;if(!a->info.getState())specificPending=true;}else if(a->info.getSymptomId().empty()){hasLegacyAppointment=true;if(!a->info.getState())legacyPending=true;}}a=a->next;}
                    if((hasSpecificAppointment&&!specificPending)||(!hasSpecificAppointment&&hasLegacyAppointment&&!legacyPending))continue;
                    if(!hasSpecificAppointment&&!hasLegacyAppointment&&record->info.isResolved())continue;
                    auto *s = symptoms->getHeader()->next;
                    while (s && s->next) { if (s->info.getS_id() == id) { names << qs(s->info.getSName()); break; } s = s->next; }
                }
            }
            record = record->next;
        }
        names.removeDuplicates();
        return names.isEmpty() ? QStringLiteral("Đã khám hết") : QStringLiteral("Chưa khám: %1").arg(names.join(QStringLiteral(", ")));
    }
    void updatePatientSymptomStatus(const string &patientId) {
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
    QString style() const {
        return QStringLiteral(R"(
            QWidget { font-family: 'Segoe UI'; font-size: 14px; color: #183044; }
            QMainWindow, QWidget#appRoot { background: #f4f7fb; }
            QLabel#muted { color: #8091a4; }
            QLabel#pageTitle { font-size: 26px; font-weight: 700; color: #17334a; }
            QLabel#pageSubtitle { font-size: 13px; color: #7d8da0; }
            QFrame#sidebar { background: #102f43; border: 0; }
            QLabel#brand { color: white; font-size: 20px; font-weight: 750; }
            QLabel#brandSub { color: #9db7c6; font-size: 11px; }
            QPushButton#nav { color: #bfd0d9; text-align: left; padding: 13px 16px; border: 0; border-radius: 9px; background: transparent; font-size: 14px; }
            QPushButton#nav:hover { color: white; background: #1b465d; }
            QPushButton#nav[active="true"] { color: white; background: #226b73; font-weight: 650; }
            QPushButton#primary { background: #167d79; color: white; border: 0; border-radius: 9px; padding: 11px 18px; font-weight: 650; }
            QPushButton#primary:hover { background: #106d69; }
            QPushButton#secondary { background: white; color: #355166; border: 1px solid #dce5eb; border-radius: 9px; padding: 10px 16px; }
            QPushButton#danger { background: #fff0ef; color: #b94b45; border: 0; border-radius: 8px; padding: 9px 14px; }
            QLineEdit, QComboBox, QSpinBox, QDateTimeEdit { background: white; border: 1px solid #d8e2e9; border-radius: 8px; padding: 10px 12px; min-height: 20px; selection-background-color: #167d79; }
            QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDateTimeEdit:focus { border: 1px solid #167d79; }
            QTableWidget { background: white; border: 1px solid #e3eaf0; border-radius: 11px; gridline-color: #eff3f6; selection-background-color: #e1f3f1; selection-color: #17334a; }
            QHeaderView::section { background: #f7f9fb; color: #75889a; border: 0; border-bottom: 1px solid #e6edf2; padding: 12px 9px; font-size: 12px; font-weight: 700; }
            QTableWidget::item { padding: 9px; }
            QFrame#card, QFrame#panel { background: white; border: 1px solid #e8eef2; border-radius: 13px; }
            QLabel#metricValue { color: #15384f; font-size: 28px; font-weight: 750; }
            QLabel#metricLabel { color: #8292a1; font-size: 12px; }
            QLabel#pill { background: #e6f5f2; color: #14756e; border-radius: 10px; padding: 5px 9px; font-size: 11px; font-weight: 700; }
            QCheckBox { spacing: 9px; }
        )");
    }
    QWidget *page(QString title, QString subtitle) {
        QWidget *container = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->setContentsMargins(32, 26, 32, 26); layout->setSpacing(20);
        QLabel *heading = new QLabel(title); heading->setObjectName("pageTitle");
        QLabel *description = new QLabel(subtitle); description->setObjectName("pageSubtitle");
        layout->addWidget(heading); layout->addWidget(description);
        return container;
    }
    QFrame *panel() {
        QFrame *frame = new QFrame; frame->setObjectName("panel");
        auto *shadow = new QGraphicsDropShadowEffect(frame); shadow->setBlurRadius(18); shadow->setOffset(0, 4); shadow->setColor(QColor(20, 48, 65, 12)); frame->setGraphicsEffect(shadow);
        return frame;
    }
    QPushButton *button(QString text, QString kind = "primary") {
        auto *b = new QPushButton(text); b->setObjectName(kind); b->setCursor(Qt::PointingHandCursor); return b;
    }
    QTableWidget *table(QStringList headers) {
        auto *t = new QTableWidget; t->setColumnCount(headers.size()); t->setHorizontalHeaderLabels(headers);
        t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); t->verticalHeader()->hide();
        t->setAlternatingRowColors(false); t->setEditTriggers(QAbstractItemView::NoEditTriggers); t->setSelectionBehavior(QAbstractItemView::SelectRows); t->setShowGrid(false);
        return t;
    }
    void cell(QTableWidget *t, int row, int column, const QString &text) {
        auto *item = new QTableWidgetItem(text); item->setFlags(item->flags() & ~Qt::ItemIsEditable); t->setItem(row, column, item);
    }
    void setPage(int index) { activeNav = index; pages->setCurrentIndex(index); updateNavigation(); }
    void updateNavigation() {
        for (int i = 0; i < sidebar->findChildren<QPushButton *>().size(); ++i) {
            auto *b = sidebar->findChildren<QPushButton *>()[i]; b->setProperty("active", i == activeNav); b->style()->unpolish(b); b->style()->polish(b);
        }
    }
    void setupShell() {
        root = new QWidget; root->setObjectName("appRoot"); setCentralWidget(root);
        auto *outer = new QHBoxLayout(root); outer->setContentsMargins(0, 0, 0, 0); outer->setSpacing(0);
        sidebar = new QFrame; sidebar->setObjectName("sidebar"); sidebar->setFixedWidth(238);
        auto *side = new QVBoxLayout(sidebar); side->setContentsMargins(18, 24, 18, 20); side->setSpacing(8);
        auto *brand = new QLabel("✦  AN TÂM"); brand->setObjectName("brand");
        auto *brandSub = new QLabel("PHÒNG KHÁM ĐA KHOA"); brandSub->setObjectName("brandSub");
        side->addWidget(brand); side->addWidget(brandSub); side->addSpacing(27);
        const QStringList labels = {"⌂   Tổng quan", "＋   Đặt lịch khám", "♡   Bệnh nhân", "✚   Bác sĩ", "▤   Quản lý lịch khám", "▧   Lịch nghỉ"};
        for (int i = 0; i < labels.size(); ++i) { auto *b = button(labels[i], "nav"); connect(b, &QPushButton::clicked, this, [this, i]{ refreshPage(i); setPage(i); }); side->addWidget(b); }
        if (isAdministrator()) {
            auto *b = button("♙   Quản lý tài khoản", "nav"); connect(b, &QPushButton::clicked, this, [this]{ refreshManagers(); setPage(6); }); side->addWidget(b);
        }
        side->addStretch();
        userLabel = new QLabel; userLabel->setStyleSheet("color:#e6f0f4; padding:8px 4px;"); side->addWidget(userLabel);
        auto *logout = button("↪   Đăng xuất", "nav"); connect(logout, &QPushButton::clicked, this, [this]{ saveAll(); currentManager = nullptr; showLogin(); }); side->addWidget(logout);
        pages = new QStackedWidget;
        outer->addWidget(sidebar); outer->addWidget(pages, 1);
        pages->addWidget(dashboardPage()); pages->addWidget(bookingPage()); pages->addWidget(patientPage()); pages->addWidget(doctorPage()); pages->addWidget(appointmentPage()); pages->addWidget(holidayPage());
        if (isAdministrator()) pages->addWidget(managerPage());
        userLabel->setText(QStringLiteral("●  %1\n     %2").arg(qs(currentManager->getfullName()), qs(currentManager->getID())));
        updateNavigation(); setPage(0);
    }
    QWidget *dashboardPage() {
        QWidget *w = page("Chào buổi sáng 👋", "Chào mừng bạn đến với hệ thống quản lý Phòng khám An Tâm.");
        auto *layout = qobject_cast<QVBoxLayout *>(w->layout());
        auto *metrics = new QGridLayout; metrics->setSpacing(15);
        const QString labels[] = {"BỆNH NHÂN", "BÁC SĨ", "LỊCH KHÁM", "CHƯA KHÁM"};
        const QString values[] = {QString::number(countPatients()), QString::number(countDoctors()), QString::number(countAppointments()), QString::number(pendingAppointments())};
        for (int i = 0; i < 4; ++i) {
            auto *card = panel(); card->setObjectName("card"); auto *v = new QVBoxLayout(card); v->setContentsMargins(19, 18, 19, 18);
            auto *number = new QLabel(values[i]); number->setObjectName("metricValue"); auto *caption = new QLabel(labels[i]); caption->setObjectName("metricLabel"); v->addWidget(number); v->addWidget(caption); metrics->addWidget(card, 0, i);
        }
        layout->addLayout(metrics);
        auto *welcome = panel(); auto *box = new QHBoxLayout(welcome); box->setContentsMargins(24, 24, 24, 24);
        auto *copy = new QVBoxLayout; auto *title = new QLabel("Chăm sóc chu đáo, bắt đầu từ một lịch hẹn tốt."); title->setStyleSheet("font-size:20px;font-weight:700;color:#17384d;");
        auto *sub = new QLabel("Tạo lịch khám mới và chọn bác sĩ phù hợp với triệu chứng của bệnh nhân."); sub->setObjectName("muted"); copy->addWidget(title); copy->addWidget(sub);
        auto *book = button("Đặt lịch khám  →"); connect(book, &QPushButton::clicked, this, [this]{ setPage(1); }); box->addLayout(copy, 1); box->addWidget(book); layout->addWidget(welcome);
        auto *recent = panel(); auto *recentLayout = new QVBoxLayout(recent); recentLayout->setContentsMargins(18, 16, 18, 16);
        auto *titleRecent = new QLabel("Lịch hẹn sắp tới"); titleRecent->setStyleSheet("font-size:16px;font-weight:700;"); recentLayout->addWidget(titleRecent);
        auto *t = table({"MÃ LỊCH", "BỆNH NHÂN", "BÁC SĨ", "THỜI GIAN", "TRẠNG THÁI"}); fillAppointmentTable(t, true); recentLayout->addWidget(t); layout->addWidget(recent, 1); return w;
    }
    int pendingAppointments() const {
        int count = 0; auto *n = appointments->getHeader()->next;
        while (n && n->next) { if (n->info.getState() == 0) ++count; n = n->next; } return count;
    }
    void fillAppointmentTable(QTableWidget *t, bool upcomingOnly) {
        t->setRowCount(0); vector<Appointment *> all; auto *n = appointments->getHeader()->next;
        const QDate today = QDate::currentDate();
        while (n && n->next) {
            if (!n->info.isDefault()) {
                Date d = n->info.getDateGet();
                QDate appointmentDay(d.getYear(), d.getMonth(), d.getDay());
                if (!upcomingOnly || (appointmentDay >= today && appointmentDay <= today.addDays(1))) all.push_back(&n->info);
            }
            n = n->next;
        }
        sort(all.begin(), all.end(), [](Appointment *a, Appointment *b){ return a->getDateGet().toString() < b->getDateGet().toString(); });
        int limit = all.size();
        for (int i = 0; i < limit; ++i) {
            auto *a = all[i]; int row = t->rowCount(); t->insertRow(row); Patient *p = findPatient(a->getPatientId()); Doctor *d = findDoctor(a->getDoctorId());
            cell(t,row,0,qs(a->getId())); cell(t,row,1,p ? qs(p->getfullName()) : qs(a->getPatientId())); cell(t,row,2,d ? qs(d->getfullName()) : qs(a->getDoctorId())); cell(t,row,3,qs(a->getDateGet().toString())); cell(t,row,4,a->getState() ? QStringLiteral("Đã khám") : QStringLiteral("Chưa khám"));
        }
    }
    QWidget *bookingPage() {
        QWidget *w = page("Đặt lịch khám", "Chọn hồ sơ bệnh nhân, triệu chứng và bác sĩ phù hợp."); auto *layout = qobject_cast<QVBoxLayout *>(w->layout());
        auto *card = panel(); auto *form = new QFormLayout(card); form->setContentsMargins(25, 23, 25, 23); form->setSpacing(14);
        auto *patient = new QComboBox; auto *p = patients->getHeader()->next; while (p && p->next) { patient->addItem(QStringLiteral("%1  ·  %2").arg(qs(p->info.getID()), qs(p->info.getfullName())), qs(p->info.getID())); p = p->next; }
        auto *symptom = new QComboBox; symptom->addItem("Chọn triệu chứng chính…", ""); auto *s = symptoms->getHeader()->next; while (s && s->next) { symptom->addItem(QStringLiteral("%1  ·  %2").arg(qs(s->info.getSName()), specialty(s->info.getSp_id())), qs(s->info.getS_id())); s = s->next; }
        auto *doctor = new QComboBox; doctor->addItem("Chọn bác sĩ…", ""); auto *fee = new QLabel("Chọn bác sĩ để xem phí khám"); fee->setObjectName("muted"); auto *date = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1)); date->setDisplayFormat("dd/MM/yyyy  HH:mm"); date->setCalendarPopup(true);
        form->addRow("Bệnh nhân", patient); form->addRow("Triệu chứng", symptom); form->addRow("Bác sĩ khám", doctor); form->addRow("Phí khám", fee); form->addRow("Ngày giờ hẹn", date);
        QObject::connect(symptom, qOverload<int>(&QComboBox::currentIndexChanged), w, [this, symptom, doctor, fee]{ doctor->clear(); doctor->addItem("Chọn bác sĩ…", ""); fee->setText("Chọn bác sĩ để xem phí khám"); auto *s = symptoms->getHeader()->next; string spid; while (s && s->next) { if (qs(s->info.getS_id()) == symptom->currentData().toString()) { spid = s->info.getSp_id(); break; } s = s->next; }
            auto *d = doctors->getHeader()->next; while (d && d->next) { if (d->info.getState() && (spid.empty() || d->info.getSpId() == spid)) doctor->addItem(QStringLiteral("%1  ·  %2  ·  %3").arg(qs(d->info.getID()), qs(d->info.getfullName()), specialty(d->info.getSpId())), qs(d->info.getID())); d = d->next; }
        });
        connect(doctor, qOverload<int>(&QComboBox::currentIndexChanged), w, [this, doctor, fee]{ Doctor *selected = findDoctor(ss(doctor->currentData().toString())); fee->setText(selected ? QLocale(QLocale::Vietnamese).toString(selected->getFee(), 'f', 0) + " ₫" : "Chọn bác sĩ để xem phí khám"); });
        auto *save = button("Xác nhận đặt lịch"); save->setVisible(hasPermission("Per_4")); connect(save, &QPushButton::clicked, w, [this, patient, symptom, doctor, date]{
            if (patient->currentIndex() < 0 || doctor->currentData().toString().isEmpty() || symptom->currentData().toString().isEmpty()) { QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng chọn bệnh nhân, triệu chứng và bác sĩ."); return; }
            if (date->dateTime() <= QDateTime::currentDateTime()) { QMessageBox::warning(this, "Thời gian không hợp lệ", "Vui lòng chọn thời gian trong tương lai."); return; }
            if (isHoliday(date->date())) { QMessageBox::warning(this, "Ngày nghỉ lễ", "Phòng khám nghỉ lễ vào ngày này. Vui lòng chọn ngày khác."); return; }
            string pId = ss(patient->currentData().toString()), dId = ss(doctor->currentData().toString());
            auto *s = symptoms->getHeader()->next; string symptomId = ss(symptom->currentData().toString());
            appointments->insert(Appointment(nextAppointmentId(), pId, dId, Date::now(), Date::loadFromString(ss(date->dateTime().toString("yyyy-MM-dd HH:mm"))), 0, symptomId));
            while (s && s->next) { if (s->info.getS_id() == symptomId) { symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients), symptomId, pId, Date::now())); break; } s = s->next; }
            saveAll(); QMessageBox::information(this, "Đã đặt lịch", "Lịch khám đã được lưu thành công."); refreshPage(4); refreshPage(0);
        });
        layout->addWidget(card); layout->addWidget(save, 0, Qt::AlignLeft); layout->addStretch(); return w;
    }
    string nextAppointmentId() const {
        int maxId = 0; auto *n = appointments->getHeader()->next; while (n && n->next) { string id = n->info.getId(); if (id.rfind("A_",0)==0) try { maxId = max(maxId, stoi(id.substr(2))); } catch (...) {} n = n->next; }
        ostringstream out; out << "A_" << setfill('0') << setw(3) << maxId + 1; return out.str();
    }
    QWidget *patientPage() {
        QWidget *w = page("Bệnh nhân", "Quản lý hồ sơ và thông tin triệu chứng của người bệnh."); auto *layout = qobject_cast<QVBoxLayout *>(w->layout());
        auto *actions = new QHBoxLayout; auto *search = new QLineEdit; search->setPlaceholderText("Tìm theo tên, mã, điện thoại hoặc CCCD…"); auto *add = button("＋  Thêm bệnh nhân"); auto *edit = button("Sửa hồ sơ", "secondary"); auto *remove = button("Xóa đã khám hết", "danger"); actions->addWidget(search,1); actions->addWidget(add); actions->addWidget(edit); actions->addWidget(remove); layout->addLayout(actions);
        edit->setVisible(hasPermission("Per_3")); remove->setVisible(hasPermission("Per_3")); add->setVisible(hasPermission("Per_3"));
        auto *t = table({"MÃ HỒ SƠ", "HỌ VÀ TÊN", "ĐIỆN THOẠI", "CCCD", "TUỔI", "GIỚI TÍNH", "TRIỆU CHỨNG"});
        auto fill = [this,t,search]{ t->setRowCount(0); QString q = search->text().trimmed().toLower(); auto *n = patients->getHeader()->next; while (n && n->next) { QString name = qs(n->info.getfullName()), id = qs(n->info.getID()), phone = qs(n->info.getPhone()), cccd = qs(n->info.getCCCD()); if (q.isEmpty() || name.toLower().contains(q) || id.toLower().contains(q) || phone.contains(q) || cccd.contains(q)) { int r=t->rowCount(); t->insertRow(r); cell(t,r,0,id); cell(t,r,1,name); cell(t,r,2,phone); cell(t,r,3,cccd); cell(t,r,4,QString::number(n->info.getAge())); cell(t,r,5,n->info.getGender()?"Nam":"Nữ"); cell(t,r,6,unresolvedPatientSymptoms(n->info.getID())); } n=n->next; }};
        connect(search, &QLineEdit::textChanged, w, [fill]{fill();}); fill(); layout->addWidget(t,1);
        connect(add, &QPushButton::clicked, w, [this,fill]{ addPatientDialog(); fill(); });
        connect(edit, &QPushButton::clicked, w, [this,t,fill]{ int row=t->currentRow(); if(row<0)return; editPatientDialog(ss(t->item(row,0)->text())); fill(); });
        connect(remove, &QPushButton::clicked, w, [this,t,fill]{ int row=t->currentRow(); if(row<0)return; string id=ss(t->item(row,0)->text()); if(unresolvedPatientSymptoms(id)!=QStringLiteral("Đã khám hết")){QMessageBox::warning(this,"Chưa thể xóa","Chỉ có thể xóa hồ sơ đã khám hết các triệu chứng.");return;} if(QMessageBox::question(this,"Xóa bệnh nhân","Xóa hồ sơ bệnh nhân đã khám hết?")!=QMessageBox::Yes)return; auto*n=patients->getHeader()->next;while(n&&n->next){if(n->info.getID()==id){patients->remove(n);break;}n=n->next;} auto*s=symptomPatients->getHeader()->next;while(s&&s->next){auto*next=s->next;if(s->info.getP_id()==id)symptomPatients->remove(s);s=next;} auto*a=appointments->getHeader()->next;while(a&&a->next){auto*next=a->next;if(a->info.getPatientId()==id)appointments->remove(a);a=next;}saveAll();fill(); }); return w;
    }
    void addPatientDialog() {
        QDialog dialog(this); dialog.setWindowTitle("Thêm bệnh nhân"); dialog.setMinimumWidth(440); auto *v = new QVBoxLayout(&dialog);
        auto *form = new QFormLayout; auto *name = new QLineEdit; auto *phone = new QLineEdit; auto *cccd = new QLineEdit; auto *age = new QSpinBox; age->setRange(1,130); auto *gender = new QComboBox; gender->addItems({"Nam","Nữ"});
        auto *symptomChecks = new QWidget; auto *checks = new QVBoxLayout(symptomChecks); vector<QCheckBox *> boxes; auto *s = symptoms->getHeader()->next; while (s && s->next) { auto *check = new QCheckBox(QStringLiteral("%1  ·  %2").arg(qs(s->info.getSName()), specialty(s->info.getSp_id()))); check->setProperty("sid", qs(s->info.getS_id())); checks->addWidget(check); boxes.push_back(check); s = s->next; }
        auto *scroll = new QScrollArea; scroll->setWidgetResizable(true); scroll->setWidget(symptomChecks); scroll->setFixedHeight(150);
        form->addRow("Họ và tên",name); form->addRow("Điện thoại",phone); form->addRow("CCCD",cccd); form->addRow("Tuổi",age); form->addRow("Giới tính",gender); form->addRow("Triệu chứng",scroll); v->addLayout(form);
        auto *buttons = new QHBoxLayout; buttons->addStretch(); auto *cancel=button("Hủy","secondary"); auto *save=button("Tạo hồ sơ"); buttons->addWidget(cancel); buttons->addWidget(save); v->addLayout(buttons);
        connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
        connect(save,&QPushButton::clicked,&dialog,[this,&dialog,name,phone,cccd,age,gender,boxes]{ if(name->text().trimmed().isEmpty()||phone->text().trimmed().isEmpty()||cccd->text().trimmed().isEmpty()){QMessageBox::warning(&dialog,"Thiếu thông tin","Vui lòng nhập đủ họ tên, điện thoại và CCCD.");return;}
            auto *n=patients->getHeader()->next;while(n&&n->next){if(n->info.getPhone()==ss(phone->text().trimmed())||n->info.getCCCD()==ss(cccd->text().trimmed())){QMessageBox::warning(&dialog,"Trùng hồ sơ","Điện thoại hoặc CCCD đã tồn tại.");return;}n=n->next;}
            string id=Patient::generateNextId(patients); patients->insert(Patient(id,ss(name->text().trimmed()),ss(phone->text().trimmed()),gender->currentIndex()==0,age->value(),ss(cccd->text().trimmed())));
            string ids; for(auto *box:boxes)if(box->isChecked()){if(!ids.empty())ids+=", ";ids+=ss(box->property("sid").toString());}
            if(!ids.empty()) symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients),ids,id,Date::now()));
            saveAll(); dialog.accept();
        }); dialog.exec();
    }
    void editPatientDialog(const string &id) {
        Patient *patient = findPatient(id); if (!patient || !hasPermission("Per_3")) return;
        QDialog dialog(this); dialog.setWindowTitle("Chỉnh sửa bệnh nhân"); auto *layout = new QVBoxLayout(&dialog); auto *form = new QFormLayout;
        auto *name = new QLineEdit(qs(patient->getfullName())); auto *phone = new QLineEdit(qs(patient->getPhone())); auto *cccd = new QLineEdit(qs(patient->getCCCD())); auto *age = new QSpinBox; age->setRange(1,130); age->setValue(patient->getAge()); auto *gender = new QComboBox; gender->addItems({"Nam","Nữ"}); gender->setCurrentIndex(patient->getGender()?0:1);
        form->addRow("Họ và tên",name); form->addRow("Điện thoại",phone); form->addRow("CCCD",cccd); form->addRow("Tuổi",age); form->addRow("Giới tính",gender); layout->addLayout(form);
        auto *save=button("Lưu thay đổi");layout->addWidget(save);connect(save,&QPushButton::clicked,&dialog,[this,&dialog,patient,name,phone,cccd,age,gender]{if(name->text().trimmed().isEmpty()||phone->text().trimmed().isEmpty()||cccd->text().trimmed().isEmpty()){QMessageBox::warning(&dialog,"Thiếu thông tin","Vui lòng nhập đủ họ tên, điện thoại và CCCD.");return;}auto*n=patients->getHeader()->next;while(n&&n->next){if(&n->info!=patient&&(n->info.getPhone()==ss(phone->text().trimmed())||n->info.getCCCD()==ss(cccd->text().trimmed()))){QMessageBox::warning(&dialog,"Trùng hồ sơ","Điện thoại hoặc CCCD đã tồn tại.");return;}n=n->next;}patient->setfullName(ss(name->text().trimmed()));patient->setPhone(ss(phone->text().trimmed()));patient->setCCCD(ss(cccd->text().trimmed()));patient->setAge(age->value());patient->setGender(gender->currentIndex()==0);saveAll();dialog.accept();});dialog.exec();
    }
    QWidget *doctorPage() {
        QWidget *w=page("Đội ngũ bác sĩ","Danh sách bác sĩ và chuyên khoa đang hoạt động tại phòng khám."); auto *layout=qobject_cast<QVBoxLayout *>(w->layout()); auto *actions=new QHBoxLayout; auto *search=new QLineEdit;search->setPlaceholderText("Tìm bác sĩ hoặc chuyên khoa…");auto*edit=button("Sửa thông tin", "secondary");auto*remove=button("Xóa bác sĩ đã nghỉ", "danger");edit->setVisible(hasPermission("Per_2"));remove->setVisible(hasPermission("Per_2"));actions->addWidget(search,1);actions->addWidget(edit);actions->addWidget(remove);layout->addLayout(actions);
        auto *t=table({"MÃ BÁC SĨ","BÁC SĨ","CHUYÊN KHOA","PHÒNG","PHÍ KHÁM","ĐÁNH GIÁ","TRẠNG THÁI"});
        auto fill=[this,t,search]{t->setRowCount(0);QString q=search->text().trimmed().toLower();auto*n=doctors->getHeader()->next;while(n&&n->next){QString name=qs(n->info.getfullName()),spec=specialty(n->info.getSpId());if(q.isEmpty()||name.toLower().contains(q)||spec.toLower().contains(q)){int r=t->rowCount();t->insertRow(r);cell(t,r,0,qs(n->info.getID()));cell(t,r,1,name);cell(t,r,2,spec);cell(t,r,3,qs(n->info.getRoomId()));cell(t,r,4,QLocale(QLocale::Vietnamese).toString(n->info.getFee(),'f',0)+" ₫");cell(t,r,5,QString::number(n->info.getRating(),'f',1)+" ★");cell(t,r,6,n->info.getState()?"Đang nhận khám":"Tạm nghỉ");}n=n->next;}};
        connect(search,&QLineEdit::textChanged,w,[fill]{fill();});fill();layout->addWidget(t,1);
        connect(edit,&QPushButton::clicked,w,[this,t,fill]{int row=t->currentRow();if(row<0)return;editDoctorDialog(ss(t->item(row,0)->text()));fill();});
        connect(remove,&QPushButton::clicked,w,[this,t,fill]{int row=t->currentRow();if(row<0)return;string id=ss(t->item(row,0)->text());Doctor*d=findDoctor(id);if(!d||d->getState()){QMessageBox::warning(this,"Chưa thể xóa","Chỉ có thể xóa bác sĩ đã nghỉ việc.");return;}if(QMessageBox::question(this,"Xóa bác sĩ","Xóa hồ sơ bác sĩ đã nghỉ việc?")!=QMessageBox::Yes)return;auto*n=doctors->getHeader()->next;while(n&&n->next){auto*next=n->next;if(n->info.getID()==id)doctors->remove(n);n=next;}saveAll();fill();});return w;
    }
    void editDoctorDialog(const string &id) {
        Doctor *doctor=findDoctor(id);if(!doctor||!hasPermission("Per_2"))return;QDialog dialog(this);dialog.setWindowTitle("Chỉnh sửa bác sĩ");auto*layout=new QVBoxLayout(&dialog);auto*form=new QFormLayout;auto*name=new QLineEdit(qs(doctor->getfullName()));auto*phone=new QLineEdit(qs(doctor->getPhone()));auto*room=new QLineEdit(qs(doctor->getRoomId()));auto*fee=new QDoubleSpinBox;fee->setRange(0,100000000);fee->setDecimals(0);fee->setValue(doctor->getFee());auto*rating=new QDoubleSpinBox;rating->setRange(0,5);rating->setSingleStep(0.1);rating->setValue(doctor->getRating());auto*age=new QSpinBox;age->setRange(18,120);age->setValue(doctor->getAge());auto*gender=new QComboBox;gender->addItems({"Nam","Nữ"});gender->setCurrentIndex(doctor->getGender()?0:1);auto*specialtyBox=new QComboBox;ifstream specs("Data/Doctor/Specialization.txt");string line;getline(specs,line);getline(specs,line);while(getline(specs,line)){auto split=line.find('|');if(split==string::npos)continue;string sid=line.substr(0,split),label=line.substr(split+1);libPBL2::trim(sid);libPBL2::trim(label);specialtyBox->addItem(qs(label),qs(sid));}specialtyBox->setCurrentIndex(specialtyBox->findData(qs(doctor->getSpId())));auto*state=new QComboBox;state->addItems({"Đang làm việc","Đã nghỉ việc"});state->setCurrentIndex(doctor->getState()?0:1);form->addRow("Họ và tên",name);form->addRow("Điện thoại",phone);form->addRow("Tuổi",age);form->addRow("Giới tính",gender);form->addRow("Chuyên khoa",specialtyBox);form->addRow("Phòng",room);form->addRow("Phí khám",fee);form->addRow("Đánh giá",rating);form->addRow("Trạng thái",state);layout->addLayout(form);auto*save=button("Lưu thay đổi");layout->addWidget(save);connect(save,&QPushButton::clicked,&dialog,[this,&dialog,doctor,name,phone,room,fee,rating,age,gender,specialtyBox,state]{if(name->text().trimmed().isEmpty()||phone->text().trimmed().isEmpty()){QMessageBox::warning(&dialog,"Thiếu thông tin","Vui lòng nhập họ tên và điện thoại.");return;}doctor->setfullName(ss(name->text().trimmed()));doctor->setPhone(ss(phone->text().trimmed()));doctor->setAge(age->value());doctor->setGender(gender->currentIndex()==0);doctor->setSpId(ss(specialtyBox->currentData().toString()));doctor->setRoomId(ss(room->text().trimmed()));doctor->setFee(fee->value());doctor->setRating(rating->value());doctor->setState(state->currentIndex()==0);saveAll();dialog.accept();});dialog.exec();
    }
    QWidget *appointmentPage() {
        QWidget *w=page("Quản lý lịch khám","Theo dõi lịch theo bác sĩ, ngày hẹn và trạng thái khám.");auto *layout=qobject_cast<QVBoxLayout *>(w->layout());auto *filters=new QHBoxLayout;auto *doctor=new QComboBox;doctor->addItem("Tất cả bác sĩ","");auto*d=doctors->getHeader()->next;while(d&&d->next){doctor->addItem(QStringLiteral("%1 · %2").arg(qs(d->info.getID()),qs(d->info.getfullName())),qs(d->info.getID()));d=d->next;}auto *date=new QDateEdit;date->setCalendarPopup(true);date->setDisplayFormat("dd/MM/yyyy");auto *allDates=new QCheckBox("Tất cả ngày");allDates->setChecked(true);auto *status=new QComboBox;status->addItems({"Tất cả trạng thái","Chưa khám","Đã khám"});filters->addWidget(new QLabel("Bác sĩ"));filters->addWidget(doctor,1);filters->addWidget(new QLabel("Ngày hẹn"));filters->addWidget(date);filters->addWidget(allDates);filters->addWidget(status);layout->addLayout(filters);
        auto *t=table({"MÃ LỊCH","BỆNH NHÂN","TRIỆU CHỨNG","BÁC SĨ / CHUYÊN KHOA","NGÀY GIỜ HẸN","TRẠNG THÁI"});
        auto fill=[this,t,doctor,date,allDates,status]{t->setRowCount(0);vector<Appointment*> ordered;auto*n=appointments->getHeader()->next;while(n&&n->next){if(!n->info.isDefault())ordered.push_back(&n->info);n=n->next;}sort(ordered.begin(),ordered.end(),[](Appointment*a,Appointment*b){Date x=a->getDateGet(),y=b->getDateGet();return make_tuple(x.getYear(),x.getMonth(),x.getDay(),x.getHour(),x.getMinute())<make_tuple(y.getYear(),y.getMonth(),y.getDay(),y.getHour(),y.getMinute());});for(auto *item:ordered){auto&a=*item;Doctor*d=findDoctor(a.getDoctorId());Patient*p=findPatient(a.getPatientId());Date ad=a.getDateGet();bool matches=(doctor->currentData().toString().isEmpty()||qs(a.getDoctorId())==doctor->currentData().toString())&&(allDates->isChecked()||(ad.getYear()==date->date().year()&&ad.getMonth()==date->date().month()&&ad.getDay()==date->date().day()))&&(status->currentIndex()==0||(status->currentIndex()==1&&a.getState()==0)||(status->currentIndex()==2&&a.getState()==1));if(matches){int r=t->rowCount();t->insertRow(r);cell(t,r,0,qs(a.getId()));cell(t,r,1,p?qs(p->getfullName()):qs(a.getPatientId()));cell(t,r,2,patientSymptoms(a.getPatientId()));cell(t,r,3,d?QStringLiteral("%1 · %2").arg(qs(d->getfullName()),specialty(d->getSpId())):qs(a.getDoctorId()));cell(t,r,4,qs(a.getDateGet().toString()));cell(t,r,5,a.getState()?"Đã khám":"Chưa khám");}}};
        connect(doctor,qOverload<int>(&QComboBox::currentIndexChanged),w,[fill]{fill();});connect(date,&QDateEdit::dateChanged,w,[fill]{fill();});connect(allDates,&QCheckBox::toggled,w,[fill]{fill();});connect(status,qOverload<int>(&QComboBox::currentIndexChanged),w,[fill]{fill();});
        auto *actions=new QHBoxLayout;auto *invite=button("Mời bệnh nhân đến khám");auto *edit=button("Sửa lịch hẹn","secondary");auto *remove=button("Xóa lịch đã khám","danger");invite->setVisible(hasPermission("Per_4"));edit->setVisible(hasPermission("Per_4"));remove->setVisible(hasPermission("Per_4"));actions->addWidget(invite);actions->addWidget(edit);actions->addWidget(remove);actions->addStretch();layout->addLayout(actions);layout->addWidget(t,1);fill();
        connect(invite,&QPushButton::clicked,w,[this,t,fill]{int row=t->currentRow();if(row<0){QMessageBox::information(this,"Chọn lịch","Hãy chọn lịch hẹn cần mời.");return;}string id=ss(t->item(row,0)->text());auto*n=appointments->getHeader()->next;while(n&&n->next){if(n->info.getId()==id){if(n->info.getState()){QMessageBox::information(this,"Lịch đã hoàn tất","Lịch này đã ở trạng thái đã khám.");return;}Patient*p=findPatient(n->info.getPatientId());Doctor*d=findDoctor(n->info.getDoctorId());QString patientName=p?qs(p->getfullName()):qs(n->info.getPatientId());QString room=d?qs(d->getRoomId()):QStringLiteral("chưa xác định");QMessageBox::information(this,"Mời bệnh nhân đến khám",QStringLiteral("Mời bệnh nhân %1 đến phòng %2 để khám bệnh.").arg(patientName,room));n->info.setState(1);updatePatientSymptomStatus(n->info.getPatientId());saveAll();fill();refreshPage(2);return;}n=n->next;}});
        connect(edit,&QPushButton::clicked,w,[this,t,fill]{int row=t->currentRow();if(row>=0){editAppointmentDialog(ss(t->item(row,0)->text()));fill();}});
        connect(remove,&QPushButton::clicked,w,[this,t,fill]{int row=t->currentRow();if(row<0)return;string id=ss(t->item(row,0)->text());auto*n=appointments->getHeader()->next;while(n&&n->next){if(n->info.getId()==id){if(!n->info.getState()){QMessageBox::warning(this,"Chưa thể xóa","Chỉ có thể xóa lịch hẹn đã khám.");return;}if(QMessageBox::question(this,"Xóa lịch","Xóa lịch hẹn đã khám này?")!=QMessageBox::Yes)return;appointments->remove(n);saveAll();fill();return;}n=n->next;}});return w;
    }
    void editAppointmentDialog(const string &id) {
        if(!hasPermission("Per_4")) return;
        Appointment*appointment=nullptr;auto*n=appointments->getHeader()->next;while(n&&n->next){if(n->info.getId()==id){appointment=&n->info;break;}n=n->next;}if(!appointment)return;
        QDialog dialog(this);dialog.setWindowTitle("Chỉnh sửa lịch hẹn");auto*layout=new QVBoxLayout(&dialog);auto*form=new QFormLayout;auto*patient=new QComboBox;auto*p=patients->getHeader()->next;while(p&&p->next){patient->addItem(QStringLiteral("%1 · %2").arg(qs(p->info.getID()),qs(p->info.getfullName())),qs(p->info.getID()));p=p->next;}patient->setCurrentIndex(patient->findData(qs(appointment->getPatientId())));auto*doctor=new QComboBox;auto*d=doctors->getHeader()->next;while(d&&d->next){doctor->addItem(QStringLiteral("%1 · %2").arg(qs(d->info.getID()),qs(d->info.getfullName())),qs(d->info.getID()));d=d->next;}doctor->setCurrentIndex(doctor->findData(qs(appointment->getDoctorId())));auto*symptom=new QComboBox;auto*s=symptoms->getHeader()->next;while(s&&s->next){symptom->addItem(QStringLiteral("%1 · %2").arg(qs(s->info.getS_id()),qs(s->info.getSName())),qs(s->info.getS_id()));s=s->next;}symptom->setCurrentIndex(symptom->findData(qs(appointment->getSymptomId())));Date old=appointment->getDateGet();auto*date=new QDateTimeEdit(QDateTime(QDate(old.getYear(),old.getMonth(),old.getDay()),QTime(old.getHour(),old.getMinute())));date->setDisplayFormat("dd/MM/yyyy HH:mm");date->setCalendarPopup(true);auto*state=new QComboBox;state->addItems({"Chưa khám","Đã khám"});state->setCurrentIndex(appointment->getState()?1:0);form->addRow("Bệnh nhân",patient);form->addRow("Bác sĩ",doctor);form->addRow("Triệu chứng",symptom);form->addRow("Ngày giờ",date);form->addRow("Trạng thái",state);layout->addLayout(form);auto*save=button("Lưu thay đổi");layout->addWidget(save);connect(save,&QPushButton::clicked,&dialog,[this,&dialog,appointment,patient,doctor,symptom,date,state]{if(patient->currentIndex()<0||doctor->currentIndex()<0){QMessageBox::warning(&dialog,"Thiếu thông tin","Hãy chọn bệnh nhân và bác sĩ.");return;}if(isHoliday(date->date())){QMessageBox::warning(&dialog,"Ngày nghỉ lễ","Không thể xếp lịch vào ngày nghỉ lễ.");return;}string oldPatientId=appointment->getPatientId();appointment->setPatientId(ss(patient->currentData().toString()));appointment->setDoctorId(ss(doctor->currentData().toString()));string symptomId=ss(symptom->currentData().toString());appointment->setSymptomId(symptomId);appointment->setDateGet(Date::loadFromString(ss(date->dateTime().toString("yyyy-MM-dd HH:mm"))));appointment->setState(state->currentIndex());if(!symptomId.empty()){bool found=false;auto*r=symptomPatients->getHeader()->next;while(r&&r->next){if(r->info.getP_id()==appointment->getPatientId()&&r->info.getS_id().find(symptomId)!=string::npos)found=true;r=r->next;}if(!found)symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients),symptomId,appointment->getPatientId(),Date::now()));}updatePatientSymptomStatus(oldPatientId);updatePatientSymptomStatus(appointment->getPatientId());saveAll();dialog.accept();});dialog.exec();
    }
    QWidget *holidayPage() {
        QWidget*w=page("Lịch nghỉ","Quản lý ngày nghỉ lễ áp dụng cho lịch đặt khám.");auto*layout=qobject_cast<QVBoxLayout*>(w->layout());auto*actions=new QHBoxLayout;auto*add=button("＋ Thêm ngày nghỉ");auto*edit=button("Sửa lịch nghỉ","secondary");auto*remove=button("Xóa ngày nghỉ","danger");add->setVisible(hasPermission("Per_5"));edit->setVisible(hasPermission("Per_5"));remove->setVisible(hasPermission("Per_5"));actions->addWidget(add);actions->addWidget(edit);actions->addWidget(remove);actions->addStretch();layout->addLayout(actions);auto*t=table({"NGÀY NGHỈ","GHI CHÚ"});layout->addWidget(t,1);
        auto fill=[this,t]{t->setRowCount(0);ifstream file(holidayConfigPath);string line;while(getline(file,line)){libPBL2::trim(line);if(line.empty())continue;size_t split=line.find('-');string date=split==string::npos?line:line.substr(0,split),note=split==string::npos?"Nghỉ lễ":line.substr(split+1);libPBL2::trim(date);libPBL2::trim(note);int row=t->rowCount();t->insertRow(row);cell(t,row,0,qs(date));cell(t,row,1,qs(note));}};
        auto persist=[this,t]{ofstream file(holidayConfigPath);for(int r=0;r<t->rowCount();++r)file<<ss(t->item(r,0)->text())<<" - "<<ss(t->item(r,1)->text())<<"\n";};
        auto editHoliday=[this,t,fill,persist](int row,bool isNew){QString oldDate=isNew?QString():t->item(row,0)->text(),oldNote=isNew?QString():t->item(row,1)->text();QDialog dialog(this);dialog.setWindowTitle(isNew?"Thêm ngày nghỉ":"Chỉnh sửa ngày nghỉ");auto*v=new QVBoxLayout(&dialog);auto*f=new QFormLayout;auto*date=new QDateEdit(QDate::currentDate());date->setDisplayFormat("d/M");date->setCalendarPopup(true);if(!isNew){auto parts=oldDate.split('/');if(parts.size()==2)date->setDate(QDate(QDate::currentDate().year(),parts[1].toInt(),parts[0].toInt()));}auto*note=new QLineEdit(oldNote);f->addRow("Ngày nghỉ",date);f->addRow("Ghi chú",note);v->addLayout(f);auto*save=button("Lưu");v->addWidget(save);connect(save,&QPushButton::clicked,&dialog,[this,&dialog,t,date,note,row,isNew,fill,persist]{if(note->text().trimmed().isEmpty()){QMessageBox::warning(&dialog,"Thiếu ghi chú","Vui lòng nhập ghi chú ngày nghỉ.");return;}QString value=date->date().toString("d/M");if(isNew){int r=t->rowCount();t->insertRow(r);cell(t,r,0,value);cell(t,r,1,note->text().trimmed());}else{t->item(row,0)->setText(value);t->item(row,1)->setText(note->text().trimmed());}persist();fill();dialog.accept();});dialog.exec();};
        connect(add,&QPushButton::clicked,w,[editHoliday]{editHoliday(0,true);});connect(edit,&QPushButton::clicked,w,[this,t,editHoliday]{if(t->currentRow()>=0)editHoliday(t->currentRow(),false);});connect(remove,&QPushButton::clicked,w,[this,t,fill,persist]{int r=t->currentRow();if(r<0)return;if(QMessageBox::question(this,"Xóa ngày nghỉ","Xóa ngày nghỉ đã chọn?")!=QMessageBox::Yes)return;t->removeRow(r);persist();fill();});fill();return w;
    }
    QWidget *managerPage() {
        QWidget *w=page("Tài khoản & cấu hình","Quản trị viên HoangMA001 quản lý tài khoản, quyền hệ thống và lịch nghỉ lễ.");auto *layout=qobject_cast<QVBoxLayout *>(w->layout());auto *actions=new QHBoxLayout;auto *add=button("＋  Thêm quản lý");auto *toggle=button("Khóa / mở khóa tài khoản", "secondary");actions->addWidget(add);actions->addWidget(toggle);actions->addStretch();layout->addLayout(actions);auto*t=table({"MÃ","HỌ VÀ TÊN","TÀI KHOẢN","VAI TRÒ","TRẠNG THÁI"});layout->addWidget(t,1);managerTable=t;connect(add,&QPushButton::clicked,w,[this]{addManagerDialog();refreshManagers();});connect(toggle,&QPushButton::clicked,w,[this,t]{int row=t->currentRow();if(row<0){QMessageBox::information(this,"Chọn tài khoản","Hãy chọn tài khoản cần khóa hoặc mở khóa.");return;}const string id=ss(t->item(row,0)->text());if(id=="MA_001"){QMessageBox::warning(this,"Không thể thay đổi","Không thể khóa tài khoản quản trị viên gốc.");return;}if(currentManager&&id==currentManager->getID()){QMessageBox::warning(this,"Không thể thay đổi","Không thể khóa tài khoản đang đăng nhập.");return;}auto*n=managers->getHeader()->next;while(n&&n->next){if(n->info.getID()==id){bool lock=n->info.isActive();auto answer=QMessageBox::question(this,lock?"Khóa tài khoản":"Mở khóa tài khoản",QString("Bạn có muốn %1 tài khoản %2?").arg(lock?"khóa":"mở khóa",qs(id)));if(answer==QMessageBox::Yes){n->info.setState(lock?0:1);saveAll();refreshManagerTable();}return;}n=n->next;}});
        auto *grant = button("Cấp quyền cho tài khoản đã chọn", "secondary");grant->setVisible(isRootAdmin());layout->addWidget(grant,0,Qt::AlignLeft);connect(grant,&QPushButton::clicked,w,[this,t]{int row=t->currentRow();if(row<0){QMessageBox::information(this,"Chọn tài khoản","Hãy chọn tài khoản cần cấp quyền.");return;}grantPermissions(ss(t->item(row,0)->text()));});
        auto *note=new QLabel("Danh sách quyền chỉ xuất hiện trong cửa sổ cấp quyền. Dùng mục Lịch nghỉ để cập nhật ngày nghỉ.");note->setObjectName("muted");layout->addWidget(note);refreshManagerTable();return w;
    }
    QTableWidget *managerTable = nullptr;
    void grantPermissions(const string &managerId) {
        if (!isRootAdmin()) {
            QMessageBox::warning(this, "Không có quyền", "Chỉ tài khoản HoangMA001 được cấp quyền.");
            return;
        }
        if (managerId == currentManager->getID()) return;
        set<string> granted; ifstream input(managerPermissionPath); string line;
        while (getline(input, line)) { stringstream row(line); string id, permission; if(!getline(row,id,'|')||!getline(row,permission,'|'))continue;libPBL2::trim(id);libPBL2::trim(permission);if(id==managerId)granted.insert(permission); }
        QDialog dialog(this); dialog.setWindowTitle(QStringLiteral("Cấp quyền · %1").arg(qs(managerId)));
        auto *layout = new QVBoxLayout(&dialog); auto *checks = new vector<pair<QCheckBox*,QSpinBox*>>;
        for (const auto &entry : permissionDefinitions) { auto *check = new QCheckBox(qs(entry.second)); check->setChecked(granted.count(entry.first)>0); check->setProperty("permissionId", qs(entry.first)); layout->addWidget(check); QSpinBox *duration=nullptr;if(entry.first=="Per_1"){auto*row=new QHBoxLayout;row->addWidget(new QLabel("Thời hạn (ngày)"));duration=new QSpinBox;duration->setRange(1,3650);duration->setValue(1);row->addWidget(duration);layout->addLayout(row);duration->setVisible(check->isChecked());QObject::connect(check,&QCheckBox::toggled,duration,&QWidget::setVisible);}checks->push_back({check,duration}); }
        auto *save = button("Lưu quyền"); layout->addWidget(save);
        connect(save,&QPushButton::clicked,&dialog,[this,&dialog,managerId,checks]{
            struct PermissionGrant{string id,permission,expiry;};vector<PermissionGrant> all; ifstream existing(managerPermissionPath); string line;
            while(getline(existing,line)){stringstream row(line);string id,permission,expiry;if(!getline(row,id,'|')||!getline(row,permission,'|'))continue;getline(row,expiry);libPBL2::trim(id);libPBL2::trim(permission);libPBL2::trim(expiry);if(id.empty()||id=="MA_ID"||id.find('-')!=string::npos||id==managerId)continue;all.push_back({id,permission,expiry});}
            for(const auto &entry:*checks)if(entry.first->isChecked()){string expiry;if(entry.second)expiry=ss(QDateTime::currentDateTime().addDays(entry.second->value()).toString(Qt::ISODate));all.push_back({managerId,ss(entry.first->property("permissionId").toString()),expiry});}
            ofstream output(managerPermissionPath); if(!output){QMessageBox::warning(&dialog,"Lỗi lưu","Không thể lưu danh sách quyền.");return;}
            output<<"MA_ID | Per_ID | Expires_At\n-------+--------+--------------------\n";for(const auto &entry:all)output<<entry.id<<" | "<<entry.permission<<" | "<<entry.expiry<<"\n";
            dialog.accept();QMessageBox::information(this,"Đã cấp quyền",QStringLiteral("HoangMA001 đã cập nhật quyền cho tài khoản %1.").arg(qs(managerId)));
        }); dialog.exec();
    }
    void refreshManagerTable() {
        if(!managerTable) return;
        managerTable->setRowCount(0);auto*n=managers->getHeader()->next;while(n&&n->next){int r=managerTable->rowCount();managerTable->insertRow(r);cell(managerTable,r,0,qs(n->info.getID()));cell(managerTable,r,1,qs(n->info.getfullName()));cell(managerTable,r,2,qs(n->info.getUsername()));cell(managerTable,r,3,roleName(n->info.getRole()));cell(managerTable,r,4,n->info.isActive()?"Hoạt động":"Đã khóa");n=n->next;}
    }
    void refreshManagers(){refreshManagerTable();}
    void addManagerDialog(){QDialog dialog(this);dialog.setWindowTitle("Thêm tài khoản quản lý");auto*v=new QVBoxLayout(&dialog);auto*f=new QFormLayout;auto*name=new QLineEdit;auto*phone=new QLineEdit;auto*username=new QLineEdit;auto*password=new QLineEdit;password->setEchoMode(QLineEdit::Password);auto*age=new QSpinBox;age->setRange(18,120);auto*gender=new QComboBox;gender->addItems({"Nam","Nữ"});auto*role=new QComboBox;role->addItems({"Lễ tân","Lễ tân chuyên trách","Quản trị viên"});f->addRow("Họ tên",name);f->addRow("Điện thoại",phone);f->addRow("Tuổi",age);f->addRow("Giới tính",gender);f->addRow("Tên đăng nhập",username);f->addRow("Mật khẩu",password);f->addRow("Vai trò",role);v->addLayout(f);auto*save=button("Tạo tài khoản");v->addWidget(save);connect(save,&QPushButton::clicked,&dialog,[this,&dialog,name,phone,username,password,age,gender,role]{if(name->text().trimmed().isEmpty()||phone->text().trimmed().isEmpty()||username->text().trimmed().isEmpty()||password->text().isEmpty()){QMessageBox::warning(&dialog,"Thiếu thông tin","Vui lòng nhập đủ thông tin.");return;}auto*n=managers->getHeader()->next;while(n&&n->next){if(n->info.getUsername()==ss(username->text().trimmed())){QMessageBox::warning(&dialog,"Trùng tài khoản","Tên đăng nhập đã được sử dụng.");return;}n=n->next;}int maximum=0;n=managers->getHeader()->next;while(n&&n->next){string id=n->info.getID();if(id.rfind("MA_",0)==0)try{maximum=max(maximum,stoi(id.substr(3)));}catch(...){}n=n->next;}ostringstream id;id<<"MA_"<<setfill('0')<<setw(3)<<maximum+1;ManagerRole r=role->currentIndex()==2?ManagerRole::Administrator:(role->currentIndex()==1?ManagerRole::SpecialReceptionist:ManagerRole::Receptionist);managers->insert(Manager(id.str(),ss(name->text()),ss(phone->text()),gender->currentIndex()==0,age->value(),ss(username->text()),ss(password->text()),r,1));saveAll();dialog.accept();});dialog.exec();}
    void refreshPage(int index){if(!pages)return;if(index>=0&&index<=5){QWidget*old=pages->widget(index);pages->removeWidget(old);delete old;QWidget*updated=nullptr;switch(index){case 0:updated=dashboardPage();break;case 1:updated=bookingPage();break;case 2:updated=patientPage();break;case 3:updated=doctorPage();break;case 4:updated=appointmentPage();break;case 5:updated=holidayPage();break;}pages->insertWidget(index,updated);}else if(index==6)refreshManagers();}
    void saveAll(){bool ok=Patient::saveToFile(patientPath,patients)&&Manager::saveToFile(managerPath,managers)&&Doctor::saveToFile("Data/Doctor/Doctor.txt",doctors)&&SymptomPatient::saveToFile(symptomPatientPath,symptomPatients)&&Appointment::saveToFile(appointmentPath,appointments);if(!ok)QMessageBox::warning(this,"Lỗi lưu dữ liệu","Không thể lưu một hoặc nhiều tệp dữ liệu.");}
    void showLogin(){setWindowTitle("An Tâm · Đăng nhập");delete root;root=nullptr;auto*panelWidget=new QWidget;panelWidget->setObjectName("appRoot");setCentralWidget(panelWidget);auto*layout=new QHBoxLayout(panelWidget);layout->setContentsMargins(70,50,70,50);layout->setSpacing(40);
        auto*hero=new QFrame;hero->setStyleSheet("QFrame{background:#12394c;border-radius:24px;}");auto*hv=new QVBoxLayout(hero);hv->setContentsMargins(45,45,45,45);auto*logo=new QLabel("✦  AN TÂM");logo->setStyleSheet("font-size:24px;font-weight:750;color:white;");auto*headline=new QLabel("Chăm sóc tận tâm.\nSức khỏe vẹn toàn.");headline->setStyleSheet("font-size:32px;font-weight:750;color:white;");auto*caption=new QLabel("Nền tảng quản lý lịch khám\nđược thiết kế cho trải nghiệm an tâm.");caption->setStyleSheet("font-size:15px;color:#b4d0d8;line-height:1.5;");hv->addWidget(logo);hv->addStretch();hv->addWidget(headline);hv->addWidget(caption);hv->addStretch();auto*heroNote=new QLabel("PHÒNG KHÁM ĐA KHOA  ·  AN TÂM MỖI NGÀY");heroNote->setStyleSheet("font-size:10px;color:#8bb1bb;letter-spacing:1px;");hv->addWidget(heroNote);
        auto*formCard=panel();auto*fv=new QVBoxLayout(formCard);fv->setContentsMargins(38,38,38,38);fv->addStretch();auto*title=new QLabel("Chào mừng trở lại");title->setStyleSheet("font-size:25px;font-weight:750;color:#18384d;");auto*sub=new QLabel("Đăng nhập để tiếp tục quản lý phòng khám.");sub->setObjectName("muted");auto*username=new QLineEdit;username->setPlaceholderText("Tên đăng nhập");auto*password=new QLineEdit;password->setPlaceholderText("Mật khẩu");password->setEchoMode(QLineEdit::Password);auto*login=button("Đăng nhập  →");auto*hint=new QLabel("Tài khoản demo: HoangMA001");hint->setObjectName("muted");fv->addWidget(title);fv->addWidget(sub);fv->addSpacing(18);fv->addWidget(username);fv->addWidget(password);fv->addWidget(login);fv->addWidget(hint);fv->addStretch();layout->addWidget(hero,5);layout->addWidget(formCard,4);
        auto doLogin=[this,username,password]{currentManager=AuthController::authenticate(managers,ss(username->text().trimmed()),ss(password->text()));if(currentManager){setupShell();setWindowTitle("An Tâm · Quản lý phòng khám");return;}QMessageBox::warning(this,"Không thể đăng nhập","Tên đăng nhập hoặc mật khẩu không đúng, hoặc tài khoản đã bị khóa.");};connect(login,&QPushButton::clicked,this,doLogin);connect(password,&QLineEdit::returnPressed,this,doLogin);
    }
public:
    ClinicWindow(){setStyleSheet(style());setMinimumSize(1120,720);resize(1360,850);setWindowTitle("An Tâm · Quản lý phòng khám");QTimer::singleShot(350,this,[this]{showLogin();});}
    ~ClinicWindow() override {delete managers;delete patients;delete doctors;delete symptoms;delete symptomPatients;delete appointments;}
    bool loadData(){managers=Manager::loadFromFile(managerPath);patients=Patient::loadFromFile(patientPath);doctors=Doctor::loadFromFile("Data/Doctor/Doctor.txt");symptoms=Symptom::loadFromFile("Data/Symptom/Symptom.txt");symptomPatients=SymptomPatient::loadFromFile(symptomPatientPath);appointments=Appointment::loadFromFile(appointmentPath);bool ok=managers&&patients&&doctors&&symptoms&&symptomPatients&&appointments;if(!ok)return false;auto*p=patients->getHeader()->next;while(p&&p->next){updatePatientSymptomStatus(p->info.getID());p=p->next;}return true;}
};

int runClinicApp(int argc,char **argv){QApplication app(argc,argv);QApplication::setApplicationName("Phòng khám An Tâm");QApplication::setStyle("Fusion");ClinicWindow window;if(!window.loadData()){QMessageBox::critical(nullptr,"Không tải được dữ liệu","Không tìm thấy dữ liệu phòng khám. Hãy chạy ứng dụng từ thư mục gốc dự án.");return 1;}window.show();return app.exec();}

