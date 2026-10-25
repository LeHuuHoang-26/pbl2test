#define NOMINMAX
#include "MainWindow.h"
#include "AppointmentView.h"
#include "PatientView.h"
#include "DoctorView.h"
#include "AccountView.h"
#include "StatisticView.h"
#include "Login.h"
#include "UIHelper.h"

#include "AppointmentController.h"
#include "DoctorController.h"
#include "PatientController.h"
#include "ManagerController.h"
#include "StatisticController.h"
#include "AuthController.h"
#include "libPBL2.h"
#include <sstream>

#include <QApplication>
#include <QWidget>
#include <QStackedWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QIcon>
#include <QStyle>
#include <QTimer>

using namespace std;

// Đường dẫn tệp cấu hình và dữ liệu chuẩn
static const string patientPath = "Data/Patient.txt";
static const string managerPath = "Data/Manager.txt";
static const string doctorPath = "Data/Doctor/Doctor.txt";
static const string appointmentPath = "Data/Appointment.txt";
static const string symptomPatientPath = "Data/Symptom/SymptomPatient.txt";
static const string symptomPath = "Data/Symptom/Symptom.txt";
static const string specializationPath = "Data/Doctor/Specialization.txt";
static const string holidayConfigPath = "Config/Lichnghile.txt";
static const string permissionManagerPath = "Data/PermissionManager.txt";

// --- KIỂM TRA PHÂN QUYỀN (Ủy thác ManagerController) ---
bool MainWindow::isRootAdmin() const
{
    return ManagerController::isRootAdmin(currentManager);
}

bool MainWindow::isAdministrator() const
{
    return ManagerController::isAdministrator(currentManager, permissionManagerPath);
}

bool MainWindow::hasPermission(const string &permissionId) const
{
    return ManagerController::hasPermission(currentManager, permissionId, permissionManagerPath);
}

// --- TIỆN ÍCH DỮ LIỆU (Ủy thác Controllers) ---
QString MainWindow::specialty(const string &id) const
{
    return DoctorController::getSpecialty(id, specializationPath);
}

Patient *MainWindow::findPatient(const string &id) const
{
    return PatientController::findById(patients, id);
}

Doctor *MainWindow::findDoctor(const string &id) const
{
    return DoctorController::findById(doctors, id);
}

SymptomPatient *MainWindow::findSymptomPatient(const string &id) const
{
    if (!symptomPatients || id.empty()) return nullptr;
    auto *node = symptomPatients->getHeader()->next;
    while (node && node->next) {
        if (node->info.getSP_id() == id) {
            return &node->info;
        }
        node = node->next;
    }
    return nullptr;
}

Patient *MainWindow::findPatientForAppointment(const Appointment &a) const
{
    SymptomPatient *sp = findSymptomPatient(a.getSymptomPatientId());
    if (sp) {
        return findPatient(sp->getP_id());
    }
    return nullptr;
}

string MainWindow::getPatientIdForAppointment(const Appointment &a) const
{
    SymptomPatient *sp = findSymptomPatient(a.getSymptomPatientId());
    if (sp) {
        return sp->getP_id();
    }
    return "";
}

QString MainWindow::getSymptomNamesForAppointment(const Appointment &a) const
{
    SymptomPatient *sp = findSymptomPatient(a.getSymptomPatientId());
    if (!sp || sp->getS_id().empty()) return QStringLiteral("Chưa ghi nhận");

    QStringList names;
    stringstream ss(sp->getS_id());
    string singleId;
    while (getline(ss, singleId, ',')) {
        libPBL2::trim(singleId);
        if (symptoms && symptoms->getHeader()) {
            auto *sNode = symptoms->getHeader()->next;
            while (sNode && sNode->next) {
                if (sNode->info.getS_id() == singleId) {
                    names.append(QString::fromUtf8(sNode->info.getSName().c_str()));
                    break;
                }
                sNode = sNode->next;
            }
        }
    }
    names.removeDuplicates();
    return names.isEmpty() ? QStringLiteral("Chưa ghi nhận") : names.join(QStringLiteral(", "));
}

