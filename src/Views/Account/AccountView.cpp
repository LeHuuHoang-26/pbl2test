#include "AccountView.h"
#include "MainWindow.h"
#include "ManagerController.h"
#include "UIHelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QLabel>
#include <QFrame>
#include <QHeaderView>
#include <QShowEvent>
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QAction>
#include <QIcon>
#include <QTimer>

struct PermissionDef {
    const char *id;
    const char *name;
};

static const PermissionDef permissionDefinitions[] = {
    {"Per_1", "Cấp quyền tạm thời làm admin có thời hạn"},
    {"Per_2", "Chỉnh sửa thông tin bác sĩ"},
    {"Per_3", "Chỉnh sửa thông tin bệnh nhân"},
    {"Per_4", "Chỉnh sửa thông tin lịch hẹn khám bệnh"},
    {"Per_5", "Chỉnh sửa lịch nghỉ lễ / Tết"}
};
static const int permissionDefinitionCount = 5;

static QIcon createEyeIcon(bool isVisible) {
    int size = 48;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(size / 24.0, size / 24.0);

    QColor color = isVisible ? QColor("#0d9488") : QColor("#94a3b8");
    QPen pen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    QPainterPath eyePath;
    eyePath.moveTo(3.0, 12.0);
    eyePath.quadTo(12.0, 5.0, 21.0, 12.0);
    eyePath.quadTo(12.0, 19.0, 3.0, 12.0);
    p.drawPath(eyePath);

    if (isVisible) {
        p.setBrush(color);
        p.drawEllipse(QPointF(12.0, 12.0), 3.5, 3.5);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(13.2, 10.8), 1.0, 1.0);
    } else {
        p.drawEllipse(QPointF(12.0, 12.0), 2.5, 2.5);
        p.drawLine(QPointF(4.5, 4.5), QPointF(19.5, 19.5));
    }

    pix.setDevicePixelRatio(2.0);
    return QIcon(pix);
}

AccountView::AccountView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), mainWindow(mainWindow) {
    setupUI();
    refresh();
}

void AccountView::resetToFirstPage() {
    currentPage = 0;
    refresh();
}

void AccountView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    currentPage = 0;
    refresh();
}

void AccountView::setupUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(36, 32, 36, 32);
    layout->setSpacing(16);

    // Header Title
    auto *h = new QVBoxLayout;
    h->setSpacing(4);
    auto *t = new QLabel("Quản lý tài khoản");
    t->setObjectName("pageTitle");
    auto *s = new QLabel("Quản lý người dùng nhân viên và phân quyền bảo mật cho hệ thống.");
    s->setObjectName("pageSubtitle");
    h->addWidget(t);
    h->addWidget(s);
    layout->addLayout(h);

    // Actions bar: Bỏ "Khóa / mở khóa tài khoản", chỉ giữ lại "+ Thêm quản lý"
    auto *actions = new QHBoxLayout;
    addBtn = UIHelper::button("＋  Thêm quản lý");
    actions->addWidget(addBtn);
    actions->addStretch();
    layout->addLayout(actions);

    // Table: Bỏ cột mã, đặt các cột thẳng hàng tương tự tổng quan
    table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"HỌ VÀ TÊN", "TÀI KHOẢN", "VAI TRÒ", "TRẠNG THÁI"});
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setStyleSheet(R"(
        QTableWidget {
            background-color: transparent;
            border: none;
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
            padding: 10px 14px;
            border: none;
            border-top: 1px solid #e2e8f0;
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
        QTableWidget::item:selected {
            background-color: #ccfbf1;
            color: #0f766e;
            font-weight: 700;
        }
    )");
    table->setWordWrap(false);
    table->setTextElideMode(Qt::ElideRight);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(0)) table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(1)) table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(2)) table->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(3)) table->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    table->setColumnWidth(3, 160);
    table->verticalHeader()->setDefaultSectionSize(48);
    layout->addWidget(table, 1);

    // Pagination: Thêm page với stack là 8
    auto *pagination = new QHBoxLayout;
    pagination->setContentsMargins(4, 4, 4, 0);

    totalLabel = new QLabel("Tổng số: 0 tài khoản");
    totalLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #475569;");
    pagination->addWidget(totalLabel);

    pagination->addStretch();

    prevBtn = UIHelper::button("‹  Trang trước", "secondary");
    nextBtn = UIHelper::button("Trang sau  ›", "secondary");
    pageStatus = new QLabel("Trang 1 / 1");
    pageStatus->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #0f172a; padding: 0 10px;");

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

    connect(addBtn, &QPushButton::clicked, this, [this] {
        addManagerDialog();
        refresh();
    });

    // Dup click vào hàng thì có thể sửa thông tin
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row < 0 || row >= table->rowCount()) return;
        auto *item = table->item(row, 0);
        if (!item) return;
        std::string id = UIHelper::ss(item->data(Qt::UserRole).toString());
        if (id.empty()) return;
        editManagerDialog(id);
    });
}

