#include "DoctorView.h"
#include "MainWindow.h"
#include "DoctorController.h"
#include "UIHelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QLocale>
#include <QHeaderView>
#include <QLabel>
#include <QFrame>
#include <QTimer>

DoctorView::DoctorView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), mainWindow(mainWindow) {
    setupUI();
    refresh();
}

void DoctorView::setupUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(36, 32, 36, 32);
    layout->setSpacing(20);

    // Header Title
    auto *h = new QVBoxLayout;
    h->setSpacing(4);
    auto *t = new QLabel("Đội ngũ bác sĩ");
    t->setObjectName("pageTitle");
    auto *s = new QLabel("Danh sách bác sĩ và chuyên khoa đang hoạt động tại phòng khám.");
    s->setObjectName("pageSubtitle");
    h->addWidget(t);
    h->addWidget(s);
    layout->addLayout(h);

    // Actions bar
    auto *actions = new QHBoxLayout;
    search = new QLineEdit;
    search->setPlaceholderText("Tìm bác sĩ hoặc chuyên khoa…");
    addBtn = UIHelper::button("+  Thêm bác sĩ");
    actions->addWidget(search, 1);
    actions->addWidget(addBtn);
    layout->addLayout(actions);

    // Table: không hiển thị mã bác sĩ
    table = UIHelper::table({"BÁC SĨ", "CHUYÊN KHOA", "PHÒNG", "PHÍ KHÁM", "ĐÁNH GIÁ", "TRẠNG THÁI"});
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
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    table->setColumnWidth(2, 160);
    table->setColumnWidth(3, 150);
    table->setColumnWidth(4, 115);
    table->setColumnWidth(5, 185);
    table->verticalHeader()->setDefaultSectionSize(48);

    // Căn lề tên trường trùng khớp với dữ liệu bên dưới
    if (table->horizontalHeaderItem(0)) table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(1)) table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(2)) table->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    if (table->horizontalHeaderItem(3)) table->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(4)) table->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);
    if (table->horizontalHeaderItem(5)) table->horizontalHeaderItem(5)->setTextAlignment(Qt::AlignCenter);

    layout->addWidget(table, 1);

    // Thanh phân trang (50 người / trang)
    auto *pagination = new QHBoxLayout;
    pagination->setContentsMargins(4, 4, 4, 0);

    totalLabel = new QLabel("Đang hoạt động: 0 / 0 bác sĩ");
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

    if (addBtn) {
        connect(addBtn, &QPushButton::clicked, this, [this] {
            addDoctorDialog();
            refresh();
        });
    }

    // Nháy đúp chuột để chỉnh sửa thông tin bác sĩ
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row < 0 || row >= table->rowCount()) return;
        auto *item = table->item(row, 0);
        if (!item) return;
        std::string id = UIHelper::ss(item->data(Qt::UserRole).toString());
        if (id.empty()) return;
        editDoctorDialog(id);
        refresh();
    });
}

void DoctorView::resetToFirstPage() {
    currentPage = 0;
    refresh();
}

void DoctorView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    currentPage = 0;
    refresh();
}