bool MainWindow::isHoliday(const QDate &date) const
{
    return AppointmentController::isHoliday(date, holidayConfigPath);
}

QString MainWindow::holidayNote(const QDate &date) const
{
    return AppointmentController::getHolidayNote(date, holidayConfigPath);
}

QString MainWindow::patientSymptoms(const string &patientId) const
{
    QStringList names = PatientController::getSymptomsForPatient(patientId, symptomPatients, symptoms);
    return names.isEmpty() ? QStringLiteral("Chưa ghi nhận") : names.join(QStringLiteral(", "));
}

QString MainWindow::unresolvedPatientSymptoms(const string &patientId) const
{
    return PatientController::getUnresolvedSymptoms(patientId, symptomPatients, symptoms, appointments);
}

void MainWindow::updatePatientSymptomStatus(const string &patientId)
{
    PatientController::updateSymptomStatus(patientId, symptomPatients, appointments);
}

string MainWindow::nextAppointmentId() const
{
    return AppointmentController::generateNextId(appointments);
}

bool MainWindow::showBookingDialog(const QDate &defaultDate)
{
    return appointmentView ? appointmentView->showBookingDialog(defaultDate) : false;
}

void MainWindow::editAppointmentDialog(const string &id)
{
    if (appointmentView) {
        appointmentView->editAppointmentDialog(id);
    }
}

