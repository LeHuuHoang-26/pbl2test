#include "PatientView.h"
#include "MainWindow.h"
#include "PatientController.h"
#include "UIHelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QFrame>
#include <QTimer>

PatientView::PatientView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), mainWindow(mainWindow) {
    setupUI();
    refresh();
}

void PatientView::setupUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(36, 32, 36, 32);
    layout->setSpacing(20);

    // Header Title
    auto *h = new QVBoxLayout;
    h->setSpacing(4);
    auto *t = new QLabel("Bệnh nhân");
    t->setObjectName("pageTitle");
    auto *s = new QLabel("Quản lý danh sách hồ sơ người bệnh.");
    s->setObjectName("pageSubtitle");
    h->addWidget(t);
    h->addWidget(s);
    layout->addLayout(h);

    // Actions bar
    auto *actions = new QHBoxLayout;
    search = new QLineEdit;
    search->setPlaceholderText("Tìm theo tên, điện thoại hoặc CCCD…");

    addBtn = UIHelper::button("＋  Thêm bệnh nhân");

    actions->addWidget(search, 1);
    actions->addWidget(addBtn);
    layout->addLayout(actions);

    if (mainWindow && !mainWindow->hasPermission("Per_3")) {
        addBtn->hide();
    }

    // Table: không hiển thị mã bệnh nhân, không hiển thị triệu chứng
    table = UIHelper::table({"HỌ VÀ TÊN", "ĐIỆN THOẠI", "CCCD", "TUỔI", "GIỚI TÍNH"});
    table->setCursor(Qt::PointingHandCursor);
    table->setStyleSheet(R"(
        QTableWidget {
            background-color: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 10px;
            gridline-color: transparent;
        }
        QHeaderView {
            border: none;
            background: transparent;
        }
        QHeaderView::section {
            background: #f8fafc;
            color: #475569;
            font-weight: 700;
            font-size: 11.5px;
            padding: 11px 14px;
            border: none;
            border-bottom: 2px solid #e2e8f0;
            border-left: none;
            border-right: none;
        }
        QTableWidget::item {
            background: #ffffff;
            border: none;
            border-bottom: 1px solid #f1f5f9;
            border-left: none;
            border-right: none;
            padding-left: 14px;
            padding-right: 14px;
            color: #1e293b;
            font-size: 13px;
        }
        QTableWidget::item:hover {
            background: #f0fdfa;
        }
    )");

    table->setWordWrap(false);
    table->setTextElideMode(Qt::ElideRight);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    table->setColumnWidth(1, 170);
    table->setColumnWidth(2, 190);
    table->setColumnWidth(3, 90);
    table->setColumnWidth(4, 110);
    table->verticalHeader()->setDefaultSectionSize(48);

    // Căn lề tên trường (Header) trùng khớp với dữ liệu bên dưới giống trang tổng quát
    if (table->horizontalHeaderItem(0)) table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(1)) table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(2)) table->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(3)) table->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    if (table->horizontalHeaderItem(4)) table->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);

    layout->addWidget(table, 1);

    // Thanh phân trang (50 người / trang)
    auto *pagination = new QHBoxLayout;
    pagination->setContentsMargins(4, 4, 4, 0);

    totalLabel = new QLabel("Tổng số: 0 bệnh nhân");
    totalLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #475569;");
    pagination->addWidget(totalLabel);

    pagination->addStretch();

    prevBtn = UIHelper::button("‹  Trang trước", "secondary");
    nextBtn = UIHelper::button("Trang sau  ›", "secondary");
    pageStatus = new QLabel("Trang 1 / 1");
    pageStatus->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #0f172a; padding: 0 12px;");

    pagination->addWidget(prevBtn);
    pagination->addWidget(pageStatus);
    pagination->addWidget(nextBtn);
    layout->addLayout(pagination);

    connect(prevBtn, &QPushButton::clicked, this, [this] {
        if (currentPage > 0) {
            currentPage--;
            refresh();
        }
    });

    connect(nextBtn, &QPushButton::clicked, this, [this] {
        currentPage++;
        refresh();
    });

    connect(search, &QLineEdit::textChanged, this, [this] {
        currentPage = 0;
        refresh();
    });

    connect(addBtn, &QPushButton::clicked, this, [this] {
        addPatientDialog();
        refresh();
    });

    // Tích hợp sửa hồ sơ khi click 2 lần chuột trái (double-click) vào hàng bệnh nhân
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row < 0 || row >= table->rowCount()) return;
        auto *item = table->item(row, 0);
        if (!item) return;
        std::string id = UIHelper::ss(item->data(Qt::UserRole).toString());
        if (id.empty()) return;
        editPatientDialog(id);
        refresh();
    });
}