void DoctorView::refresh() {
    if (!mainWindow || !table) return;
    table->setRowCount(0);

    auto *doctors = mainWindow->getDoctors();
    if (!doctors) return;

    QString q = search ? search->text().trimmed().toLower() : "";
    QList<Doctor*> matched;
    int activeCount = 0;

    auto *n = doctors->getHeader()->next;
    while (n && n->next) {
        QString name = UIHelper::qs(n->info.getfullName());
        QString spec = mainWindow->specialty(n->info.getSpId());
        QString room = UIHelper::formatRoom(UIHelper::qs(n->info.getRoomId()));
        QString id = UIHelper::qs(n->info.getID());

        if (q.isEmpty() || name.toLower().contains(q) || spec.toLower().contains(q) || room.toLower().contains(q) || id.toLower().contains(q)) {
            matched.append(&n->info);
            if (n->info.getState()) {
                activeCount++;
            }
        }
        n = n->next;
    }

    int total = matched.size();
    int totalPages = total == 0 ? 1 : (total + PAGE_SIZE - 1) / PAGE_SIZE;
    currentPage = qBound(0, currentPage, totalPages - 1);

    if (totalLabel) {
        totalLabel->setText(QStringLiteral("Đang hoạt động: %1 / %2 bác sĩ").arg(activeCount).arg(total));
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
        Doctor *d = matched[i];
        int r = table->rowCount();
        table->insertRow(r);
        table->setRowHeight(r, 48);

        QString name = UIHelper::qs(d->getfullName());
        QString spec = mainWindow->specialty(d->getSpId());
        QString room = UIHelper::formatRoom(UIHelper::qs(d->getRoomId()));
        QString feeStr = QLocale(QLocale::Vietnamese).toString(d->getFee(), 'f', 0) + " ₫";
        QString ratingStr = QString::number(d->getRating(), 'f', 1) + " ★";
        bool working = d->getState();

        auto *item0 = new QTableWidgetItem(name);
        item0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item0->setData(Qt::UserRole, UIHelper::qs(d->getID()));
        item0->setToolTip("Nháy đúp chuột để sửa thông tin bác sĩ");
        QFont f = item0->font();
        f.setWeight(QFont::DemiBold);
        item0->setFont(f);
        table->setItem(r, 0, item0);

        auto *item1 = new QTableWidgetItem(spec);
        item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item1->setToolTip("Nháy đúp chuột để sửa thông tin bác sĩ");
        table->setItem(r, 1, item1);

        auto *item2 = new QTableWidgetItem(room);
        item2->setTextAlignment(Qt::AlignCenter);
        item2->setToolTip("Nháy đúp chuột để sửa thông tin bác sĩ");
        table->setItem(r, 2, item2);

        auto *item3 = new QTableWidgetItem(feeStr);
        item3->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        item3->setToolTip("Nháy đúp chuột để sửa thông tin bác sĩ");
        table->setItem(r, 3, item3);

        auto *item4 = new QTableWidgetItem(ratingStr);
        item4->setTextAlignment(Qt::AlignCenter);
        item4->setToolTip("Nháy đúp chuột để sửa thông tin bác sĩ");
        table->setItem(r, 4, item4);

        auto *item5 = new QTableWidgetItem(working ? "Đang nhận khám" : "Tạm nghỉ");
        item5->setTextAlignment(Qt::AlignCenter);
        item5->setForeground(working ? QColor("#059669") : QColor("#dc2626"));
        item5->setToolTip("Nháy đúp chuột để sửa thông tin bác sĩ");
        table->setItem(r, 5, item5);
    }
}