// --- STYLESHEET TOÀN CỤC ---
QString MainWindow::style() const
{
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
            QComboBox, QDateEdit, QDateTimeEdit {
                padding-right: 28px;
            }
            QComboBox:disabled, QDateEdit:disabled, QDateTimeEdit:disabled {
                background: #f1f5f9;
                color: #94a3b8;
                border-color: #e2e8f0;
            }
            QComboBox::drop-down, QDateEdit::drop-down, QDateTimeEdit::drop-down {
                subcontrol-origin: border;
                subcontrol-position: top right;
                width: 28px;
                border: none;
                background: transparent;
            }
            QComboBox::drop-down:disabled, QDateEdit::drop-down:disabled, QDateTimeEdit::drop-down:disabled {
                width: 0px;
                border: none;
            }
            QComboBox::down-arrow, QDateEdit::down-arrow, QDateTimeEdit::down-arrow {
                image: url(:/assets/textures/arrow_down_dark.png);
                width: 11px;
                height: 11px;
                margin-right: 8px;
            }
            QComboBox::down-arrow:disabled, QDateEdit::down-arrow:disabled, QDateTimeEdit::down-arrow:disabled {
                image: none;
            }
            QSpinBox, QDoubleSpinBox {
                padding-right: 30px;
            }
            QSpinBox::up-button, QDoubleSpinBox::up-button {
                subcontrol-origin: border;
                subcontrol-position: top right;
                width: 25px;
                border-left: 1.5px solid #94a3b8;
                border-bottom: 1px solid #94a3b8;
                border-top-right-radius: 7px;
                background: #e2e8f0;
            }
            QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {
                background: #0d9488;
            }
            QSpinBox::up-button:hover QSpinBox::up-arrow,
            QDoubleSpinBox::up-button:hover QDoubleSpinBox::up-arrow {
                image: url(:/assets/textures/arrow_up_white.png);
            }
            QSpinBox::up-button:pressed, QDoubleSpinBox::up-button:pressed {
                background: #0f766e;
            }
            QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
                image: url(:/assets/textures/arrow_up_dark.png);
                width: 9px;
                height: 9px;
            }
            QSpinBox::down-button, QDoubleSpinBox::down-button {
                subcontrol-origin: border;
                subcontrol-position: bottom right;
                width: 25px;
                border-left: 1.5px solid #94a3b8;
                border-bottom-right-radius: 7px;
                background: #e2e8f0;
            }
            QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
                background: #0d9488;
            }
            QSpinBox::down-button:hover QSpinBox::down-arrow,
            QDoubleSpinBox::down-button:hover QDoubleSpinBox::down-arrow {
                image: url(:/assets/textures/arrow_down_white.png);
            }
            QSpinBox::down-button:pressed, QDoubleSpinBox::down-button:pressed {
                background: #0f766e;
            }
            QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
                image: url(:/assets/textures/arrow_down_dark.png);
                width: 9px;
                height: 9px;
            }
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                color: #0f172a;
                border: 1.5px solid #cbd5e1;
                border-radius: 8px;
                padding: 4px;
                selection-background-color: #0d9488;
                selection-color: #ffffff;
                outline: none;
            }
            QComboBox QAbstractItemView::item {
                min-height: 28px;
                padding: 4px 10px;
                border-radius: 6px;
                color: #0f172a;
            }
            QComboBox QAbstractItemView::item:hover {
                background-color: #f0fdfa;
                color: #0f766e;
            }
            QComboBox QAbstractItemView::item:selected {
                background-color: #0d9488;
                color: #ffffff;
            }
            QScrollBar:vertical {
                border: none;
                background: #f1f5f9;
                width: 6px;
                border-radius: 3px;
                margin: 2px;
            }
            QScrollBar::handle:vertical {
                background: #cbd5e1;
                min-height: 20px;
                border-radius: 3px;
            }
            QScrollBar::handle:vertical:hover {
                background: #94a3b8;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QTableWidget { background: white; border: 1px solid #e2e8f0; border-radius: 12px; gridline-color: transparent; selection-background-color: #f1f5f9; selection-color: #17334a; }
            QHeaderView { border: none; background: transparent; }
            QHeaderView::section { background: #f8fafc; color: #475569; border: none; border-bottom: 2px solid #e2e8f0; border-left: none; border-right: none; padding: 12px 20px; font-size: 12px; font-weight: 700; letter-spacing: 0.5px; }
            QTableWidget::item { padding: 4px 8px; font-size: 13.5px; color: #1e293b; border: none; border-bottom: 1px solid #f1f5f9; border-left: none; border-right: none; }
            QTableWidget::item:hover { background-color: #f8fafc; }
            QFrame#card, QFrame#panel { background: white; border: 1px solid #e2e8f0; border-radius: 14px; }
            QLabel#metricValue { font-size: 32px; font-weight: 800; }
            QLabel#metricLabel { color: #64748b; font-size: 12px; font-weight: 700; letter-spacing: 0.5px; }
            QLabel#pill { background: #e6f5f2; color: #14756e; border-radius: 10px; padding: 5px 9px; font-size: 11px; font-weight: 700; }
            QCheckBox { spacing: 9px; }
        )");
}

QPushButton *MainWindow::button(const QString &text, const QString &kind)
{
    auto *b = new QPushButton(text);
    b->setObjectName(kind);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

void MainWindow::setPage(int index)
{
    activeNav = index;
    if (pages)
        pages->setCurrentIndex(index);
    updateNavigation();
}

void MainWindow::updateNavigation()
{
    for (int i = 0; i < navButtons.size(); ++i)
    {
        navButtons[i]->setProperty("active", i == activeNav);
        navButtons[i]->style()->unpolish(navButtons[i]);
        navButtons[i]->style()->polish(navButtons[i]);
    }
}

// --- MÀN HÌNH ĐĂNG NHẬP ---
void MainWindow::showLogin()
{
    setWindowTitle("An Tâm · Đăng nhập");
    if (loginView)
    {
        loginView->clearInputs();
        loginView->clearMessage();
    }
    if (windowStack && loginView)
    {
        windowStack->setCurrentWidget(loginView);
    }
}

void MainWindow::handleLogin()
{
    if (!loginView) return;
    loginView->clearMessage();

    QString username = loginView->getUsername();
    QString password = loginView->getPassword();

    QString errorMessage;
    Manager *manager = nullptr;
    bool success = AuthController::login(managers, username, password, errorMessage, manager);

    if (success && manager)
    {
        currentManager = manager;
        loginView->showSuccessMessage("Đăng nhập thành công!");

        // Khởi dựng Shell làm việc theo phân quyền tài khoản
        setupShell();
        if (windowStack && root)
        {
            windowStack->addWidget(root);
            windowStack->setCurrentWidget(root);
        }
        setWindowTitle("An Tâm · Quản lý phòng khám");
        setPage(0);
    }
    else
    {
        loginView->showErrorMessage(errorMessage);
    }
}

void MainWindow::handleLogout()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Đăng xuất");
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setFixedSize(430, 200);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 14px;
        }
    )");

    auto *rootLayout = new QVBoxLayout(&dialog);
    rootLayout->setContentsMargins(24, 22, 24, 20);
    rootLayout->setSpacing(18);

    auto *contentRow = new QHBoxLayout;
    contentRow->setSpacing(16);

    // Icon đăng xuất bo tròn màu đỏ nhạt
    auto *iconBox = new QLabel("↪");
    iconBox->setFixedSize(48, 48);
    iconBox->setAlignment(Qt::AlignCenter);
    iconBox->setStyleSheet(R"(
        QLabel {
            background: #fee2e2;
            color: #dc2626;
            font-size: 24px;
            font-weight: bold;
            border-radius: 24px;
        }
    )");
    contentRow->addWidget(iconBox, 0, Qt::AlignTop);

    auto *textCol = new QVBoxLayout;
    textCol->setSpacing(6);
    auto *titleLabel = new QLabel("Xác nhận đăng xuất");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #0f172a;");

    auto *descLabel = new QLabel("Bạn có chắc chắn muốn đăng xuất khỏi hệ thống không? Dữ liệu phiên làm việc sẽ được lưu tự động.");
    descLabel->setStyleSheet("font-size: 13px; color: #64748b; line-height: 1.4;");
    descLabel->setWordWrap(true);

    textCol->addWidget(titleLabel);
    textCol->addWidget(descLabel);
    contentRow->addLayout(textCol, 1);
    rootLayout->addLayout(contentRow);

    // Nút bấm hành động
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton("Hủy bỏ");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: #f1f5f9;
            color: #475569;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            font-size: 13px;
            font-weight: 600;
            padding: 8px 18px;
            min-height: 20px;
        }
        QPushButton:hover {
            background: #e2e8f0;
            color: #0f172a;
        }
    )");
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *confirmBtn = new QPushButton("Đăng xuất");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background: #ef4444;
            color: #ffffff;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: 700;
            padding: 8px 20px;
            min-height: 20px;
        }
        QPushButton:hover {
            background: #dc2626;
        }
    )");
    connect(confirmBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);
    rootLayout->addLayout(btnRow);

    if (dialog.exec() == QDialog::Accepted)
    {
        saveAll();
        currentManager = nullptr;
        showLogin();
    }
}