void PatientView::resetToFirstPage() {
    currentPage = 0;
    refresh();
}

void PatientView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    currentPage = 0;
    refresh();
}

void PatientView::refresh() {
    if (!mainWindow || !table) return;
    table->setRowCount(0);

    auto *patients = mainWindow->getPatients();
    if (!patients) return;

    QString q = search ? search->text().trimmed().toLower() : "";
    QList<Patient*> matched;

    auto *n = patients->getHeader()->next;
    while (n && n->next) {
        QString name = UIHelper::qs(n->info.getfullName());
        QString id = UIHelper::qs(n->info.getID());
        QString phone = UIHelper::qs(n->info.getPhone());
        QString cccd = UIHelper::qs(n->info.getCCCD());

        if (q.isEmpty() || name.toLower().contains(q) || id.toLower().contains(q) || phone.contains(q) || cccd.contains(q)) {
            matched.append(&n->info);
        }
        n = n->next;
    }

    int total = matched.size();
    int totalPages = total == 0 ? 1 : (total + PAGE_SIZE - 1) / PAGE_SIZE;
    currentPage = qBound(0, currentPage, totalPages - 1);

    if (totalLabel) {
        totalLabel->setText(QStringLiteral("Tổng số: %1 bệnh nhân").arg(total));
    }
    if (pageStatus) {
        pageStatus->setText(QStringLiteral("Trang %1 / %2").arg(currentPage + 1).arg(totalPages));
    }
    if (prevBtn) {
        prevBtn->setEnabled(currentPage > 0);
    }
    if (nextBtn) {
        nextBtn->setEnabled(currentPage < totalPages - 1);
    }

    int start = currentPage * PAGE_SIZE;
    int end = qMin(start + PAGE_SIZE, total);

    for (int i = start; i < end; ++i) {
        Patient *p = matched[i];
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 48);

        QString name = UIHelper::qs(p->getfullName());
        QString phone = UIHelper::qs(p->getPhone());
        QString cccd = UIHelper::qs(p->getCCCD());

        auto *item0 = new QTableWidgetItem(name);
        item0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item0->setData(Qt::UserRole, UIHelper::qs(p->getID()));
        item0->setToolTip("Nháy đúp chuột để sửa hồ sơ bệnh nhân");
        QFont f = item0->font();
        f.setWeight(QFont::DemiBold);
        item0->setFont(f);
        table->setItem(r, 0, item0);

        auto *item1 = new QTableWidgetItem(phone);
        item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item1->setToolTip("Nháy đúp chuột để sửa hồ sơ bệnh nhân");
        table->setItem(r, 1, item1);

        auto *item2 = new QTableWidgetItem(cccd);
        item2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item2->setToolTip("Nháy đúp chuột để sửa hồ sơ bệnh nhân");
        table->setItem(r, 2, item2);

        auto *item3 = new QTableWidgetItem(QString::number(p->getAge()));
        item3->setTextAlignment(Qt::AlignCenter);
        item3->setToolTip("Nháy đúp chuột để sửa hồ sơ bệnh nhân");
        table->setItem(r, 3, item3);

        auto *item4 = new QTableWidgetItem(p->getGender() ? "Nam" : "Nữ");
        item4->setTextAlignment(Qt::AlignCenter);
        item4->setToolTip("Nháy đúp chuột để sửa hồ sơ bệnh nhân");
        table->setItem(r, 4, item4);
    }
}