void AccountView::refresh() {
    if (!mainWindow || !table) return;
    table->setRowCount(0);

    auto *managers = mainWindow->getManagers();
    if (!managers) return;

    QList<Manager*> allList;
    auto *n = managers->getHeader()->next;
    while (n && n->next) {
        if (!n->info.isDefault()) {
            allList.append(&n->info);
        }
        n = n->next;
    }

    int total = allList.size();
    int totalPages = total == 0 ? 1 : (total + PAGE_SIZE - 1) / PAGE_SIZE;
    currentPage = qBound(0, currentPage, totalPages - 1);

    if (totalLabel) {
        totalLabel->setText(QStringLiteral("Tổng số: %1 tài khoản").arg(total));
    }
    if (pageStatus) {
        pageStatus->setText(QStringLiteral("Trang %1 / %2").arg(currentPage + 1).arg(totalPages));
    }
    if (prevBtn) prevBtn->setEnabled(currentPage > 0);
    if (nextBtn) nextBtn->setEnabled(currentPage < totalPages - 1);

    int start = currentPage * PAGE_SIZE;
    int end = qMin(start + PAGE_SIZE, total);

    for (int i = start; i < end; ++i) {
        Manager *m = allList[i];
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 48);

        // Cột 0: Họ và tên (Lưu ID vào UserRole để double click lấy ra)
        auto *item0 = new QTableWidgetItem(UIHelper::qs(m->getfullName()));
        item0->setData(Qt::UserRole, UIHelper::qs(m->getID()));
        item0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QFont f = item0->font();
        f.setWeight(QFont::DemiBold);
        item0->setFont(f);
        item0->setForeground(QColor("#0f172a"));
        table->setItem(r, 0, item0);

        // Cột 1: Tài khoản
        auto *item1 = new QTableWidgetItem(UIHelper::qs(m->getUsername()));
        item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item1->setForeground(QColor("#334155"));
        table->setItem(r, 1, item1);

        // Cột 2: Vai trò
        auto *item2 = new QTableWidgetItem(ManagerController::roleName(m->getRole()));
        item2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item2->setForeground(QColor("#334155"));
        table->setItem(r, 2, item2);

        // Cột 3: Trạng thái (thẳng hàng giữa với tiêu đề TRẠNG THÁI)
        bool active = m->isActive();
        auto *item3 = new QTableWidgetItem(active ? "● Hoạt động" : "● Đã khóa");
        item3->setTextAlignment(Qt::AlignCenter);
        item3->setForeground(active ? QColor("#059669") : QColor("#dc2626"));
        QFont sf = item3->font();
        sf.setWeight(QFont::Bold);
        item3->setFont(sf);
        table->setItem(r, 3, item3);
    }
}