// --- SHELL GIAO DIỆN CHÍNH ---
void MainWindow::setupShell()
{
    if (root)
    {
        if (windowStack)
            windowStack->removeWidget(root);
        delete root;
        root = nullptr;
    }

    root = new QWidget;
    root->setObjectName("appRoot");

    auto *outer = new QHBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(238);

    auto *side = new QVBoxLayout(sidebar);
    side->setContentsMargins(18, 22, 18, 16);
    side->setSpacing(8);

    // Tiêu đề thương hiệu chuẩn ✦ AN TÂM
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
        "📊   Thống kê"};
    for (int i = 0; i < labels.size(); ++i)
    {
        auto *b = button(labels[i], "nav");
        navButtons.push_back(b);
        connect(b, &QPushButton::clicked, this, [this, i]
        {
            refreshPage(i);
            setPage(i);
        });
        side->addWidget(b);
    }

    if (isAdministrator())
    {
        auto *b = button("♙   Quản lý tài khoản", "nav");
        navButtons.push_back(b);
        connect(b, &QPushButton::clicked, this, [this]
        {
            refreshPage(6);
            setPage(6);
        });
        side->addWidget(b);
    }

    side->addStretch();

    // Khung tài khoản góc dưới bên trái (nhích lên phía trên thời gian thực)
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
    userLayout->setContentsMargins(12, 10, 12, 10);
    userLayout->setSpacing(8);

    auto *profileRow = new QHBoxLayout;
    profileRow->setSpacing(10);

    QLabel *avatarLabel = new QLabel;
    avatarLabel->setFixedSize(38, 38);
    QString currentName = currentManager ? UIHelper::qs(currentManager->getfullName()) : "Quản trị viên";
    avatarLabel->setPixmap(UIHelper::createAvatar(currentName, 38));
    profileRow->addWidget(avatarLabel);

    auto *nameCol = new QVBoxLayout;
    nameCol->setSpacing(2);
    QLabel *nameLabel = new QLabel(currentName);
    nameLabel->setStyleSheet("color: white; font-weight: 700; font-size: 13px;");
    nameLabel->setWordWrap(true);

    QLabel *roleLabel = new QLabel(currentManager ? ManagerController::roleName(currentManager->getRole()) : "Quản trị hệ thống");
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
    connect(logout, &QPushButton::clicked, this, &MainWindow::handleLogout);
    userLayout->addWidget(logout);
    side->addWidget(userCard);

    side->addSpacing(8);

    // Khung thời gian thực phía dưới account
    QFrame *timeCard = new QFrame;
    timeCard->setObjectName("timeCard");
    timeCard->setStyleSheet(R"(
            QFrame#timeCard {
                background: #14384e;
                border: 1px solid #1e4b66;
                border-radius: 12px;
            }
        )");
    auto *timeLayout = new QVBoxLayout(timeCard);
    timeLayout->setContentsMargins(12, 8, 12, 8);
    timeLayout->setSpacing(2);

    auto *timeRow = new QHBoxLayout;
    timeRow->setSpacing(7);

    auto *clockDot = new QLabel("●");
    clockDot->setStyleSheet("color: #2dd4bf; font-size: 11px; background: transparent; border: none;");

    auto *timeLabel = new QLabel;
    timeLabel->setStyleSheet("color: #f8fafc; font-weight: 750; font-size: 14.5px; font-family: 'Consolas', 'Segoe UI', monospace; letter-spacing: 0.5px; background: transparent; border: none;");

    timeRow->addWidget(clockDot);
    timeRow->addWidget(timeLabel);
    timeRow->addStretch();

    auto *dateLabel = new QLabel;
    dateLabel->setStyleSheet("color: #8faec0; font-size: 11px; font-weight: 500; margin-left: 2px; background: transparent; border: none;");

    timeLayout->addLayout(timeRow);
    timeLayout->addWidget(dateLabel);

    auto updateClock = [timeLabel, dateLabel]() {
        QDateTime now = QDateTime::currentDateTime();
        timeLabel->setText(now.toString("HH:mm:ss"));
        static const QString dowNames[] = {
            "", "Thứ Hai", "Thứ Ba", "Thứ Tư", "Thứ Năm", "Thứ Sáu", "Thứ Bảy", "Chủ Nhật"
        };
        int dow = now.date().dayOfWeek();
        QString dowStr = (dow >= 1 && dow <= 7) ? dowNames[dow] : "";
        dateLabel->setText(QStringLiteral("%1, %2").arg(dowStr, now.toString("dd/MM/yyyy")));
    };

    auto *clockTimer = new QTimer(timeCard);
    connect(clockTimer, &QTimer::timeout, timeCard, updateClock);
    clockTimer->start(1000);
    updateClock();

    side->addWidget(timeCard);

    pages = new QStackedWidget;
    outer->addWidget(sidebar);
    outer->addWidget(pages, 1);

    // Khởi tạo các Views từ từng module
    appointmentView = new AppointmentView(this, this);

    pages->addWidget(appointmentView->createDashboardWidget(pages)); // index 0: Tổng quan

    pages->addWidget(appointmentView->createBookingWidget(pages)); // index 1: Đặt lịch khám

    patientView = new PatientView(this, pages);
    pages->addWidget(patientView); // index 2: Bệnh nhân

    doctorView = new DoctorView(this, pages);
    pages->addWidget(doctorView); // index 3: Bác sĩ

    pages->addWidget(appointmentView->createAppointmentListWidget(pages)); // index 4: Quản lý lịch khám

    statisticView = new StatisticView(this, pages);
    pages->addWidget(statisticView); // index 5: Thống kê

    if (isAdministrator())
    {
        accountView = new AccountView(this, pages);
        pages->addWidget(accountView); // index 6: Quản lý tài khoản
    }

    if (windowStack)
    {
        windowStack->addWidget(root);
        windowStack->setCurrentWidget(root);
    }

    updateNavigation();
    setPage(0);
}