void PatientView::addPatientDialog() {
    if (!mainWindow) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Thêm bệnh nhân mới");
    dialog.setFixedWidth(540);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
            font-family: 'Segoe UI', sans-serif;
        }
        QLineEdit, QSpinBox, QComboBox {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
            color: #0f172a;
            min-height: 20px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1.5px solid #0d9488;
            background: #ffffff;
        }
        QSpinBox {
            padding-right: 30px;
        }
        QSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1.5px solid #94a3b8;
            border-bottom: 1px solid #94a3b8;
            border-top-right-radius: 7px;
            background: #e2e8f0;
        }
        QSpinBox::up-button:hover {
            background: #0d9488;
        }
        QSpinBox::up-button:hover QSpinBox::up-arrow {
            image: url(:/assets/textures/arrow_up_white.png);
        }
        QSpinBox::up-button:pressed {
            background: #0f766e;
        }
        QSpinBox::up-arrow {
            image: url(:/assets/textures/arrow_up_dark.png);
            width: 9px;
            height: 9px;
        }
        QSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 25px;
            border-left: 1.5px solid #94a3b8;
            border-bottom-right-radius: 7px;
            background: #e2e8f0;
        }
        QSpinBox::down-button:hover {
            background: #0d9488;
        }
        QSpinBox::down-button:hover QSpinBox::down-arrow {
            image: url(:/assets/textures/arrow_down_white.png);
        }
        QSpinBox::down-button:pressed {
            background: #0f766e;
        }
        QSpinBox::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 9px;
            height: 9px;
        }
        QComboBox {
            padding-right: 28px;
        }
        QComboBox:disabled {
            background: #f1f5f9;
            color: #94a3b8;
            border-color: #e2e8f0;
        }
        QComboBox::drop-down {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 28px;
            border: none;
            background: transparent;
        }
        QComboBox::drop-down:disabled {
            width: 0px;
            border: none;
        }
        QComboBox::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 11px;
            height: 11px;
            margin-right: 8px;
        }
        QComboBox::down-arrow:disabled {
            image: none;
        }
        QComboBox QAbstractItemView {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            outline: none;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding-left: 8px;
            border-radius: 6px;
        }
        QComboBox QAbstractItemView::item:hover {
            background: #f0fdfa;
            color: #0d9488;
        }
    )");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);

    // Header Banner
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(12);

    auto *icon = new QLabel("➕");
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 20px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel("Thêm bệnh nhân mới");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel("Nhập thông tin hồ sơ và định danh của người bệnh");
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    root->addLayout(headerBox);

    // Form Card Container
    auto *card = new QFrame;
    card->setObjectName("patientCard");
    card->setStyleSheet("QFrame#patientCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 20);
    cardLayout->setSpacing(14);

    auto makeField = [](const QString &labelText, QWidget *inputWidget) {
        auto *col = new QVBoxLayout;
        col->setSpacing(5);
        auto *lbl = new QLabel(labelText);
        lbl->setStyleSheet("font-size: 12px; font-weight: 700; color: #334155;");
        col->addWidget(lbl);
        col->addWidget(inputWidget);
        return col;
    };

    auto *name = new QLineEdit;
    name->setPlaceholderText("Ví dụ: Nguyễn Văn An");
    cardLayout->addLayout(makeField("Họ và tên bệnh nhân", name));

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(14);
    auto *phone = new QLineEdit;
    phone->setPlaceholderText("Ví dụ: 0905123456");
    auto *cccd = new QLineEdit;
    cccd->setPlaceholderText("Ví dụ: 048096001234");
    row1->addLayout(makeField("Số điện thoại liên hệ", phone), 1);
    row1->addLayout(makeField("Số CCCD / Định danh", cccd), 1);
    cardLayout->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(14);
    auto *age = new QSpinBox;
    age->setRange(1, 130);
    age->setValue(30);
    auto *gender = new QComboBox;
    gender->addItems({"Nam", "Nữ"});
    row2->addLayout(makeField("Tuổi", age), 1);
    row2->addLayout(makeField("Giới tính", gender), 1);
    cardLayout->addLayout(row2);

    root->addWidget(card);

    // Footer Buttons
    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    buttons->addStretch();
    auto *cancel = new QPushButton("Hủy");
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("QPushButton { background: #ffffff; color: #475569; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 9px 22px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #f1f5f9; color: #0f172a; }");

    auto *save = new QPushButton("Tạo hồ sơ");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background: #0d9488; color: #ffffff; border: none; border-radius: 8px; padding: 9px 24px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #0f766e; }");

    buttons->addWidget(cancel);
    buttons->addWidget(save);
    root->addLayout(buttons);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, [this, &dialog, name, phone, cccd, age, gender] {
        if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() || cccd->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đủ họ tên, điện thoại và CCCD.");
            return;
        }
        auto *patients = mainWindow->getPatients();
        if (patients) {
            auto *n = patients->getHeader()->next;
            while (n && n->next) {
                if (n->info.getPhone() == UIHelper::ss(phone->text().trimmed()) || n->info.getCCCD() == UIHelper::ss(cccd->text().trimmed())) {
                    QMessageBox::warning(&dialog, "Trùng hồ sơ", "Số điện thoại hoặc CCCD đã tồn tại.");
                    return;
                }
                n = n->next;
            }
        }

        std::string id = Patient::generateNextId(patients);
        if (patients) {
            patients->insert(Patient(id, UIHelper::ss(name->text().trimmed()), UIHelper::ss(phone->text().trimmed()), gender->currentIndex() == 0, age->value(), UIHelper::ss(cccd->text().trimmed())));
        }

        mainWindow->saveAll();
        refresh();
        dialog.accept();
    });
    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}