void DoctorView::editDoctorDialog(const std::string &id) {
    if (!mainWindow) return;
    Doctor *doctor = mainWindow->findDoctor(id);
    if (!doctor) return;
    if (!mainWindow->hasPermission("Per_2")) {
        QMessageBox::warning(this, "Không có quyền", "Bạn không có quyền chỉnh sửa thông tin bác sĩ.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Chỉnh sửa thông tin bác sĩ");
    dialog.setFixedWidth(560);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
            font-family: 'Segoe UI', sans-serif;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
            color: #0f172a;
            min-height: 20px;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1.5px solid #0d9488;
            background: #ffffff;
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
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            padding: 4px;
        }
    )");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);

    // 1. Header Banner
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(12);

    auto *icon = new QLabel("👨‍⚕️");
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 22px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel("Chỉnh sửa thông tin bác sĩ");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel("Cập nhật chuyên khoa, phòng khám và thông tin làm việc");
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    root->addLayout(headerBox);

    // 2. Card Container
    auto *card = new QFrame;
    card->setObjectName("doctorCard");
    card->setStyleSheet("QFrame#doctorCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
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

    auto *name = new QLineEdit(UIHelper::qs(doctor->getfullName()));
    cardLayout->addLayout(makeField("Họ và tên bác sĩ", name));

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(14);
    auto *phone = new QLineEdit(UIHelper::qs(doctor->getPhone()));
    auto *room = new QLineEdit(UIHelper::qs(doctor->getRoomId()));
    row1->addLayout(makeField("Số điện thoại liên hệ", phone), 1);
    row1->addLayout(makeField("Phòng khám (ví dụ: Room 101)", room), 1);
    cardLayout->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(14);
    auto *age = new QSpinBox;
    age->setRange(18, 120);
    age->setValue(doctor->getAge());

    auto *gender = new QComboBox;
    gender->addItems({"Nam", "Nữ"});
    gender->setCurrentIndex(doctor->getGender() ? 0 : 1);

    row2->addLayout(makeField("Tuổi", age), 1);
    row2->addLayout(makeField("Giới tính", gender), 1);
    cardLayout->addLayout(row2);

    auto *row3 = new QHBoxLayout;
    row3->setSpacing(14);
    auto *specialtyBox = new QComboBox;
    auto specList = DoctorController::getSpecializationList("Data/Doctor/Specialization.txt");
    for (const auto &item : specList) {
        specialtyBox->addItem(item.first, item.second);
    }
    specialtyBox->setCurrentIndex(specialtyBox->findData(UIHelper::qs(doctor->getSpId())));

    auto *state = new QComboBox;
    state->addItems({"Đang làm việc", "Đã nghỉ việc"});
    state->setCurrentIndex(doctor->getState() ? 0 : 1);

    row3->addLayout(makeField("Chuyên khoa", specialtyBox), 1);
    row3->addLayout(makeField("Trạng thái công tác", state), 1);
    cardLayout->addLayout(row3);

    auto *row4 = new QHBoxLayout;
    row4->setSpacing(14);
    auto *fee = new QDoubleSpinBox;
    fee->setRange(0, 100000000);
    fee->setDecimals(0);
    fee->setValue(doctor->getFee());
    fee->setSuffix(" đ");

    auto *rating = new QDoubleSpinBox;
    rating->setRange(0, 5);
    rating->setSingleStep(0.1);
    rating->setValue(doctor->getRating());
    rating->setSuffix(" ★");

    row4->addLayout(makeField("Phí khám tham chiếu", fee), 1);
    row4->addLayout(makeField("Đánh giá chuyên môn", rating), 1);
    cardLayout->addLayout(row4);

    root->addWidget(card);

    // 3. Footer Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancel = new QPushButton("Hủy");
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("QPushButton { background: #ffffff; color: #475569; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 9px 22px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #f1f5f9; color: #0f172a; }");

    auto *save = new QPushButton("✓  Lưu thay đổi");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background: #0d9488; color: #ffffff; border: none; border-radius: 8px; padding: 9px 24px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #0f766e; }");

    btnRow->addWidget(cancel);
    btnRow->addWidget(save);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, [this, &dialog, doctor, name, phone, room, fee, rating, age, gender, specialtyBox, state] {
        if (name->text().trimmed().isEmpty() || phone->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập họ tên và điện thoại.");
            return;
        }
        doctor->setfullName(UIHelper::ss(name->text().trimmed()));
        doctor->setPhone(UIHelper::ss(phone->text().trimmed()));
        doctor->setAge(age->value());
        doctor->setGender(gender->currentIndex() == 0);
        doctor->setSpId(UIHelper::ss(specialtyBox->currentData().toString()));
        doctor->setRoomId(UIHelper::ss(room->text().trimmed()));
        doctor->setFee(fee->value());
        doctor->setRating(rating->value());
        doctor->setState(state->currentIndex() == 0);

        mainWindow->saveAll();
        dialog.accept();
    });
    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}