void MainWindow::refreshPage(int index)
{
    if (!pages)
        return;

    switch (index)
    {
    case 0:
        if (appointmentView)
        {
            QWidget *old = pages->widget(0);
            pages->removeWidget(old);
            delete old;
            pages->insertWidget(0, appointmentView->createDashboardWidget(pages));
        }
        break;
    case 1:
        if (appointmentView)
        {
            QWidget *old = pages->widget(1);
            pages->removeWidget(old);
            delete old;
            pages->insertWidget(1, appointmentView->createBookingWidget(pages));
        }
        break;
    case 2:
        if (patientView)
            patientView->resetToFirstPage();
        break;
    case 3:
        if (doctorView)
            doctorView->resetToFirstPage();
        break;
    case 4:
        if (appointmentView)
        {
            QWidget *old = pages->widget(4);
            pages->removeWidget(old);
            delete old;
            pages->insertWidget(4, appointmentView->createAppointmentListWidget(pages));
        }
        break;
    case 5:
        if (statisticView)
            statisticView->refresh();
        break;
    case 6:
        if (accountView)
            accountView->resetToFirstPage();
        break;
    }
}

void MainWindow::saveAll()
{
    bool ok = Patient::saveToFile(patientPath, patients) &&
              Manager::saveToFile(managerPath, managers) &&
              Doctor::saveToFile(doctorPath, doctors) &&
              SymptomPatient::saveToFile(symptomPatientPath, symptomPatients) &&
              Appointment::saveToFile(appointmentPath, appointments);
    if (!ok)
    {
        QMessageBox::warning(this, "Lỗi lưu dữ liệu", "Không thể lưu một hoặc nhiều tệp dữ liệu.");
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setStyleSheet(style());
    setMinimumSize(1120, 720);
    resize(1360, 850);
    setWindowTitle("An Tâm · Quản lý phòng khám");

    QIcon appIcon(":/assets/textures/icon_app.ico");
    if (appIcon.isNull())
        appIcon = QIcon(":/assets/textures/icon_app.png");
    if (appIcon.isNull())
        appIcon = QIcon("assets/textures/icon_app.ico");
    if (appIcon.isNull())
        appIcon = QIcon("assets/textures/icon_app.png");
    if (!appIcon.isNull())
        setWindowIcon(appIcon);

    initApp();
}

MainWindow::~MainWindow()
{
    delete managers;
    delete patients;
    delete doctors;
    delete symptoms;
    delete symptomPatients;
    delete appointments;
}

bool MainWindow::initApp()
{
    // Nạp dữ liệu từ các file
    managers = Manager::loadFromFile(managerPath);
    patients = Patient::loadFromFile(patientPath);
    doctors = Doctor::loadFromFile(doctorPath);
    symptoms = Symptom::loadFromFile(symptomPath);
    symptomPatients = SymptomPatient::loadFromFile(symptomPatientPath);
    appointments = Appointment::loadFromFile(appointmentPath);

    bool ok = managers && patients && doctors && symptoms && symptomPatients && appointments;
    if (!ok)
    {
        QMessageBox::critical(this, "Lỗi dữ liệu", "Không thể nạp dữ liệu từ thư mục Data/! Vui lòng đảm bảo tệp dữ liệu tồn tại và ứng dụng được khởi chạy từ đúng thư mục.");
        return false;
    }

    // Cập nhật trạng thái triệu chứng
    auto *p = patients->getHeader()->next;
    while (p && p->next)
    {
        updatePatientSymptomStatus(p->info.getID());
        p = p->next;
    }

    // Khởi tạo windowStack và nạp màn hình Login
    windowStack = new QStackedWidget(this);
    setCentralWidget(windowStack);

    loginView = new Login(windowStack);
    windowStack->addWidget(loginView);
    connect(loginView, &Login::loginClicked, this, &MainWindow::handleLogin);

    showLogin();
    return true;
}