void PatientView::editPatientDialog(const std::string &id) {
    if (!mainWindow) return;
    Patient *patient = mainWindow->findPatient(id);
    if (!patient) return;
    if (!mainWindow->hasPermission("Per_3")) {
        QMessageBox::warning(this, "Không có quyền", "Bạn không có quyền chỉnh sửa hồ sơ bệnh nhân.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Chỉnh sửa bệnh nhân");
    dialog.setFixedWidth(520);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
            font-family: 'Segoe UI', sans-serif;
        }
        QLineEdit, QSpinBox, QComboBox {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
            color: #0f172a;
            min-height: 20px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1.5px solid #0d9488;
            background: #ffffff;
        }
        QSpinBox {
            padding-right: 30px;
        }
        QSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1.5px solid #94a3b8;
            border-bottom: 1px solid #94a3b8;
            border-top-right-radius: 7px;
            background: #e2e8f0;
        }
        QSpinBox::up-button:hover {
            background: #0d9488;
        }
        QSpinBox::up-button:hover QSpinBox::up-arrow {
            image: url(:/assets/textures/arrow_up_white.png);
        }
        QSpinBox::up-button:pressed {
            background: #0f766e;
        }
        QSpinBox::up-arrow {
            image: url(:/assets/textures/arrow_up_dark.png);
            width: 9px;
            height: 9px;
        }
        QSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 25px;
            border-left: 1.5px solid #94a3b8;
            border-bottom-right-radius: 7px;
            background: #e2e8f0;
        }
        QSpinBox::down-button:hover {
            background: #0d9488;
        }
        QSpinBox::down-button:hover QSpinBox::down-arrow {
            image: url(:/assets/textures/arrow_down_white.png);
        }
        QSpinBox::down-button:pressed {
            background: #0f766e;
        }
        QSpinBox::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 9px;
            height: 9px;
        }
        QComboBox {
            padding-right: 28px;
        }
        QComboBox:disabled {
            background: #f1f5f9;
            color: #94a3b8;
            border-color: #e2e8f0;
        }
        QComboBox::drop-down {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 28px;
            border: none;
            background: transparent;
        }
        QComboBox::drop-down:disabled {
            width: 0px;
            border: none;
        }
        QComboBox::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 11px;
            height: 11px;
            margin-right: 8px;
        }
        QComboBox::down-arrow:disabled {
            image: none;
        }
        QComboBox QAbstractItemView {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            outline: none;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding-left: 8px;
            border-radius: 6px;
        }
        QComboBox QAbstractItemView::item:hover {
            background: #f0fdfa;
            color: #0d9488;
        }
    )");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);

    // 1. Header Banner
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(12);

    auto *icon = new QLabel("👤");
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 22px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel("Chỉnh sửa bệnh nhân");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel("Cập nhật thông tin cá nhân và định danh của bệnh nhân");
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    root->addLayout(headerBox);

    // 2. White Card Container
    auto *card = new QFrame;
    card->setObjectName("editCard");
    card->setStyleSheet("QFrame#editCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 20);
    cardLayout->setSpacing(14);

    auto makeField = [](const QString &labelText, QWidget *inputWidget) {
        auto *col = new QVBoxLayout;
        col->setSpacing(5);
        auto *lbl = new QLabel(labelText);
        lbl->setStyleSheet("font-size: 12px; font-weight: 700; color: #334155;");
        col->addWidget(lbl);
        col->addWidget(inputWidget);
        return col;
    };

    auto *name = new QLineEdit(UIHelper::qs(patient->getfullName()));
    cardLayout->addLayout(makeField("Họ và tên bệnh nhân", name));

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(14);
    auto *phone = new QLineEdit(UIHelper::qs(patient->getPhone()));
    auto *cccd = new QLineEdit(UIHelper::qs(patient->getCCCD()));
    row1->addLayout(makeField("Số điện thoại liên hệ", phone), 1);
    row1->addLayout(makeField("Số CCCD / Định danh", cccd), 1);
    cardLayout->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(14);
    auto *age = new QSpinBox;
    age->setRange(1, 130);
    age->setValue(patient->getAge());

    auto *gender = new QComboBox;
    gender->addItems({"Nam", "Nữ"});
    gender->setCurrentIndex(patient->getGender() ? 0 : 1);

    row2->addLayout(makeField("Tuổi", age), 1);
    row2->addLayout(makeField("Giới tính", gender), 1);
    cardLayout->addLayout(row2);

    root->addWidget(card);

    // 3. Footer Buttons
    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    buttons->addStretch();
    auto *cancel = new QPushButton("Hủy");
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("QPushButton { background: #ffffff; color: #475569; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 9px 22px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #f1f5f9; color: #0f172a; }");

    auto *save = new QPushButton("✓  Lưu thay đổi");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background: #0d9488; color: #ffffff; border: none; border-radius: 8px; padding: 9px 24px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #0f766e; }");

    buttons->addWidget(cancel);
    buttons->addWidget(save);
    root->addLayout(buttons);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, [this, &dialog, patient, name, phone, cccd, age, gender] {
        if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() || cccd->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đủ họ tên, điện thoại và CCCD.");
            return;
        }
        auto *patients = mainWindow->getPatients();
        if (patients) {
            auto *n = patients->getHeader()->next;
            while (n && n->next) {
                if (&n->info != patient && (n->info.getPhone() == UIHelper::ss(phone->text().trimmed()) || n->info.getCCCD() == UIHelper::ss(cccd->text().trimmed()))) {
                    QMessageBox::warning(&dialog, "Trùng hồ sơ", "Số điện thoại hoặc CCCD đã tồn tại.");
                    return;
                }
                n = n->next;
            }
        }

        patient->setfullName(UIHelper::ss(name->text().trimmed()));
        patient->setPhone(UIHelper::ss(phone->text().trimmed()));
        patient->setCCCD(UIHelper::ss(cccd->text().trimmed()));
        patient->setAge(age->value());
        patient->setGender(gender->currentIndex() == 0);

        mainWindow->saveAll();
        dialog.accept();
    });
    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}