void DoctorView::addDoctorDialog() {
    if (!mainWindow) return;
    if (!mainWindow->hasPermission("Per_2")) {
        QMessageBox::warning(this, "Không có quyền", "Bạn không có quyền thêm bác sĩ mới.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Thêm bác sĩ mới");
    dialog.setFixedWidth(560);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
            font-family: 'Segoe UI', sans-serif;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13px;
            color: #0f172a;
            min-height: 20px;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1.5px solid #0d9488;
            background: #ffffff;
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
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            padding: 4px;
        }
    )");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(16);

    // 1. Header Banner
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(12);

    auto *icon = new QLabel("👨‍⚕️");
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 22px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel("Thêm bác sĩ mới");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel("Tạo hồ sơ và thông tin công tác cho bác sĩ mới");
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    root->addLayout(headerBox);

    // 2. Card Container
    auto *card = new QFrame;
    card->setObjectName("doctorCard");
    card->setStyleSheet("QFrame#doctorCard { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
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

    std::string newId = DoctorController::generateNextId(mainWindow->getDoctors());
    auto *idRow = new QHBoxLayout;
    idRow->setSpacing(14);
    auto *idInput = new QLineEdit(UIHelper::qs(newId));
    idInput->setReadOnly(true);
    idInput->setStyleSheet("QLineEdit { background: #f1f5f9; color: #0d9488; font-weight: 700; border: 1.5px solid #cbd5e1; }");
    idRow->addLayout(makeField("Mã bác sĩ (Tự động cấp)", idInput), 1);

    auto *name = new QLineEdit;
    name->setPlaceholderText("Ví dụ: Nguyễn Văn An");
    idRow->addLayout(makeField("Họ và tên bác sĩ", name), 2);
    cardLayout->addLayout(idRow);

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(14);
    auto *phone = new QLineEdit;
    phone->setPlaceholderText("Ví dụ: 0912345678");
    auto *room = new QLineEdit;
    room->setPlaceholderText("Ví dụ: Room 101");
    row1->addLayout(makeField("Số điện thoại liên hệ", phone), 1);
    row1->addLayout(makeField("Phòng khám (ví dụ: Room 101)", room), 1);
    cardLayout->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(14);
    auto *age = new QSpinBox;
    age->setRange(18, 120);
    age->setValue(32);

    auto *gender = new QComboBox;
    gender->addItems({"Nam", "Nữ"});
    gender->setCurrentIndex(0);

    row2->addLayout(makeField("Tuổi", age), 1);
    row2->addLayout(makeField("Giới tính", gender), 1);
    cardLayout->addLayout(row2);

    auto *row3 = new QHBoxLayout;
    row3->setSpacing(14);
    auto *specialtyBox = new QComboBox;
    auto specList = DoctorController::getSpecializationList("Data/Doctor/Specialization.txt");
    for (const auto &item : specList) {
        specialtyBox->addItem(item.first, item.second);
    }

    auto *state = new QComboBox;
    state->addItems({"Đang làm việc", "Đã nghỉ việc"});
    state->setCurrentIndex(0);

    row3->addLayout(makeField("Chuyên khoa", specialtyBox), 1);
    row3->addLayout(makeField("Trạng thái công tác", state), 1);
    cardLayout->addLayout(row3);

    auto *row4 = new QHBoxLayout;
    row4->setSpacing(14);
    auto *fee = new QDoubleSpinBox;
    fee->setRange(0, 100000000);
    fee->setDecimals(0);
    fee->setValue(200000);
    fee->setSuffix(" đ");

    auto *rating = new QDoubleSpinBox;
    rating->setRange(0, 5);
    rating->setSingleStep(0.1);
    rating->setValue(5.0);
    rating->setSuffix(" ★");

    row4->addLayout(makeField("Phí khám tham chiếu", fee), 1);
    row4->addLayout(makeField("Đánh giá ban đầu", rating), 1);
    cardLayout->addLayout(row4);

    root->addWidget(card);

    // 3. Footer Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);
    btnRow->addStretch();

    auto *cancel = new QPushButton("Hủy");
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet("QPushButton { background: #ffffff; color: #475569; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 9px 22px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #f1f5f9; color: #0f172a; }");

    auto *save = new QPushButton("✓  Thêm bác sĩ");
    save->setCursor(Qt::PointingHandCursor);
    save->setStyleSheet("QPushButton { background: #0d9488; color: #ffffff; border: none; border-radius: 8px; padding: 9px 24px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #0f766e; }");

    btnRow->addWidget(cancel);
    btnRow->addWidget(save);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, [this, &dialog, newId, name, phone, room, fee, rating, age, gender, specialtyBox, state] {
        QString nameStr = name->text().trimmed();
        QString phoneStr = phone->text().trimmed();
        QString roomStr = room->text().trimmed();
        if (nameStr.isEmpty() || phoneStr.isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng nhập đầy đủ họ tên và số điện thoại.");
            return;
        }
        if (roomStr.isEmpty()) {
            roomStr = "Room 101";
        }

        Doctor newDoctor(
            newId,
            UIHelper::ss(nameStr),
            UIHelper::ss(phoneStr),
            gender->currentIndex() == 0,
            age->value(),
            UIHelper::ss(specialtyBox->currentData().toString()),
            state->currentIndex() == 0,
            UIHelper::ss(roomStr),
            fee->value(),
            rating->value()
        );

        auto *docList = mainWindow->getDoctors();
        if (docList) {
            docList->insert(newDoctor);
            mainWindow->saveAll();
        }
        dialog.accept();
    });

    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}