void AccountView::addManagerDialog() {
    if (!mainWindow) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Thêm tài khoản quản lý mới");
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
        QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {
            background: #f1f5f9;
            color: #64748b;
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
    auto *title = new QLabel("Thêm tài khoản quản lý mới");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel("Tạo tài khoản người dùng nhân viên mới cho hệ thống");
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    root->addLayout(headerBox);

    // Form Container
    auto *card = new QFrame;
    card->setObjectName("addCard");
    card->setStyleSheet("QFrame#addCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 20);
    cardLayout->setSpacing(12);

    auto makeField = [](const QString &labelText, QWidget *inputWidget) {
        auto *col = new QVBoxLayout;
        col->setSpacing(5);
        auto *lbl = new QLabel(labelText);
        lbl->setStyleSheet("font-weight: 700; font-size: 12px; color: #475569;");
        col->addWidget(lbl);
        col->addWidget(inputWidget);
        return col;
    };

    auto *name = new QLineEdit;
    auto *phone = new QLineEdit;
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(12);
    row1->addLayout(makeField("HỌ VÀ TÊN *", name), 3);
    row1->addLayout(makeField("SỐ ĐIỆN THOẠI *", phone), 2);
    cardLayout->addLayout(row1);

    auto *age = new QSpinBox;
    age->setRange(18, 120);
    age->setValue(28);

    auto *gender = new QComboBox;
    gender->addItems({"Nam", "Nữ"});

    auto *username = new QLineEdit;

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(12);
    row2->addLayout(makeField("TUỔI", age), 1);
    row2->addLayout(makeField("GIỚI TÍNH", gender), 1);
    row2->addLayout(makeField("TÊN ĐĂNG NHẬP *", username), 2);
    cardLayout->addLayout(row2);

    auto *password = new QLineEdit;
    password->setEchoMode(QLineEdit::Normal);
    auto *toggleAddAction = password->addAction(createEyeIcon(true), QLineEdit::TrailingPosition);
    connect(toggleAddAction, &QAction::triggered, password, [password, toggleAddAction]() {
        bool isNormal = (password->echoMode() == QLineEdit::Normal);
        password->setEchoMode(isNormal ? QLineEdit::Password : QLineEdit::Normal);
        toggleAddAction->setIcon(createEyeIcon(!isNormal));
    });
    cardLayout->addLayout(makeField("MẬT KHẨU *", password));

    auto *role = new QComboBox;
    role->addItems({"Lễ tân", "Lễ tân chuyên trách", "Quản trị viên"});
    cardLayout->addLayout(makeField("VAI TRÒ HỆ THỐNG", role));

    root->addWidget(card);

    // Buttons
    auto *btnBox = new QHBoxLayout;
    btnBox->addStretch();
    auto *cancelBtn = new QPushButton("Hủy bỏ");
    cancelBtn->setStyleSheet("QPushButton { background: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px 20px; font-size: 13px; font-weight: 600; color: #475569; } QPushButton:hover { background: #e2e8f0; color: #1e293b; }");
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *saveBtn = new QPushButton("＋  Tạo tài khoản");
    saveBtn->setStyleSheet("QPushButton { background: #0d9488; border: none; border-radius: 8px; padding: 9px 22px; font-size: 13px; font-weight: 700; color: white; } QPushButton:hover { background: #0f766e; }");

    connect(saveBtn, &QPushButton::clicked, &dialog, [this, &dialog, name, phone, username, password, age, gender, role] {
        if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() ||
            username->text().trimmed().isEmpty() || password->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đầy đủ các thông tin bắt buộc.");
            return;
        }
        auto *managers = mainWindow->getManagers();
        if (managers) {
            auto *n = managers->getHeader()->next;
            while (n && n->next) {
                if (n->info.getUsername() == UIHelper::ss(username->text().trimmed())) {
                    QMessageBox::warning(&dialog, "Trùng tài khoản", "Tên đăng nhập này đã được sử dụng. Vui lòng chọn tên khác.");
                    return;
                }
                n = n->next;
            }
        }

        std::string id = ManagerController::generateNextId(managers);
        ManagerRole r = role->currentIndex() == 2 ? ManagerRole::Administrator :
                       (role->currentIndex() == 1 ? ManagerRole::SpecialReceptionist : ManagerRole::Receptionist);

        if (managers) {
            managers->insert(Manager(id, UIHelper::ss(name->text().trimmed()), UIHelper::ss(phone->text().trimmed()), gender->currentIndex() == 0, age->value(), UIHelper::ss(username->text().trimmed()), UIHelper::ss(password->text()), r, 1));
        }
        mainWindow->saveAll();
        dialog.accept();
        QMessageBox::information(this, "Thành công", "Tài khoản quản lý mới đã được tạo thành công.");
    });

    btnBox->addWidget(cancelBtn);
    btnBox->addWidget(saveBtn);
    root->addLayout(btnBox);

    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}

void AccountView::editManagerDialog(const std::string &managerId) {
    if (!mainWindow) return;

    auto *managers = mainWindow->getManagers();
    if (!managers) return;

    Manager *targetMgr = ManagerController::findById(managers, managerId);
    if (!targetMgr) return;

    bool isTargetRoot = (targetMgr->getID() == "MA_001");
    bool isCurrentRoot = mainWindow->isRootAdmin();
    bool isSelf = (mainWindow->getCurrentManager() && mainWindow->getCurrentManager()->getID() == targetMgr->getID());

    // Nếu sửa tài khoản MA_001 mà bản thân không phải root admin thì không có quyền
    if (isTargetRoot && !isCurrentRoot) {
        QMessageBox::warning(this, "Không có quyền", "Chỉ quản trị viên gốc mới có quyền chỉnh sửa tài khoản này.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Chỉnh sửa thông tin tài khoản");
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
        QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {
            background: #f1f5f9;
            color: #64748b;
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

    auto *icon = new QLabel("👤");
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 22px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel("Chỉnh sửa thông tin tài khoản");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel("Cập nhật thông tin quản lý, vai trò và trạng thái hoạt động");
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    root->addLayout(headerBox);

    // Card Container
    auto *card = new QFrame;
    card->setObjectName("editCard");
    card->setStyleSheet("QFrame#editCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 20);
    cardLayout->setSpacing(12);

    auto makeField = [](const QString &labelText, QWidget *inputWidget) {
        auto *col = new QVBoxLayout;
        col->setSpacing(5);
        auto *lbl = new QLabel(labelText);
        lbl->setStyleSheet("font-weight: 700; font-size: 12px; color: #475569;");
        col->addWidget(lbl);
        col->addWidget(inputWidget);
        return col;
    };

    auto *name = new QLineEdit(UIHelper::qs(targetMgr->getfullName()));
    auto *phone = new QLineEdit(UIHelper::qs(targetMgr->getPhone()));
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(12);
    row1->addLayout(makeField("HỌ VÀ TÊN *", name), 3);
    row1->addLayout(makeField("SỐ ĐIỆN THOẠI *", phone), 2);
    cardLayout->addLayout(row1);

    auto *age = new QSpinBox;
    age->setRange(18, 120);
    age->setValue(targetMgr->getAge());

    auto *gender = new QComboBox;
    gender->addItems({"Nam", "Nữ"});
    gender->setCurrentIndex(targetMgr->getGender() ? 0 : 1);

    auto *username = new QLineEdit(UIHelper::qs(targetMgr->getUsername()));
    if (isTargetRoot) {
        username->setEnabled(false); // Không đổi username của root admin
    }

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(12);
    row2->addLayout(makeField("TUỔI", age), 1);
    row2->addLayout(makeField("GIỚI TÍNH", gender), 1);
    row2->addLayout(makeField("TÊN ĐĂNG NHẬP *", username), 2);
    cardLayout->addLayout(row2);

    auto *password = new QLineEdit(UIHelper::qs(targetMgr->getPassword()));
    password->setEchoMode(QLineEdit::Normal);
    auto *toggleEditAction = password->addAction(createEyeIcon(true), QLineEdit::TrailingPosition);
    connect(toggleEditAction, &QAction::triggered, password, [password, toggleEditAction]() {
        bool isNormal = (password->echoMode() == QLineEdit::Normal);
        password->setEchoMode(isNormal ? QLineEdit::Password : QLineEdit::Normal);
        toggleEditAction->setIcon(createEyeIcon(!isNormal));
    });
    cardLayout->addLayout(makeField("MẬT KHẨU *", password));

    auto *role = new QComboBox;
    role->addItems({"Lễ tân", "Lễ tân chuyên trách", "Quản trị viên"});
    int roleIdx = (targetMgr->getRole() == ManagerRole::Administrator) ? 2 :
                  ((targetMgr->getRole() == ManagerRole::SpecialReceptionist) ? 1 : 0);
    role->setCurrentIndex(roleIdx);
    if (isTargetRoot) {
        role->setEnabled(false); // Root admin luôn là Administrator
    }

    auto *statusCombo = new QComboBox;
    statusCombo->addItems({"Hoạt động", "Đã khóa"});
    statusCombo->setCurrentIndex(targetMgr->isActive() ? 0 : 1);
    if (isTargetRoot || isSelf) {
        statusCombo->setEnabled(false); // Không khóa root admin hoặc chính mình
    }

    auto *row3 = new QHBoxLayout;
    row3->setSpacing(12);
    row3->addLayout(makeField("VAI TRÒ HỆ THỐNG", role), 1);
    row3->addLayout(makeField("TRẠNG THÁI TÀI KHOẢN", statusCombo), 1);
    cardLayout->addLayout(row3);

    // Phân quyền đặc biệt: Hiển thị khi Root Admin chỉnh sửa tài khoản nhân viên
    struct PermCheckItem {
        QCheckBox *box;
        QSpinBox *spin;
    };
    auto *checks = new QList<PermCheckItem>;

    if (isCurrentRoot && !isTargetRoot) {
        auto *permDivider = new QFrame;
        permDivider->setFrameShape(QFrame::HLine);
        permDivider->setStyleSheet("background-color: #f1f5f9; max-height: 1px; border: none; margin: 4px 0;");
        cardLayout->addWidget(permDivider);

        auto *permTitle = new QLabel("Phân quyền nâng cao (Quản trị viên gốc)");
        permTitle->setStyleSheet("font-size: 13px; font-weight: 750; color: #0d9488;");
        cardLayout->addWidget(permTitle);

        QStringList granted = ManagerController::getGrantedPermissions(managerId, "Data/PermissionManager.txt");

        for (int i = 0; i < permissionDefinitionCount; ++i) {
            const auto &entry = permissionDefinitions[i];
            auto *check = new QCheckBox(QString::fromUtf8(entry.name));
            check->setStyleSheet("QCheckBox { font-size: 12.5px; color: #334155; } QCheckBox:hover { color: #0d9488; }");
            check->setChecked(granted.contains(QString::fromUtf8(entry.id)));
            check->setProperty("permissionId", QString::fromUtf8(entry.id));
            cardLayout->addWidget(check);

            QSpinBox *duration = nullptr;
            if (std::string(entry.id) == "Per_1") {
                auto *durRow = new QHBoxLayout;
                durRow->setContentsMargins(20, 0, 0, 0);
                auto *durLbl = new QLabel("Thời hạn hiệu lực (ngày):");
                durLbl->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 600;");
                duration = new QSpinBox;
                duration->setRange(1, 3650);
                duration->setValue(30);
                durRow->addWidget(durLbl);
                durRow->addWidget(duration);
                durRow->addStretch();
                cardLayout->addLayout(durRow);

                durLbl->setVisible(check->isChecked());
                duration->setVisible(check->isChecked());
                connect(check, &QCheckBox::toggled, [durLbl, duration](bool c) {
                    durLbl->setVisible(c);
                    duration->setVisible(c);
                });
            }
            checks->append({check, duration});
        }
    }

    root->addWidget(card);

    // Buttons
    auto *btnBox = new QHBoxLayout;
    btnBox->addStretch();
    auto *cancelBtn = new QPushButton("Hủy bỏ");
    cancelBtn->setStyleSheet("QPushButton { background: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px 20px; font-size: 13px; font-weight: 600; color: #475569; } QPushButton:hover { background: #e2e8f0; color: #1e293b; }");
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *saveBtn = new QPushButton("✓  Lưu thay đổi");
    saveBtn->setStyleSheet("QPushButton { background: #0d9488; border: none; border-radius: 8px; padding: 9px 22px; font-size: 13px; font-weight: 700; color: white; } QPushButton:hover { background: #0f766e; }");

    connect(saveBtn, &QPushButton::clicked, &dialog, [this, &dialog, targetMgr, managerId, name, phone, age, gender, username, password, role, statusCombo, isTargetRoot, isSelf, isCurrentRoot, checks] {
        if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty() ||
            username->text().trimmed().isEmpty() || password->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đầy đủ các thông tin bắt buộc.");
            return;
        }

        std::string newUsername = UIHelper::ss(username->text().trimmed());
        if (newUsername != targetMgr->getUsername()) {
            auto *managers = mainWindow->getManagers();
            if (managers) {
                auto *n = managers->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getID() != targetMgr->getID() && n->info.getUsername() == newUsername) {
                        QMessageBox::warning(&dialog, "Trùng tài khoản", "Tên đăng nhập này đã được sử dụng. Vui lòng chọn tên khác.");
                        return;
                    }
                    n = n->next;
                    }
            }
        }

        targetMgr->setfullName(UIHelper::ss(name->text().trimmed()));
        targetMgr->setPhone(UIHelper::ss(phone->text().trimmed()));
        targetMgr->setAge(age->value());
        targetMgr->setGender(gender->currentIndex() == 0);
        targetMgr->setUsername(newUsername);
        targetMgr->setPassword(UIHelper::ss(password->text().trimmed()));

        if (!isTargetRoot) {
            ManagerRole r = (role->currentIndex() == 2) ? ManagerRole::Administrator :
                           ((role->currentIndex() == 1) ? ManagerRole::SpecialReceptionist : ManagerRole::Receptionist);
            targetMgr->setRole(r);
        }

        if (!isTargetRoot && !isSelf) {
            targetMgr->setState(statusCombo->currentIndex() == 0 ? 1 : 0);
        }

        if (isCurrentRoot && !isTargetRoot) {
            QList<PermissionRecord> newGrants;
            for (const auto &entry : *checks) {
                if (entry.box->isChecked()) {
                    std::string expiry;
                    if (entry.spin) {
                        expiry = UIHelper::ss(QDateTime::currentDateTime().addDays(entry.spin->value()).toString(Qt::ISODate));
                    }
                    newGrants.append({managerId, UIHelper::ss(entry.box->property("permissionId").toString()), expiry});
                }
            }
            ManagerController::savePermissions(managerId, newGrants, "Data/PermissionManager.txt");
        }

        mainWindow->saveAll();
        refresh();
        dialog.accept();
        QMessageBox::information(this, "Thành công", "Thông tin tài khoản đã được cập nhật thành công.");
    });

    btnBox->addWidget(cancelBtn);
    btnBox->addWidget(saveBtn);
    root->addLayout(btnBox);

    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}
