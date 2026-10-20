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

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "Appointment.h"
#include "Doctor.h"
#include "Manager.h"
#include "Patient.h"
#include "Symptom.h"
#include "SymptomPatient.h"
#include "Permission.h"
#include "libPBL2.h"

using namespace std;

static QString qs(const string &s) { return QString::fromUtf8(s.c_str()); }
static string ss(const QString &s) { return s.toUtf8().toStdString(); }
static const string patientPath = "Data/Patient.txt";
static const string managerPath = "Data/Manager.txt";
static const string appointmentPath = "Data/Appointment/Appointment.txt";
static const string symptomPatientPath = "Data/Symptom/SymptomPatient.txt";

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
        const QStringList labels = {"⌂   Tổng quan", "＋   Đặt lịch khám", "♡   Bệnh nhân", "✚   Bác sĩ", "▤   Quản lý lịch khám"};
        for (int i = 0; i < labels.size(); ++i) { auto *b = button(labels[i], "nav"); connect(b, &QPushButton::clicked, this, [this, i]{ refreshPage(i); setPage(i); }); side->addWidget(b); }
        if (currentManager && currentManager->getID() == "MA_001") {
            auto *b = button("♙   Quản lý tài khoản", "nav"); connect(b, &QPushButton::clicked, this, [this]{ refreshManagers(); setPage(5); }); side->addWidget(b);
        }
        side->addStretch();
        userLabel = new QLabel; userLabel->setStyleSheet("color:#e6f0f4; padding:8px 4px;"); side->addWidget(userLabel);
        auto *logout = button("↪   Đăng xuất", "nav"); connect(logout, &QPushButton::clicked, this, [this]{ saveAll(); currentManager = nullptr; showLogin(); }); side->addWidget(logout);
        pages = new QStackedWidget;
        outer->addWidget(sidebar); outer->addWidget(pages, 1);
        pages->addWidget(dashboardPage()); pages->addWidget(bookingPage()); pages->addWidget(patientPage()); pages->addWidget(doctorPage()); pages->addWidget(appointmentPage());
        if (currentManager && currentManager->getID() == "MA_001") pages->addWidget(managerPage());
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
        while (n && n->next) { if (!n->info.isDefault()) all.push_back(&n->info); n = n->next; }
        sort(all.begin(), all.end(), [](Appointment *a, Appointment *b){ return a->getDateGet().toString() < b->getDateGet().toString(); });
        int limit = upcomingOnly ? min<int>(5, all.size()) : all.size();
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
        auto *doctor = new QComboBox; doctor->addItem("Chọn bác sĩ…", ""); auto *date = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1)); date->setDisplayFormat("dd/MM/yyyy  HH:mm"); date->setCalendarPopup(true);
        form->addRow("Bệnh nhân", patient); form->addRow("Triệu chứng", symptom); form->addRow("Bác sĩ khám", doctor); form->addRow("Ngày giờ hẹn", date);
        QObject::connect(symptom, qOverload<int>(&QComboBox::currentIndexChanged), w, [this, symptom, doctor]{ doctor->clear(); doctor->addItem("Chọn bác sĩ…", ""); auto *s = symptoms->getHeader()->next; string spid; while (s && s->next) { if (qs(s->info.getS_id()) == symptom->currentData().toString()) { spid = s->info.getSp_id(); break; } s = s->next; }
            auto *d = doctors->getHeader()->next; while (d && d->next) { if (d->info.getState() && (spid.empty() || d->info.getSpId() == spid)) doctor->addItem(QStringLiteral("%1  ·  %2  ·  %3").arg(qs(d->info.getID()), qs(d->info.getfullName()), specialty(d->info.getSpId())), qs(d->info.getID())); d = d->next; }
        });
        auto *save = button("Xác nhận đặt lịch"); connect(save, &QPushButton::clicked, w, [this, patient, symptom, doctor, date]{
            if (patient->currentIndex() < 0 || doctor->currentData().toString().isEmpty() || symptom->currentData().toString().isEmpty()) { QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng chọn bệnh nhân, triệu chứng và bác sĩ."); return; }
            if (date->dateTime() <= QDateTime::currentDateTime()) { QMessageBox::warning(this, "Thời gian không hợp lệ", "Vui lòng chọn thời gian trong tương lai."); return; }
            string pId = ss(patient->currentData().toString()), dId = ss(doctor->currentData().toString());
            appointments->insert(Appointment(nextAppointmentId(), pId, dId, Date::now(), Date::loadFromString(ss(date->dateTime().toString("yyyy-MM-dd HH:mm"))), 0));
            auto *s = symptoms->getHeader()->next; string symptomId = ss(symptom->currentData().toString());
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
        auto *actions = new QHBoxLayout; auto *search = new QLineEdit; search->setPlaceholderText("Tìm theo tên, mã hoặc số điện thoại…"); auto *add = button("＋  Thêm bệnh nhân"); actions->addWidget(search,1); actions->addWidget(add); layout->addLayout(actions);
        auto *t = table({"MÃ HỒ SƠ", "HỌ VÀ TÊN", "ĐIỆN THOẠI", "TUỔI", "GIỚI TÍNH", "TRIỆU CHỨNG"});
        auto fill = [this,t,search]{ t->setRowCount(0); QString q = search->text().trimmed().toLower(); auto *n = patients->getHeader()->next; while (n && n->next) { QString name = qs(n->info.getfullName()), id = qs(n->info.getID()), phone = qs(n->info.getPhone()); if (q.isEmpty() || name.toLower().contains(q) || id.toLower().contains(q) || phone.contains(q)) { int r=t->rowCount(); t->insertRow(r); cell(t,r,0,id); cell(t,r,1,name); cell(t,r,2,phone); cell(t,r,3,QString::number(n->info.getAge())); cell(t,r,4,n->info.getGender()?"Nam":"Nữ"); cell(t,r,5,patientSymptoms(n->info.getID())); } n=n->next; }};
        connect(search, &QLineEdit::textChanged, w, [fill]{fill();}); fill(); layout->addWidget(t,1);
        connect(add, &QPushButton::clicked, w, [this,fill]{ addPatientDialog(); fill(); }); return w;
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
            if(!ids.empty())symptomPatients->insert(SymptomPatient(SymptomPatient::generateNextId(symptomPatients),ids,id,Date::now())); saveAll(); dialog.accept();
        }); dialog.exec();
    }
    QWidget *doctorPage() {
        QWidget *w=page("Đội ngũ bác sĩ","Danh sách bác sĩ và chuyên khoa đang hoạt động tại phòng khám."); auto *layout=qobject_cast<QVBoxLayout *>(w->layout()); auto *search=new QLineEdit;search->setPlaceholderText("Tìm bác sĩ hoặc chuyên khoa…");layout->addWidget(search);
        auto *t=table({"MÃ BÁC SĨ","BÁC SĨ","CHUYÊN KHOA","PHÒNG","PHÍ KHÁM","ĐÁNH GIÁ","TRẠNG THÁI"});
        auto fill=[this,t,search]{t->setRowCount(0);QString q=search->text().trimmed().toLower();auto*n=doctors->getHeader()->next;while(n&&n->next){QString name=qs(n->info.getfullName()),spec=specialty(n->info.getSpId());if(q.isEmpty()||name.toLower().contains(q)||spec.toLower().contains(q)){int r=t->rowCount();t->insertRow(r);cell(t,r,0,qs(n->info.getID()));cell(t,r,1,name);cell(t,r,2,spec);cell(t,r,3,qs(n->info.getRoomId()));cell(t,r,4,QLocale(QLocale::Vietnamese).toString(n->info.getFee(),'f',0)+" ₫");cell(t,r,5,QString::number(n->info.getRating(),'f',1)+" ★");cell(t,r,6,n->info.getState()?"Đang nhận khám":"Tạm nghỉ");}n=n->next;}};
        connect(search,&QLineEdit::textChanged,w,[fill]{fill();});fill();layout->addWidget(t,1);return w;
    }
    QWidget *appointmentPage() {
        QWidget *w=page("Quản lý lịch khám","Theo dõi lịch theo bác sĩ, ngày hẹn và trạng thái khám.");auto *layout=qobject_cast<QVBoxLayout *>(w->layout());auto *filters=new QHBoxLayout;auto *doctor=new QComboBox;doctor->addItem("Tất cả bác sĩ","");auto*d=doctors->getHeader()->next;while(d&&d->next){doctor->addItem(QStringLiteral("%1 · %2").arg(qs(d->info.getID()),qs(d->info.getfullName())),qs(d->info.getID()));d=d->next;}auto *date=new QDateEdit;date->setCalendarPopup(true);date->setDisplayFormat("dd/MM/yyyy");auto *allDates=new QCheckBox("Tất cả ngày");allDates->setChecked(true);auto *status=new QComboBox;status->addItems({"Tất cả trạng thái","Chưa khám","Đã khám"});filters->addWidget(new QLabel("Bác sĩ"));filters->addWidget(doctor,1);filters->addWidget(new QLabel("Ngày hẹn"));filters->addWidget(date);filters->addWidget(allDates);filters->addWidget(status);layout->addLayout(filters);
        auto *t=table({"MÃ LỊCH","BỆNH NHÂN","TRIỆU CHỨNG","BÁC SĨ / CHUYÊN KHOA","NGÀY GIỜ HẸN","TRẠNG THÁI"});
        auto fill=[this,t,doctor,date,allDates,status]{t->setRowCount(0);auto*n=appointments->getHeader()->next;while(n&&n->next){auto&a=n->info;Doctor*d=findDoctor(a.getDoctorId());Patient*p=findPatient(a.getPatientId());Date ad=a.getDateGet();bool matches=(doctor->currentData().toString().isEmpty()||qs(a.getDoctorId())==doctor->currentData().toString())&&(allDates->isChecked()||(ad.getYear()==date->date().year()&&ad.getMonth()==date->date().month()&&ad.getDay()==date->date().day()))&&(status->currentIndex()==0||(status->currentIndex()==1&&a.getState()==0)||(status->currentIndex()==2&&a.getState()==1));if(matches){int r=t->rowCount();t->insertRow(r);cell(t,r,0,qs(a.getId()));cell(t,r,1,p?qs(p->getfullName()):qs(a.getPatientId()));cell(t,r,2,patientSymptoms(a.getPatientId()));cell(t,r,3,d?QStringLiteral("%1 · %2").arg(qs(d->getfullName()),specialty(d->getSpId())):qs(a.getDoctorId()));cell(t,r,4,qs(a.getDateGet().toString()));cell(t,r,5,a.getState()?"Đã khám":"Chưa khám");}n=n->next;}};
        connect(doctor,qOverload<int>(&QComboBox::currentIndexChanged),w,[fill]{fill();});connect(date,&QDateEdit::dateChanged,w,[fill]{fill();});connect(allDates,&QCheckBox::toggled,w,[fill]{fill();});connect(status,qOverload<int>(&QComboBox::currentIndexChanged),w,[fill]{fill();});
        auto *mark=button("Đổi trạng thái lịch được chọn","secondary");connect(mark,&QPushButton::clicked,w,[this,t,fill]{int row=t->currentRow();if(row<0){QMessageBox::information(this,"Chọn lịch","Hãy chọn một lịch trong danh sách.");return;}string id=ss(t->item(row,0)->text());auto*n=appointments->getHeader()->next;while(n&&n->next){if(n->info.getId()==id){n->info.setState(n->info.getState()?0:1);saveAll();fill();return;}n=n->next;}});layout->addWidget(mark,0,Qt::AlignLeft);layout->addWidget(t,1);fill();return w;
    }
    QWidget *managerPage() {
        QWidget *w=page("Tài khoản quản lý","Chỉ quản trị viên cấp cao mới có thể tạo và xem tài khoản quản lý.");auto *layout=qobject_cast<QVBoxLayout *>(w->layout());auto *add=button("＋  Thêm quản lý");layout->addWidget(add,0,Qt::AlignLeft);auto*t=table({"MÃ","HỌ VÀ TÊN","TÀI KHOẢN","VAI TRÒ","TRẠNG THÁI"});layout->addWidget(t,1);managerTable=t;connect(add,&QPushButton::clicked,w,[this]{addManagerDialog();refreshManagers();});refreshManagerTable();return w;
    }
    QTableWidget *managerTable = nullptr;
    void refreshManagerTable() {
        if(!managerTable)return;managerTable->setRowCount(0);auto*n=managers->getHeader()->next;while(n&&n->next){int r=managerTable->rowCount();managerTable->insertRow(r);cell(managerTable,r,0,qs(n->info.getID()));cell(managerTable,r,1,qs(n->info.getfullName()));cell(managerTable,r,2,qs(n->info.getUsername()));cell(managerTable,r,3,qs(Manager::roleToString(n->info.getRole())));cell(managerTable,r,4,n->info.isActive()?"Hoạt động":"Đã khóa");n=n->next;}
    }
    void refreshManagers(){refreshManagerTable();}
    void addManagerDialog(){QDialog dialog(this);dialog.setWindowTitle("Thêm tài khoản quản lý");auto*v=new QVBoxLayout(&dialog);auto*f=new QFormLayout;auto*name=new QLineEdit;auto*phone=new QLineEdit;auto*username=new QLineEdit;auto*password=new QLineEdit;password->setEchoMode(QLineEdit::Password);auto*age=new QSpinBox;age->setRange(18,120);auto*gender=new QComboBox;gender->addItems({"Nam","Nữ"});auto*role=new QComboBox;role->addItems({"Lễ tân","Lễ tân chuyên trách","Quản trị viên"});f->addRow("Họ tên",name);f->addRow("Điện thoại",phone);f->addRow("Tuổi",age);f->addRow("Giới tính",gender);f->addRow("Tên đăng nhập",username);f->addRow("Mật khẩu",password);f->addRow("Vai trò",role);v->addLayout(f);auto*save=button("Tạo tài khoản");v->addWidget(save);connect(save,&QPushButton::clicked,&dialog,[this,&dialog,name,phone,username,password,age,gender,role]{if(name->text().trimmed().isEmpty()||phone->text().trimmed().isEmpty()||username->text().trimmed().isEmpty()||password->text().isEmpty()){QMessageBox::warning(&dialog,"Thiếu thông tin","Vui lòng nhập đủ thông tin.");return;}auto*n=managers->getHeader()->next;while(n&&n->next){if(n->info.getUsername()==ss(username->text().trimmed())){QMessageBox::warning(&dialog,"Trùng tài khoản","Tên đăng nhập đã được sử dụng.");return;}n=n->next;}int maximum=0;n=managers->getHeader()->next;while(n&&n->next){string id=n->info.getID();if(id.rfind("MA_",0)==0)try{maximum=max(maximum,stoi(id.substr(3)));}catch(...){}n=n->next;}ostringstream id;id<<"MA_"<<setfill('0')<<setw(3)<<maximum+1;ManagerRole r=role->currentIndex()==2?ManagerRole::Administrator:(role->currentIndex()==1?ManagerRole::SpecialReceptionist:ManagerRole::Receptionist);managers->insert(Manager(id.str(),ss(name->text()),ss(phone->text()),gender->currentIndex()==0,age->value(),ss(username->text()),ss(password->text()),r,1));saveAll();dialog.accept();});dialog.exec();}
    void refreshPage(int index){if(!pages)return;if(index==0||index==1||index==2||index==3||index==4){QWidget*old=pages->widget(index);pages->removeWidget(old);delete old;QWidget*updated=nullptr;switch(index){case 0:updated=dashboardPage();break;case 1:updated=bookingPage();break;case 2:updated=patientPage();break;case 3:updated=doctorPage();break;case 4:updated=appointmentPage();break;}pages->insertWidget(index,updated);}else if(index==5)refreshManagers();}
    void saveAll(){bool ok=Patient::saveToFile(patientPath,patients)&&Manager::saveToFile(managerPath,managers)&&SymptomPatient::saveToFile(symptomPatientPath,symptomPatients)&&Appointment::saveToFile(appointmentPath,appointments);if(!ok)QMessageBox::warning(this,"Lỗi lưu dữ liệu","Không thể lưu một hoặc nhiều tệp dữ liệu.");}
    void showLogin(){setWindowTitle("An Tâm · Đăng nhập");delete root;root=nullptr;auto*panelWidget=new QWidget;panelWidget->setObjectName("appRoot");setCentralWidget(panelWidget);auto*layout=new QHBoxLayout(panelWidget);layout->setContentsMargins(70,50,70,50);layout->setSpacing(40);
        auto*hero=new QFrame;hero->setStyleSheet("QFrame{background:#12394c;border-radius:24px;}");auto*hv=new QVBoxLayout(hero);hv->setContentsMargins(45,45,45,45);auto*logo=new QLabel("✦  AN TÂM");logo->setStyleSheet("font-size:24px;font-weight:750;color:white;");auto*headline=new QLabel("Chăm sóc tận tâm.\nSức khỏe vẹn toàn.");headline->setStyleSheet("font-size:32px;font-weight:750;color:white;");auto*caption=new QLabel("Nền tảng quản lý lịch khám\nđược thiết kế cho trải nghiệm an tâm.");caption->setStyleSheet("font-size:15px;color:#b4d0d8;line-height:1.5;");hv->addWidget(logo);hv->addStretch();hv->addWidget(headline);hv->addWidget(caption);hv->addStretch();auto*heroNote=new QLabel("PHÒNG KHÁM ĐA KHOA  ·  AN TÂM MỖI NGÀY");heroNote->setStyleSheet("font-size:10px;color:#8bb1bb;letter-spacing:1px;");hv->addWidget(heroNote);
        auto*formCard=panel();auto*fv=new QVBoxLayout(formCard);fv->setContentsMargins(38,38,38,38);fv->addStretch();auto*title=new QLabel("Chào mừng trở lại");title->setStyleSheet("font-size:25px;font-weight:750;color:#18384d;");auto*sub=new QLabel("Đăng nhập để tiếp tục quản lý phòng khám.");sub->setObjectName("muted");auto*username=new QLineEdit;username->setPlaceholderText("Tên đăng nhập");auto*password=new QLineEdit;password->setPlaceholderText("Mật khẩu");password->setEchoMode(QLineEdit::Password);auto*login=button("Đăng nhập  →");auto*hint=new QLabel("Tài khoản demo: HoangMA001");hint->setObjectName("muted");fv->addWidget(title);fv->addWidget(sub);fv->addSpacing(18);fv->addWidget(username);fv->addWidget(password);fv->addWidget(login);fv->addWidget(hint);fv->addStretch();layout->addWidget(hero,5);layout->addWidget(formCard,4);
        auto doLogin=[this,username,password]{auto*n=managers->getHeader()->next;while(n&&n->next){if(n->info.isActive()&&n->info.isValid(ss(username->text().trimmed()),ss(password->text()))){currentManager=&n->info;setupShell();setWindowTitle("An Tâm · Quản lý phòng khám");return;}n=n->next;}QMessageBox::warning(this,"Không thể đăng nhập","Tên đăng nhập hoặc mật khẩu không đúng, hoặc tài khoản đã bị khóa.");};connect(login,&QPushButton::clicked,this,doLogin);connect(password,&QLineEdit::returnPressed,this,doLogin);
    }
public:
    ClinicWindow(){setStyleSheet(style());setMinimumSize(1120,720);resize(1360,850);setWindowTitle("An Tâm · Quản lý phòng khám");QTimer::singleShot(350,this,[this]{showLogin();});}
    ~ClinicWindow() override {delete managers;delete patients;delete doctors;delete symptoms;delete symptomPatients;delete appointments;}
    bool loadData(){managers=Manager::loadFromFile(managerPath);patients=Patient::loadFromFile(patientPath);doctors=Doctor::loadFromFile("Data/Doctor/Doctor.txt");symptoms=Symptom::loadFromFile("Data/Symptom/Symptom.txt");symptomPatients=SymptomPatient::loadFromFile(symptomPatientPath);appointments=Appointment::loadFromFile(appointmentPath);return managers&&patients&&doctors&&symptoms&&symptomPatients&&appointments;}
};

int main(int argc,char **argv){QApplication app(argc,argv);QApplication::setApplicationName("Phòng khám An Tâm");QApplication::setStyle("Fusion");ClinicWindow window;if(!window.loadData()){QMessageBox::critical(nullptr,"Không tải được dữ liệu","Không tìm thấy dữ liệu phòng khám. Hãy chạy ứng dụng từ thư mục gốc dự án E:/pbl2test.");return 1;}window.show();return app.exec();}
