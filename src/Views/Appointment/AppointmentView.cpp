#include "AppointmentView.h"
#include "MainWindow.h"
#include "AppointmentController.h"
#include "DoctorController.h"
#include "PatientController.h"
#include "StatisticController.h"
#include "UIHelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QCheckBox>
#include <QAbstractButton>
#include <QMessageBox>
#include <QLocale>
#include <QTime>
#include <QListView>
#include <QTimer>
#include <QScrollBar>
#include <QDateTime>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QMap>
#include <QStandardItemModel>
#include <QBoxLayout>

AppointmentView::AppointmentView(MainWindow *mainWindow, QObject *parent)
    : QObject(parent), mainWindow(mainWindow) {
}

QList<Appointment*> AppointmentView::getAppointmentsForDate(const QDate &targetDate) const {
    if (!mainWindow || !mainWindow->getAppointments()) return QList<Appointment*>();
    return AppointmentController::getAppointmentsForDate(mainWindow->getAppointments(), targetDate);
}

void AppointmentView::highlightCalendarAppointments(QCalendarWidget *cal, int year, int month) {
    if (!cal || !mainWindow || !mainWindow->getAppointments()) return;
    int dayCount[32] = {0};
    AppointmentController::getMonthDayCounts(mainWindow->getAppointments(), year, month, dayCount);

    // Xóa màu đỏ của Thứ 7 & Chủ Nhật trên tiêu đề tuần (bỏ hoàn toàn màu đỏ)
    QTextCharFormat weekdayHeaderFmt;
    weekdayHeaderFmt.setForeground(QColor("#475569"));
    weekdayHeaderFmt.setFontWeight(QFont::DemiBold);
    for (int dow = 1; dow <= 7; ++dow) {
        cal->setWeekdayTextFormat(static_cast<Qt::DayOfWeek>(dow), weekdayHeaderFmt);
    }

    // Lưới hiển thị của QCalendarWidget gồm 6 tuần = 42 ô
    QDate firstDay(year, month, 1);
    int firstDow = firstDay.dayOfWeek(); // 1 = Monday, 7 = Sunday
    QDate gridStart = firstDay.addDays(-(firstDow - 1));
    QDate gridEnd = gridStart.addDays(41);

    for (QDate d = gridStart; d <= gridEnd; d = d.addDays(1)) {
        QTextCharFormat fmt;
        if (d.month() != month) {
            // Ngày không thuộc tháng hiện tại: làm mờ nhạt rõ rệt so với ngày chính
            fmt.setForeground(QColor("#cbd5e1"));
            fmt.setFontWeight(QFont::Normal);
            fmt.clearBackground();
        } else {
            // Ngày trong tháng
            bool isHol = mainWindow->isHoliday(d);
            if (isHol) {
                // Ngày nghỉ lễ (5 ngày Tết và các ngày lễ quy định)
                fmt.setBackground(QColor("#fee2e2"));
                fmt.setForeground(QColor("#b91c1c"));
                fmt.setFontWeight(QFont::Bold);
                fmt.setToolTip(mainWindow->holidayNote(d));
            } else if (dayCount[d.day()] > 0) {
                fmt.setBackground(QColor("#ccfbf1"));
                fmt.setForeground(QColor("#0f766e"));
                fmt.setFontWeight(QFont::Bold);
            } else {
                fmt.setForeground(QColor("#0f172a"));
                fmt.setFontWeight(QFont::Normal);
                fmt.clearBackground();
            }
        }
        cal->setDateTextFormat(d, fmt);
    }
}

namespace {

static QString stripVietnameseAccents(const QString &str) {
    QString res = str.normalized(QString::NormalizationForm_D);
    QString out;
    for (const QChar &ch : res) {
        ushort u = ch.unicode();
        if (u < 0x0300 || u > 0x036F) {
            if (ch == QChar(0x0111)) out.append('d');
            else if (ch == QChar(0x0110)) out.append('D');
            else out.append(ch);
        }
    }
    return out;
}

class FormNoWheelFilter : public QObject {
public:
    static FormNoWheelFilter* instance() {
        static FormNoWheelFilter inst;
        return &inst;
    }
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Wheel) {
            auto *cb = qobject_cast<QComboBox*>(obj);
            if (cb) {
                event->ignore();
                return true;
            }
            auto *dt = qobject_cast<QDateTimeEdit*>(obj);
            if (dt) {
                event->ignore();
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

class ToggleSwitch : public QAbstractButton {
public:
    explicit ToggleSwitch(QWidget *parent = nullptr) : QAbstractButton(parent) {
        setCheckable(true);
        setChecked(false);
        setCursor(Qt::PointingHandCursor);
        setFixedHeight(22);
        setFixedWidth(40);
    }

    QSize sizeHint() const override {
        return QSize(40, 22);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QRect trackRect(1, 1, width() - 2, height() - 2);
        QColor trackColor = isChecked() ? QColor("#0d9488") : QColor("#cbd5e1");
        if (underMouse()) {
            trackColor = isChecked() ? QColor("#0f766e") : QColor("#94a3b8");
        }

        p.setPen(Qt::NoPen);
        p.setBrush(trackColor);
        p.drawRoundedRect(trackRect, trackRect.height() / 2.0, trackRect.height() / 2.0);

        int knobDiameter = trackRect.height() - 4;
        int knobX = isChecked() ? (trackRect.right() - knobDiameter - 2) : (trackRect.left() + 2);
        int knobY = trackRect.top() + 2;

        p.setBrush(Qt::white);
        p.drawEllipse(knobX, knobY, knobDiameter, knobDiameter);
    }
};

class ClickableLabel : public QLabel {
public:
    ToggleSwitch *targetSwitch = nullptr;
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr)
        : QLabel(text, parent) {}
protected:
    void mouseReleaseEvent(QMouseEvent *e) override {
        if (targetSwitch && e->button() == Qt::LeftButton) {
            targetSwitch->toggle();
        }
        QLabel::mouseReleaseEvent(e);
    }
};

class NoWheelComboBox : public QComboBox {
public:
    explicit NoWheelComboBox(QWidget *parent = nullptr) : QComboBox(parent) {
        auto *lv = new QListView(this);
        lv->setTextElideMode(Qt::ElideNone);
        lv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        lv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setView(lv);
        installEventFilter(FormNoWheelFilter::instance());
    }

    void showPopup() override {
        if (view()) {
            view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
        QComboBox::showPopup();
    }

protected:
    void wheelEvent(QWheelEvent *e) override {
        e->ignore();
    }
};

class SearchLineEdit : public QLineEdit {
public:
    QListView *listView = nullptr;
    QComboBox *combo = nullptr;

    explicit SearchLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

protected:
    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Escape) {
            if (combo) combo->hidePopup();
            return;
        }
        if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Up) {
            if (listView) {
                listView->setFocus();
                QApplication::sendEvent(listView, e);
                return;
            }
        }
        QLineEdit::keyPressEvent(e);
    }
};

class SearchablePatientComboBox : public NoWheelComboBox {
private:
    SearchLineEdit *searchEdit = nullptr;
    bool layoutConfigured = false;

public:
    explicit SearchablePatientComboBox(QWidget *parent = nullptr) : NoWheelComboBox(parent) {
        auto *lv = qobject_cast<QListView*>(this->view());
        if (lv) {
            lv->setTextElideMode(Qt::ElideNone);
            lv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            lv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            lv->setStyleSheet(R"(
                QListView {
                    border: none;
                    background-color: #ffffff;
                    color: #0f172a;
                    outline: none;
                    padding: 4px;
                    font-size: 13.5px;
                }
                QListView::item {
                    min-height: 32px;
                    padding-left: 10px;
                    border-radius: 6px;
                }
                QListView::item:hover {
                    background-color: #f0fdfa;
                    color: #0d9488;
                }
                QListView::item:selected {
                    background-color: #0d9488;
                    color: #ffffff;
                }
                QScrollBar:vertical {
                    width: 0px;
                    height: 0px;
                    background: transparent;
                }
            )");
        }

        searchEdit = new SearchLineEdit;
        searchEdit->listView = lv;
        searchEdit->combo = this;
        searchEdit->setPlaceholderText(QString::fromUtf8("🔍 Tìm kiếm tên hoặc mã bệnh nhân..."));
        searchEdit->setClearButtonEnabled(true);
        searchEdit->setStyleSheet(R"(
            QLineEdit {
                background: #f8fafc;
                border: 1.5px solid #cbd5e1;
                border-radius: 6px;
                padding: 7px 10px;
                font-size: 13px;
                color: #0f172a;
                margin: 4px 6px;
            }
            QLineEdit:focus {
                border: 1.5px solid #0d9488;
                background: #ffffff;
            }
        )");

        connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            filterItems(text);
        });

        connect(searchEdit, &QLineEdit::returnPressed, this, [this]() {
            auto *lv = qobject_cast<QListView*>(this->view());
            auto *m = qobject_cast<QStandardItemModel*>(this->model());
            if (!lv || !m) return;
            for (int r = 1; r < m->rowCount(); ++r) {
                if (!lv->isRowHidden(r)) {
                    setCurrentIndex(r);
                    hidePopup();
                    break;
                }
            }
        });
    }

    SearchLineEdit* getSearchEdit() const { return searchEdit; }

    void showPopup() override {
        if (view()) {
            view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
        NoWheelComboBox::showPopup();
        QWidget *popup = this->view() ? this->view()->parentWidget() : nullptr;
        if (popup) {
            popup->setStyleSheet(R"(
                QWidget {
                    background-color: #ffffff;
                    border: 1.5px solid #cbd5e1;
                    border-radius: 10px;
                }
                QScrollBar:vertical {
                    width: 0px;
                    height: 0px;
                    background: transparent;
                }
            )");
            if (!layoutConfigured) {
                if (auto *boxLay = qobject_cast<QBoxLayout*>(popup->layout())) {
                    boxLay->setContentsMargins(4, 4, 4, 6);
                    boxLay->setSpacing(2);
                    boxLay->insertWidget(0, searchEdit);
                    layoutConfigured = true;
                }
            }
        }
        if (searchEdit) {
            searchEdit->clear();
            filterItems("");
            QTimer::singleShot(20, searchEdit, [this]() {
                searchEdit->setFocus();
            });
        }
    }

    void hidePopup() override {
        if (searchEdit) {
            searchEdit->clear();
            filterItems("");
        }
        NoWheelComboBox::hidePopup();
    }

private:
    void filterItems(const QString &keyword) {
        QString kw = stripVietnameseAccents(keyword.trimmed().toLower());
        auto *lv = qobject_cast<QListView*>(this->view());
        if (!lv) return;
        auto *m = qobject_cast<QStandardItemModel*>(this->model());
        if (!m) return;

        lv->setRowHidden(0, true);

        for (int r = 1; r < m->rowCount(); ++r) {
            auto *item = m->item(r);
            if (!item) continue;
            if (kw.isEmpty()) {
                lv->setRowHidden(r, false);
            } else {
                QString textNorm = stripVietnameseAccents(item->text().toLower());
                QString dataNorm = stripVietnameseAccents(item->data(Qt::UserRole).toString().toLower());
                bool match = textNorm.contains(kw) || dataNorm.contains(kw);
                lv->setRowHidden(r, !match);
            }
        }
    }
};

} // namespace

bool AppointmentView::showBookingDialog(const QDate &defaultDate) {
    if (!mainWindow) return false;

    QDialog dialog(mainWindow);
    dialog.setWindowTitle("Đặt lịch khám bệnh");
    dialog.setFixedWidth(540);
    dialog.setStyleSheet(R"(
        QDialog { background-color: #ffffff; border-radius: 12px; }
        QComboBox, QDateTimeEdit {
            background-color: #f8fafc;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 13.5px;
            color: #0f172a;
            min-height: 22px;
        }
        QComboBox:focus, QDateTimeEdit:focus {
            border: 1.5px solid #0d9488;
            background: #ffffff;
        }
        QComboBox::drop-down { border: none; width: 28px; }
        QComboBox::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 12px; height: 12px; margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #0f172a;
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            outline: none;
            padding: 4px;
            font-size: 13.5px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 32px;
            padding-left: 8px;
            border-radius: 6px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #f0fdfa;
            color: #0d9488;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            width: 0px;
            height: 0px;
            background: transparent;
            border: none;
        }
        QDateTimeEdit { padding-right: 44px; }
        QDateTimeEdit::drop-down {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 42px;
            border-left: 1.5px solid #cbd5e1;
            border-top-right-radius: 7px;
            border-bottom-right-radius: 7px;
            background-color: #f1f5f9;
        }
        QDateTimeEdit::drop-down:hover { background-color: #0d9488; border-left-color: #0d9488; }
        QDateTimeEdit::drop-down:pressed { background-color: #0f766e; }
        QDateTimeEdit::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 14px; height: 14px;
        }
        QDateTimeEdit::down-arrow:hover { image: url(:/assets/textures/arrow_down_white.png); }
    )");

    auto *rootLayout = new QVBoxLayout(&dialog);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(14);

    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(14);
    auto *iconBadge = new QLabel("📅");
    iconBadge->setFixedSize(44, 44);
    iconBadge->setAlignment(Qt::AlignCenter);
    iconBadge->setStyleSheet("background: #f0fdfa; color: #0d9488; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 20px;");

    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(2);
    auto *dlgTitle = new QLabel("Tạo lịch khám bệnh mới");
    dlgTitle->setStyleSheet("font-size: 17px; font-weight: 750; color: #0f172a;");
    auto *dlgSub = new QLabel("Điền thông tin bệnh nhân, triệu chứng và bác sĩ phụ trách");
    dlgSub->setStyleSheet("font-size: 12.5px; color: #64748b; font-weight: 500;");
    titleBox->addWidget(dlgTitle);
    titleBox->addWidget(dlgSub);
    headerBox->addWidget(iconBadge);
    headerBox->addLayout(titleBox, 1);
    rootLayout->addLayout(headerBox);

    auto *divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #f1f5f9; max-height: 1px; border: none;");
    rootLayout->addWidget(divider);

    auto *form = new QFormLayout;
    form->setSpacing(11);
    form->setLabelAlignment(Qt::AlignLeft);

    auto makeLabel = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setStyleSheet("font-weight: 600; font-size: 13px; color: #334155;");
        return l;
    };

    // 1. Patient: "Chọn hồ sơ bệnh nhân", bỏ mã bệnh nhân ra, tích hợp ô tìm kiếm bệnh nhân ở đầu dropdown
    auto *patient = new SearchablePatientComboBox;
    patient->addItem("Chọn hồ sơ bệnh nhân", "");
    auto *patients = mainWindow->getPatients();
    if (patients) {
        auto *p = patients->getHeader()->next;
        while (p && p->next) {
            patient->addItem(UIHelper::qs(p->info.getfullName()), UIHelper::qs(p->info.getID()));
            p = p->next;
        }
    }
    form->addRow(makeLabel("Bệnh nhân:"), patient);

    // 2. Symptoms: 1 to 5 dynamic dropdowns, "Chọn triệu chứng", chỉ hiển thị tên triệu chứng
    NoWheelComboBox *symptomCombos[5];
    QWidget *symptomRowWidgets[5];
    auto *symptoms = mainWindow->getSymptoms();

    for (int i = 0; i < 5; ++i) {
        symptomCombos[i] = new NoWheelComboBox;
        symptomCombos[i]->addItem("Chọn triệu chứng", "");
        if (symptoms) {
            auto *s = symptoms->getHeader()->next;
            while (s && s->next) {
                symptomCombos[i]->addItem(UIHelper::qs(s->info.getSName()), UIHelper::qs(s->info.getS_id()));
                s = s->next;
            }
        }

        symptomRowWidgets[i] = new QWidget;
        auto *rowLay = new QHBoxLayout(symptomRowWidgets[i]);
        rowLay->setContentsMargins(0, 0, 0, 0);
        rowLay->addWidget(symptomCombos[i]);

        QString lblText = (i == 0) ? "Triệu chứng:" : QStringLiteral("Triệu chứng %1:").arg(i + 1);
        form->addRow(makeLabel(lblText), symptomRowWidgets[i]);

        if (i > 0) {
            symptomRowWidgets[i]->hide();
            if (auto *lblItem = form->labelForField(symptomRowWidgets[i])) {
                lblItem->hide();
            }
        }
    }

    // 3. Specialty: Luôn hiển thị sẵn từ đầu, hiển thị chuyên khoa cao nhất trong ô box đồng bộ
    auto *specialtyValue = new QLabel("—");
    specialtyValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    specialtyValue->setStyleSheet("background-color: #f8fafc; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 8px 12px; font-size: 13.5px; color: #0d9488; font-weight: 750; min-height: 22px;");
    form->addRow(makeLabel("Chuyên khoa:"), specialtyValue);

    // 4. Doctor: "Chọn bác sĩ phụ trách", bỏ mã bác sĩ ra
    auto *doctor = new NoWheelComboBox;
    doctor->addItem("Chọn bác sĩ phụ trách", "");
    form->addRow(makeLabel("Bác sĩ khám:"), doctor);

    // 5. Fee row: Hiển thị phí khám trực tiếp trong ô box đồng bộ
    auto *feeValue = new QLabel("—");
    feeValue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    feeValue->setStyleSheet("background-color: #f8fafc; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 8px 12px; font-size: 14.5px; color: #0d9488; font-weight: 800; min-height: 22px;");
    form->addRow(makeLabel("Phí khám:"), feeValue);

    // 5. Date & Time: Khóa thời gian trước lúc đặt lịch + calendar popup đẹp, sáng rõ
    QDateTime now = QDateTime::currentDateTime();
    QDate targetD = defaultDate.isValid() ? defaultDate : now.date().addDays(1);
    QDateTime initialDT(targetD, QTime(8, 0));
    if (initialDT <= now) {
        initialDT = now.addSecs(3600);
    }
    auto *dateEdit = new QDateTimeEdit(initialDT);
    dateEdit->installEventFilter(FormNoWheelFilter::instance());
    dateEdit->setDisplayFormat("dd/MM/yyyy   HH:mm");
    dateEdit->setCalendarPopup(true);
    dateEdit->setMinimumDateTime(now);

    if (auto *calPopup = dateEdit->calendarWidget()) {
        calPopup->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
        calPopup->setFirstDayOfWeek(Qt::Monday);
        calPopup->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        calPopup->setMinimumDate(now.date());
        calPopup->setFixedSize(360, 290);
        calPopup->setStyleSheet(R"(
            QCalendarWidget {
                background-color: #ffffff;
                border: 1.5px solid #cbd5e1;
                border-radius: 12px;
                font-family: 'Segoe UI', sans-serif;
            }
            QCalendarWidget QWidget#qt_calendar_navigationbar {
                background-color: #f8fafc;
                border-top-left-radius: 11px;
                border-top-right-radius: 11px;
                border-bottom: 1.5px solid #e2e8f0;
                min-height: 38px;
            }
            QCalendarWidget QToolButton {
                color: #0f172a;
                font-weight: 750;
                font-size: 13.5px;
                background: transparent;
                border: none;
                border-radius: 6px;
                padding: 4px 8px;
                margin: 2px;
            }
            QCalendarWidget QToolButton:hover {
                background-color: #f0fdfa;
                color: #0d9488;
            }
            QCalendarWidget QToolButton#qt_calendar_prevmonth {
                qproperty-icon: none;
                text: "‹";
                font-size: 18px;
                font-weight: bold;
            }
            QCalendarWidget QToolButton#qt_calendar_nextmonth {
                qproperty-icon: none;
                text: "›";
                font-size: 18px;
                font-weight: bold;
            }
            QCalendarWidget QMenu {
                background-color: #ffffff;
                color: #0f172a;
                border: 1px solid #cbd5e1;
                border-radius: 8px;
                padding: 4px;
            }
            QCalendarWidget QSpinBox {
                background: #ffffff;
                border: 1.5px solid #cbd5e1;
                border-radius: 6px;
                color: #0f172a;
                font-weight: 700;
                padding: 2px 6px;
            }
            QCalendarWidget QTableView {
                background-color: #ffffff;
                selection-background-color: #0d9488;
                selection-color: #ffffff;
                color: #1e293b;
                font-size: 12.5px;
                font-weight: 600;
                outline: none;
                border: none;
            }
            QCalendarWidget QTableView QHeaderView::section {
                background-color: #f1f5f9;
                color: #475569;
                font-weight: 750;
                font-size: 11px;
                border: none;
                padding: 4px 0px;
            }
            QCalendarWidget QTableView::item:hover {
                background-color: #f0fdfa;
                color: #0d9488;
                border-radius: 6px;
            }
        )");
    }
    form->addRow(makeLabel("Ngày giờ hẹn:"), dateEdit);
    rootLayout->addLayout(form);

    // Dynamic symptom handling & specialty score calculation
    auto updateSpecialtyAndDoctors = [this, &dialog, symptomCombos, form, specialtyValue, doctor, feeValue]() {
        // Collect currently selected symptom IDs
        QString selectedIds[5];
        for (int i = 0; i < 5; ++i) {
            selectedIds[i] = symptomCombos[i]->currentData().toString();
        }

        // Khóa cứng lại triệu chứng đã chọn: loại bỏ triệu chứng đã được chọn ở ô khác
        auto *symptomList = mainWindow->getSymptoms();
        if (symptomList) {
            for (int i = 0; i < 5; ++i) {
                QString myCurrent = selectedIds[i];
                symptomCombos[i]->blockSignals(true);
                symptomCombos[i]->clear();
                symptomCombos[i]->addItem("Chọn triệu chứng", "");

                auto *s = symptomList->getHeader()->next;
                while (s && s->next) {
                    QString sid = UIHelper::qs(s->info.getS_id());
                    bool chosenInOther = false;
                    for (int j = 0; j < 5; ++j) {
                        if (j != i && selectedIds[j] == sid) {
                            chosenInOther = true;
                            break;
                        }
                    }
                    if (!chosenInOther) {
                        symptomCombos[i]->addItem(UIHelper::qs(s->info.getSName()), sid);
                    }
                    s = s->next;
                }
                int idx = symptomCombos[i]->findData(myCurrent);
                if (idx >= 0) symptomCombos[i]->setCurrentIndex(idx);
                else symptomCombos[i]->setCurrentIndex(0);
                symptomCombos[i]->blockSignals(false);
            }
        }

        // 1. Manage visibility of symptom rows
        for (int i = 0; i < 4; ++i) {
            bool currentSelected = (symptomCombos[i]->currentIndex() > 0);
            if (currentSelected) {
                symptomCombos[i + 1]->parentWidget()->show();
                if (auto *lbl = form->labelForField(symptomCombos[i + 1]->parentWidget())) lbl->show();
            } else {
                for (int j = i + 1; j < 5; ++j) {
                    symptomCombos[j]->blockSignals(true);
                    symptomCombos[j]->setCurrentIndex(0);
                    symptomCombos[j]->blockSignals(false);
                    symptomCombos[j]->parentWidget()->hide();
                    if (auto *lbl = form->labelForField(symptomCombos[j]->parentWidget())) lbl->hide();
                }
                break;
            }
        }

        // 2. Calculate scores for each specialty
        QMap<QString, int> spScores;
        if (symptomList) {
            for (int i = 0; i < 5; ++i) {
                QString sid = symptomCombos[i]->currentData().toString();
                if (!sid.isEmpty()) {
                    auto *s = symptomList->getHeader()->next;
                    while (s && s->next) {
                        if (UIHelper::qs(s->info.getS_id()) == sid) {
                            QString spId = UIHelper::qs(s->info.getSp_id());
                            spScores[spId] += s->info.getSScore();
                            break;
                        }
                        s = s->next;
                    }
                }
            }
        }

        // 3. Find highest score specialty
        QString bestSpId;
        int maxScore = 0;
        for (auto it = spScores.begin(); it != spScores.end(); ++it) {
            if (it.value() > maxScore) {
                maxScore = it.value();
                bestSpId = it.key();
            }
        }

        // 4. Update specialty label (hiển thị chuyên khoa cao nhất, không ghi gợi ý, không ghi điểm)
        if (!bestSpId.isEmpty()) {
            specialtyValue->setText(mainWindow->specialty(bestSpId.toStdString()));
        } else {
            specialtyValue->setText("—");
        }

        // 5. Update doctor combo
        QString prevDocId = doctor->currentData().toString();
        doctor->clear();
        doctor->addItem("Chọn bác sĩ phụ trách", "");
        feeValue->setText("—");

        if (!bestSpId.isEmpty()) {
            auto *doctors = mainWindow->getDoctors();
            if (doctors) {
                auto *d = doctors->getHeader()->next;
                while (d && d->next) {
                    if (d->info.getState() && UIHelper::qs(d->info.getSpId()) == bestSpId) {
                        QString roomStr = UIHelper::formatRoom(UIHelper::qs(d->info.getRoomId()));
                        doctor->addItem(QStringLiteral("BS. %1 · %2 (%3)")
                            .arg(UIHelper::qs(d->info.getfullName()), mainWindow->specialty(d->info.getSpId()), roomStr),
                            UIHelper::qs(d->info.getID()));
                    }
                    d = d->next;
                }
            }
            int restoredIdx = doctor->findData(prevDocId);
            if (restoredIdx >= 0) {
                doctor->setCurrentIndex(restoredIdx);
            }
        }
        dialog.adjustSize();
    };

    for (int i = 0; i < 5; ++i) {
        connect(symptomCombos[i], qOverload<int>(&QComboBox::currentIndexChanged), &dialog, updateSpecialtyAndDoctors);
    }

    connect(doctor, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [this, doctor, feeValue] {
        Doctor *selected = mainWindow->findDoctor(UIHelper::ss(doctor->currentData().toString()));
        if (selected) {
            feeValue->setText(QLocale(QLocale::Vietnamese).toString(selected->getFee(), 'f', 0) + " ₫");
        } else {
            feeValue->setText("—");
        }
    });

    auto *btnBox = new QHBoxLayout;
    btnBox->setContentsMargins(0, 10, 0, 0);
    btnBox->setSpacing(10);
    btnBox->addStretch();

    auto *cancelBtn = new QPushButton("Hủy bỏ");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px 20px; font-size: 13px; font-weight: 600; color: #475569; } QPushButton:hover { background: #e2e8f0; color: #1e293b; }");
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *confirmBtn = new QPushButton("＋  Xác nhận đặt lịch");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet("QPushButton { background: #0d9488; border: none; border-radius: 8px; padding: 9px 22px; font-size: 13px; font-weight: 700; color: white; } QPushButton:hover { background: #0f766e; }");

    connect(confirmBtn, &QPushButton::clicked, &dialog, [this, &dialog, patient, symptomCombos, doctor, dateEdit] {
        if (patient->currentData().toString().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng chọn hồ sơ bệnh nhân.");
            return;
        }

        QStringList symList;
        for (int i = 0; i < 5; ++i) {
            QString sid = symptomCombos[i]->currentData().toString();
            if (!sid.isEmpty() && !symList.contains(sid)) {
                symList.append(sid);
            }
        }

        if (symList.isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng chọn ít nhất một triệu chứng khám.");
            return;
        }

        if (doctor->currentData().toString().isEmpty()) {
            QMessageBox::warning(&dialog, "Thiếu thông tin", "Vui lòng chọn bác sĩ phụ trách.");
            return;
        }

        if (dateEdit->dateTime() <= QDateTime::currentDateTime()) {
            QMessageBox::warning(&dialog, "Thời gian không hợp lệ", "Vui lòng chọn thời gian khám trong tương lai.");
            return;
        }

        if (mainWindow->isHoliday(dateEdit->date())) {
            QMessageBox::warning(&dialog, "Ngày nghỉ lễ", QStringLiteral("Phòng khám nghỉ lễ vào ngày %1 (%2). Vui lòng chọn ngày khác.").arg(dateEdit->date().toString("dd/MM/yyyy"), mainWindow->holidayNote(dateEdit->date())));
            return;
        }

        std::string pId = UIHelper::ss(patient->currentData().toString());
        std::string dId = UIHelper::ss(doctor->currentData().toString());
        std::string combinedSymIds = UIHelper::ss(symList.join(", "));

        Doctor *doc = mainWindow->findDoctor(dId);
        std::string specId = doc ? doc->getSpId() : "";

        auto *appointments = mainWindow->getAppointments();
        auto *symptomPatients = mainWindow->getSymptomPatients();
        std::string nextSpId = SymptomPatient::generateNextId(symptomPatients);

        if (symptomPatients) {
            symptomPatients->insert(SymptomPatient(nextSpId, combinedSymIds, pId, specId, Date::now(), 0));
        }
        if (appointments) {
            appointments->insert(Appointment(AppointmentController::generateNextId(appointments), nextSpId, dId, Date::now(), Date::loadFromString(UIHelper::ss(dateEdit->dateTime().toString("yyyy-MM-dd HH:mm"))), 0));
        }

        mainWindow->updatePatientSymptomStatus(pId);
        mainWindow->saveAll();
        QMessageBox::information(&dialog, "Thành công", "Lịch khám đã được ghi nhận và lưu thành công.");
        mainWindow->refreshPage(4);
        mainWindow->refreshPage(0);
        dialog.accept();
    });

    btnBox->addWidget(cancelBtn);
    btnBox->addWidget(confirmBtn);
    rootLayout->addLayout(btnBox);

    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });

    return (dialog.exec() == QDialog::Accepted);
}


namespace {

class StepperComboBox : public QComboBox {
public:
    QWidget *stepperBox = nullptr;

    explicit StepperComboBox(QWidget *parent = nullptr) : QComboBox(parent) {
        setView(new QListView(this));
        if (view()) {
            view()->setTextElideMode(Qt::ElideNone);
        }
    }

    void showPopup() override {
        QComboBox::showPopup();
        QWidget *popup = this->view() ? this->view()->parentWidget() : nullptr;
        if (popup && stepperBox) {
            int targetWidth = qMax(stepperBox->width(), 140);
            QPoint pos = stepperBox->mapToGlobal(QPoint(0, stepperBox->height() + 2));
            popup->setFixedWidth(targetWidth);
            popup->move(pos);
            QTimer::singleShot(0, popup, [popup, pos, targetWidth]() {
                popup->setFixedWidth(targetWidth);
                popup->move(pos);
            });
        }
    }

    void applyWheelRoll(int delta) {
        int m = currentData().toInt();
        if (delta > 0) {
            m = (m == 12) ? 1 : (m + 1);
        } else {
            m = (m == 1) ? 12 : (m - 1);
        }
        int idx = findData(m);
        if (idx != -1) setCurrentIndex(idx);
    }

    void wheelEvent(QWheelEvent *event) override {
        int delta = event->angleDelta().y();
        if (delta != 0) {
            applyWheelRoll(delta);
            event->accept();
        } else {
            QComboBox::wheelEvent(event);
        }
    }
};

class YearSpinBox : public QSpinBox {
public:
    qint64 lastWheelTime = 0;

    explicit YearSpinBox(QWidget *parent = nullptr) : QSpinBox(parent) {
        setRange(0, 999999);
        setButtonSymbols(QAbstractSpinBox::NoButtons);
        setAlignment(Qt::AlignCenter);
        setPrefix("Năm ");
        setFocusPolicy(Qt::StrongFocus);
        setFixedWidth(98);
        setStyleSheet(R"(
            QSpinBox {
                background: transparent;
                border: none;
                font-weight: 700;
                font-size: 13.5px;
                color: #0f172a;
                padding: 2px 4px;
            }
            QSpinBox::up-button, QSpinBox::down-button {
                width: 0px;
                height: 0px;
                border: none;
            }
        )");
    }

    void applyWheelRoll(int delta, Qt::KeyboardModifiers mods) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 dt = now - lastWheelTime;
        lastWheelTime = now;

        int step = 1;
        if (dt < 35) {
            step = 25;  // Lướt siêu tốc: 25 năm / nấc
        } else if (dt < 70) {
            step = 10;  // Lướt nhanh: 10 năm / nấc
        } else if (dt < 130) {
            step = 3;   // Lướt vừa: 3 năm / nấc
        } else {
            step = 1;   // Lăn chậm: 1 năm / nấc
        }

        if (mods & Qt::ShiftModifier) {
            step = qMax(step, 10);
        }
        if (mods & Qt::ControlModifier) {
            step = qMax(step, 100);
        }

        if (delta > 0) {
            setValue(value() + step);
        } else {
            setValue(qMax(0, value() - step));
        }
    }

    void wheelEvent(QWheelEvent *event) override {
        int delta = event->angleDelta().y();
        if (delta != 0) {
            applyWheelRoll(delta, event->modifiers());
            event->accept();
        } else {
            QSpinBox::wheelEvent(event);
        }
    }
};

class YearWheelFilter : public QObject {
public:
    YearSpinBox *target = nullptr;
    YearWheelFilter(YearSpinBox *t, QObject *parent = nullptr) : QObject(parent), target(t) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Wheel && target) {
            auto *we = static_cast<QWheelEvent*>(event);
            target->applyWheelRoll(we->angleDelta().y(), we->modifiers());
            return true;
        }
        return QObject::eventFilter(obj, event);
    }
};

class MonthWheelFilter : public QObject {
public:
    StepperComboBox *target = nullptr;
    MonthWheelFilter(StepperComboBox *t, QObject *parent = nullptr) : QObject(parent), target(t) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Wheel && target) {
            auto *we = static_cast<QWheelEvent*>(event);
            target->applyWheelRoll(we->angleDelta().y());
            return true;
        }
        return QObject::eventFilter(obj, event);
    }
};

class AppointmentRowClickFilter : public QObject {
public:
    Appointment *appointment = nullptr;
    AppointmentView *view = nullptr;
    AppointmentRowClickFilter(Appointment *a, AppointmentView *v, QObject *parent = nullptr)
        : QObject(parent), appointment(a), view(v) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            auto *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && view && appointment) {
                view->showAppointmentDetailDialog(appointment);
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

} // namespace

QWidget *AppointmentView::createDashboardWidget(QWidget *parent) {
    if (!mainWindow) return new QWidget(parent);

    auto *w = new QWidget(parent);
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(28, 20, 28, 20);
    layout->setSpacing(16);

    // Tiêu đề & Lời chào theo thời gian
    auto *h = new QVBoxLayout;
    h->setSpacing(4);
    int hour = QTime::currentTime().hour();
    QString timeGreeting = (hour >= 5 && hour < 12) ? "Chào buổi sáng" : (hour >= 12 && hour < 18) ? "Chào buổi chiều" : "Chào buổi tối";
    auto *mgr = mainWindow->getCurrentManager();
    QString managerName = mgr ? UIHelper::qs(mgr->getfullName()) : "";
    QString gTitle = managerName.isEmpty() ? QStringLiteral("%1 👋").arg(timeGreeting)
                                          : QStringLiteral("%1, %2 👋").arg(timeGreeting, managerName);
    QString gSub = QStringLiteral("Hôm nay là %1 · Chúc bạn một ngày làm việc hiệu quả tại Phòng khám An Tâm.")
                      .arg(QDate::currentDate().toString("dd/MM/yyyy"));

    auto *greetingTitle = new QLabel(gTitle);
    greetingTitle->setObjectName("pageTitle");
    auto *greetingSub = new QLabel(gSub);
    greetingSub->setObjectName("pageSubtitle");
    h->addWidget(greetingTitle);
    h->addWidget(greetingSub);
    layout->addLayout(h);

    // 4 Thẻ chỉ số nổi bật
    auto *metrics = new QGridLayout;
    metrics->setSpacing(16);
    const QString labels[] = {"BỆNH NHÂN", "BÁC SĨ", "LỊCH KHÁM", "CHƯA KHÁM"};
    const QString colors[] = {"#0d9488", "#0284c7", "#6366f1", "#ea580c"};
    int counts[4] = {
        StatisticController::patientCount(mainWindow->getPatients()),
        StatisticController::doctorCount(mainWindow->getDoctors()),
        StatisticController::appointmentCount(mainWindow->getAppointments()),
        StatisticController::pendingAppointmentCount(mainWindow->getAppointments())
    };

    for (int i = 0; i < 4; ++i) {
        auto *c = UIHelper::panel();
        c->setStyleSheet(QStringLiteral(
            "QFrame#panel { "
            "    background: #ffffff; "
            "    border: 1px solid #e2e8f0; "
            "    border-left: 5px solid %1; "
            "    border-radius: 12px; "
            "}").arg(colors[i]));

        auto *v = new QVBoxLayout(c);
        v->setContentsMargins(20, 16, 20, 16);
        v->setSpacing(6);

        auto *val = new QLabel(QString::number(counts[i]));
        val->setStyleSheet(QStringLiteral("font-size: 32px; font-weight: 800; color: %1;").arg(colors[i]));

        auto *l = new QLabel(labels[i]);
        l->setStyleSheet("font-size: 11px; font-weight: 700; color: #475569; letter-spacing: 0.8px;");

        v->addWidget(val);
        v->addWidget(l);
        metrics->addWidget(c, 0, i);
    }
    layout->addLayout(metrics);

    // Banner đặt lịch nhanh
    auto *banner = UIHelper::panel();
    banner->setStyleSheet(R"(
        QFrame#panel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f0fdfa, stop:1 #e6fffa);
            border: 1px solid #99f6e4;
            border-radius: 14px;
        }
    )");
    auto *bLayout = new QHBoxLayout(banner);
    bLayout->setContentsMargins(24, 18, 24, 18);

    auto *bText = new QVBoxLayout;
    bText->setSpacing(4);
    auto *bTitle = new QLabel("Chăm sóc chu đáo, bắt đầu từ một lịch hẹn tốt.");
    bTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #134e4a;");
    auto *bSub = new QLabel("Tạo lịch khám mới và chọn bác sĩ phù hợp với triệu chứng của bệnh nhân.");
    bSub->setStyleSheet("font-size: 12.5px; color: #0f766e;");
    bText->addWidget(bTitle);
    bText->addWidget(bSub);
    bLayout->addLayout(bText, 1);

    auto *bookQuick = UIHelper::button("Đặt lịch khám  →", "primary");
    connect(bookQuick, &QPushButton::clicked, w, [this] {
        if (mainWindow) mainWindow->setPage(1);
    });
    bLayout->addWidget(bookQuick);
    layout->addWidget(banner);

    // Bảng danh sách lịch hẹn sắp tới
    auto *card = UIHelper::panel();
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 18);
    cardLayout->setSpacing(14);

    auto *tHeader = new QHBoxLayout;
    auto *tTitle = new QLabel("Lịch hẹn sắp tới");
    tTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #0f172a;");
    auto *tSub = new QLabel("Lịch khám trong ngày hôm nay và ngày mai");
    tSub->setStyleSheet("font-size: 12px; color: #64748b; margin-left: 8px;");
    tHeader->addWidget(tTitle);
    tHeader->addWidget(tSub);
    tHeader->addStretch();
    cardLayout->addLayout(tHeader);

    auto *table = UIHelper::table({"BỆNH NHÂN", "BÁC SĨ", "THỜI GIAN", "TRẠNG THÁI"});
    table->setShowGrid(false);
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
        }
        QTableWidget::item:hover { background: #f0fdfa; }
    )");
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    table->setColumnWidth(2, 145);
    table->setColumnWidth(3, 145);
    table->verticalHeader()->setDefaultSectionSize(54);
    if (table->horizontalHeaderItem(0)) table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(1)) table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (table->horizontalHeaderItem(2)) table->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    if (table->horizontalHeaderItem(3)) table->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    cardLayout->addWidget(table, 1);

    // Thanh phân trang
    auto *pagination = new QHBoxLayout;
    pagination->addStretch();
    auto *prevBtn = UIHelper::button("‹  Trang trước", "secondary");
    auto *nextBtn = UIHelper::button("Trang sau  ›", "secondary");
    auto *pageStatus = new QLabel("Trang 1 / 1");
    pageStatus->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #0f172a; padding: 0 10px;");
    pagination->addWidget(prevBtn);
    pagination->addWidget(pageStatus);
    pagination->addWidget(nextBtn);
    cardLayout->addLayout(pagination);

    layout->addWidget(card, 1);

    // Trạng thái phân trang
    struct DashState {
        int pageNumber = 0;
        enum { PAGE_SIZE = 8 };
    };
    auto *state = new DashState;

    auto renderRows = [=, this]() {
        QDate today = QDate::currentDate();
        QList<Appointment*> all = AppointmentController::getUpcomingAppointments(mainWindow->getAppointments(), today);
        table->setRowCount(0);
        int total = all.size();
        int totalPages = total == 0 ? 1 : (total + DashState::PAGE_SIZE - 1) / DashState::PAGE_SIZE;
        state->pageNumber = qBound(0, state->pageNumber, totalPages - 1);
        pageStatus->setText(QStringLiteral("Trang %1 / %2").arg(state->pageNumber + 1).arg(totalPages));
        prevBtn->setEnabled(state->pageNumber > 0);
        nextBtn->setEnabled(state->pageNumber < totalPages - 1);

        int start = state->pageNumber * DashState::PAGE_SIZE;
        int end = qMin(start + DashState::PAGE_SIZE, total);

        for (int i = start; i < end; ++i) {
            auto *a = all[i];
            int r = table->rowCount();
            table->insertRow(r);
            table->setRowHeight(r, 54);

            Patient *p = mainWindow->findPatientForAppointment(*a);
            Doctor *d = mainWindow->findDoctor(a->getDoctorId());
            Date ad = a->getDateGet();
            QDate appDate(ad.getYear(), ad.getMonth(), ad.getDay());

            auto *pw = new QWidget;
            auto *pwl = new QVBoxLayout(pw);
            pwl->setContentsMargins(14, 4, 14, 4);
            pwl->setSpacing(2);
            auto *nl = new QLabel(p ? UIHelper::qs(p->getfullName()) : "—");
            nl->setStyleSheet("font-weight: 700; font-size: 13px; color: #0f172a;");
            nl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            QString pSub = p ? QStringLiteral("%1 tuổi (%2)").arg(p->getAge()).arg(p->getGender() ? "Nam" : "Nữ") : "";
            auto *sl = new QLabel(pSub);
            sl->setStyleSheet("font-size: 11.5px; color: #64748b;");
            sl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            pwl->addWidget(nl);
            pwl->addWidget(sl);
            table->setCellWidget(r, 0, pw);

            auto *dw = new QWidget;
            auto *dwl = new QVBoxLayout(dw);
            dwl->setContentsMargins(14, 4, 14, 4);
            dwl->setSpacing(2);
            auto *dnl = new QLabel(d ? QStringLiteral("BS. %1").arg(UIHelper::qs(d->getfullName())) : UIHelper::qs(a->getDoctorId()));
            dnl->setStyleSheet("font-weight: 700; font-size: 13px; color: #0f172a;");
            dnl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            QString dSub = d ? QStringLiteral("%1 · %2").arg(mainWindow->specialty(d->getSpId()), UIHelper::formatRoom(UIHelper::qs(d->getRoomId()))) : "";
            auto *docSub = new QLabel(dSub);
            docSub->setStyleSheet("font-size: 11.5px; color: #0284c7; font-weight: 600;");
            docSub->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            dwl->addWidget(dnl);
            dwl->addWidget(docSub);
            table->setCellWidget(r, 1, dw);

            auto *tw = new QWidget;
            auto *twl = new QVBoxLayout(tw);
            twl->setContentsMargins(6, 4, 6, 4);
            twl->setSpacing(2);
            auto *hourLabel = new QLabel(QString::asprintf("%02d:%02d", ad.getHour(), ad.getMinute()));
            hourLabel->setStyleSheet("font-weight: 700; font-size: 13px; color: #0f172a;");
            hourLabel->setAlignment(Qt::AlignCenter);

            QString dayText;
            if (appDate == today) {
                dayText = QStringLiteral("Hôm nay, %1/%2").arg(ad.getDay()).arg(ad.getMonth());
            } else if (appDate == today.addDays(1)) {
                dayText = QStringLiteral("Ngày mai, %1/%2").arg(ad.getDay()).arg(ad.getMonth());
            } else {
                dayText = QString::asprintf("%02d/%02d/%04d", ad.getDay(), ad.getMonth(), ad.getYear());
            }
            auto *dayLabel = new QLabel(dayText);
            dayLabel->setStyleSheet("font-size: 11.5px; color: #64748b;");
            dayLabel->setAlignment(Qt::AlignCenter);
            twl->addWidget(hourLabel);
            twl->addWidget(dayLabel);
            table->setCellWidget(r, 2, tw);

            int st = a->getState();
            if (st == 0) {
                Date adg = a->getDateGet();
                QDateTime now = QDateTime::currentDateTime();
                QDateTime apptDt(QDate(adg.getYear(), adg.getMonth(), adg.getDay()), QTime(adg.getHour(), adg.getMinute()));
                if (apptDt < now) {
                    st = -1;
                }
            }
            QString badgeText = "● Chưa khám";
            QString badgeStyle = "QLabel { background: #fffbeb; color: #d97706; border: 1px solid #fde68a; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px; }";
            if (st == 1) {
                badgeText = "● Đã khám";
                badgeStyle = "QLabel { background: #ecfdf5; color: #059669; border: 1px solid #a7f3d0; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px; }";
            } else if (st == -1) {
                badgeText = "● Trễ hẹn";
                badgeStyle = "QLabel { background: #fff7ed; color: #ea580c; border: 1px solid #ffedd5; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px; }";
            }
            auto *badge = new QLabel(badgeText);
            badge->setStyleSheet(badgeStyle);
            badge->setAlignment(Qt::AlignCenter);
            badge->setFixedHeight(26);

            auto *bb = new QWidget;
            auto *bbl = new QHBoxLayout(bb);
            bbl->setContentsMargins(6, 0, 6, 0);
            bbl->setAlignment(Qt::AlignCenter);
            bbl->addWidget(badge);
            table->setCellWidget(r, 3, bb);

            auto makeRowClickable = [&](QWidget *widget, Appointment *appt) {
                widget->setCursor(Qt::PointingHandCursor);
                widget->installEventFilter(new AppointmentRowClickFilter(appt, this, widget));
                const auto children = widget->findChildren<QWidget*>();
                for (auto *c : children) {
                    c->setCursor(Qt::PointingHandCursor);
                    c->installEventFilter(new AppointmentRowClickFilter(appt, this, c));
                }
            };
            makeRowClickable(pw, a);
            makeRowClickable(dw, a);
            makeRowClickable(tw, a);
            makeRowClickable(bb, a);
        }
    };

    connect(table, &QTableWidget::cellClicked, w, [=, this](int r, int) {
        QDate today = QDate::currentDate();
        QList<Appointment*> all = AppointmentController::getUpcomingAppointments(mainWindow->getAppointments(), today);
        int idx = state->pageNumber * DashState::PAGE_SIZE + r;
        if (idx >= 0 && idx < static_cast<int>(all.size())) {
            showAppointmentDetailDialog(all[idx]);
            renderRows();
        }
    });

    connect(prevBtn, &QPushButton::clicked, w, [=]() {
        if (state->pageNumber > 0) {
            state->pageNumber--;
            renderRows();
        }
    });
    connect(nextBtn, &QPushButton::clicked, w, [=]() {
        state->pageNumber++;
        renderRows();
    });

    auto *refreshTimer = new QTimer(w);
    connect(refreshTimer, &QTimer::timeout, w, renderRows);
    refreshTimer->start(30000);

    renderRows();
    return w;
}

QWidget *AppointmentView::createBookingWidget(QWidget *parent) {
    auto *scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { width: 8px; background: transparent; } QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 4px; }");

    auto *w = new QWidget;
    w->setStyleSheet("background: transparent;");
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(24, 14, 24, 14);
    layout->setSpacing(10);

    // Header layout
    auto *headerLayout = new QHBoxLayout;
    auto *headerTextLayout = new QVBoxLayout;
    headerTextLayout->setSpacing(2);
    auto *heading = new QLabel("Lịch khám bệnh");
    heading->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Theo dõi lịch khám của bệnh nhân theo từng ngày và quản lý đặt lịch hẹn.");
    subtitle->setObjectName("pageSubtitle");
    headerTextLayout->addWidget(heading);
    headerTextLayout->addWidget(subtitle);

    auto *holidayBtn = new QPushButton("🗓   Quản lý lịch nghỉ");
    holidayBtn->setCursor(Qt::PointingHandCursor);
    holidayBtn->setStyleSheet("QPushButton { background-color: #ffffff; color: #0f766e; border: 1.5px solid #0d9488; border-radius: 9px; padding: 8px 18px; font-size: 13px; font-weight: 700; } QPushButton:hover { background-color: #f0fdfa; border-color: #0f766e; }");

    auto *bookBtn = UIHelper::button("＋   Đặt lịch", "primary");
    bookBtn->setStyleSheet(R"(
        QPushButton#primary {
            background-color: #0d9488;
            color: white;
            border: none;
            border-radius: 9px;
            padding: 9px 20px;
            font-size: 13.5px;
            font-weight: 700;
        }
        QPushButton#primary:hover {
            background-color: #0f766e;
        }
        QPushButton#primary:disabled {
            background-color: #cbd5e1;
            color: #94a3b8;
        }
    )");

    headerLayout->addLayout(headerTextLayout, 1);
    headerLayout->addWidget(holidayBtn, 0, Qt::AlignVCenter);
    headerLayout->addWidget(bookBtn, 0, Qt::AlignVCenter);
    layout->addLayout(headerLayout);

    // Bảng lịch tháng
    auto *calCard = UIHelper::panel();
    calCard->setFixedHeight(290);
    auto *calCardLayout = new QVBoxLayout(calCard);
    calCardLayout->setContentsMargins(18, 12, 18, 12);
    calCardLayout->setSpacing(8);

    auto *calTopBar = new QHBoxLayout;
    calTopBar->setSpacing(12);

    auto makeStepperGroup = []() {
        auto *g = new QFrame;
        g->setObjectName("stepperGroup");
        g->setFixedHeight(34);
        g->setStyleSheet("QFrame#stepperGroup { background: #ffffff; border: 1.5px solid #cbd5e1; border-radius: 8px; } QFrame#stepperGroup:hover { border-color: #94a3b8; }");
        return g;
    };

    auto makeStepperBtn = [](const QString &text) {
        auto *b = new QPushButton(text);
        b->setFixedSize(26, 26);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(R"(
            QPushButton {
                background: transparent;
                border: none;
                border-radius: 6px;
                font-weight: 800;
                font-size: 15px;
                color: #475569;
            }
            QPushButton:hover {
                background: #f1f5f9;
                color: #0d9488;
            }
            QPushButton:pressed {
                background: #e2e8f0;
            }
        )");
        return b;
    };

    // Chọn tháng
    auto *monthGroup = makeStepperGroup();
    auto *monthGroupLayout = new QHBoxLayout(monthGroup);
    monthGroupLayout->setContentsMargins(4, 2, 4, 2);
    monthGroupLayout->setSpacing(2);
    auto *prevMonthBtn = makeStepperBtn("‹");
    auto *monthCombo = new StepperComboBox(monthGroup);
    monthCombo->stepperBox = monthGroup;
    monthCombo->setFixedWidth(92);
    monthCombo->setMaxVisibleItems(12);
    for (int m = 1; m <= 12; ++m) {
        monthCombo->addItem(QStringLiteral("Tháng %1").arg(m), m);
    }
    monthCombo->setStyleSheet(R"(
        QComboBox {
            combobox-popup: 0;
            background: transparent;
            border: none;
            padding: 2px 4px;
            font-weight: 700;
            font-size: 13.5px;
            color: #0f172a;
        }
        QComboBox::drop-down {
            border: none;
            width: 0px;
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
            font-weight: 600;
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
            width: 6px;
            background: transparent;
            margin: 4px 2px 4px 0px;
        }
        QScrollBar::handle:vertical {
            background: #cbd5e1;
            border-radius: 3px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #94a3b8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
    auto *nextMonthBtn = makeStepperBtn("›");
    monthGroupLayout->addWidget(prevMonthBtn);
    monthGroupLayout->addWidget(monthCombo);
    monthGroupLayout->addWidget(nextMonthBtn);

    // Chọn năm (Không hiển thị danh sách thả xuống, lăn chuột mượt mà về 0 hoặc vô cùng, đoạn lăn bên phải để trống)
    auto *yearGroup = makeStepperGroup();
    auto *yearGroupLayout = new QHBoxLayout(yearGroup);
    yearGroupLayout->setContentsMargins(4, 2, 4, 2);
    yearGroupLayout->setSpacing(2);
    auto *prevYearBtn = makeStepperBtn("‹");
    prevYearBtn->setAutoRepeat(true);
    prevYearBtn->setAutoRepeatDelay(250);
    prevYearBtn->setAutoRepeatInterval(50);

    auto *yearSpin = new YearSpinBox(yearGroup);

    auto *nextYearBtn = makeStepperBtn("›");
    nextYearBtn->setAutoRepeat(true);
    nextYearBtn->setAutoRepeatDelay(250);
    nextYearBtn->setAutoRepeatInterval(50);

    yearGroupLayout->addWidget(prevYearBtn);
    yearGroupLayout->addWidget(yearSpin);
    yearGroupLayout->addWidget(nextYearBtn);

    // Bắt sự kiện lăn chuột trên toàn bộ khung chứa năm & tháng
    yearGroup->installEventFilter(new YearWheelFilter(yearSpin, yearGroup));
    prevYearBtn->installEventFilter(new YearWheelFilter(yearSpin, prevYearBtn));
    nextYearBtn->installEventFilter(new YearWheelFilter(yearSpin, nextYearBtn));

    monthGroup->installEventFilter(new MonthWheelFilter(monthCombo, monthGroup));
    prevMonthBtn->installEventFilter(new MonthWheelFilter(monthCombo, prevMonthBtn));
    nextMonthBtn->installEventFilter(new MonthWheelFilter(monthCombo, nextMonthBtn));

    auto setComboYear = [yearSpin](int y) {
        yearSpin->setValue(qMax(0, y));
    };

    auto *todayBtn = new QPushButton("Hôm nay");
    todayBtn->setCursor(Qt::PointingHandCursor);
    todayBtn->setFixedHeight(34);
    todayBtn->setStyleSheet("QPushButton { background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 8px; color: #0f766e; font-weight: 700; font-size: 12.5px; padding: 4px 14px; } QPushButton:hover { background: #0d9488; color: white; border-color: #0d9488; }");

    calTopBar->addWidget(monthGroup);
    calTopBar->addWidget(yearGroup);
    calTopBar->addWidget(todayBtn);
    calTopBar->addStretch();
    calCardLayout->addLayout(calTopBar);

    auto *cal = new QCalendarWidget;
    cal->setGridVisible(false);
    cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    cal->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    cal->setNavigationBarVisible(false);
    cal->setFirstDayOfWeek(Qt::Monday);
    cal->setDateRange(QDate(1, 1, 1), QDate(9999, 12, 31));

    // Bỏ màu đỏ của Thứ 7 và Chủ Nhật trên tiêu đề
    QTextCharFormat weekdayHeaderFmt;
    weekdayHeaderFmt.setForeground(QColor("#475569"));
    weekdayHeaderFmt.setFontWeight(QFont::DemiBold);
    for (int dow = 1; dow <= 7; ++dow) {
        cal->setWeekdayTextFormat(static_cast<Qt::DayOfWeek>(dow), weekdayHeaderFmt);
    }

    cal->setStyleSheet(R"(
        QCalendarWidget QWidget#qt_calendar_calendarview {
            background-color: #ffffff;
            selection-background-color: #0d9488;
            selection-color: white;
            font-size: 13px;
            font-weight: 500;
            outline: none;
        }
        QCalendarWidget QTableView {
            alternate-background-color: #ffffff;
            border: none;
        }
        QCalendarWidget QTableView QHeaderView::section {
            background-color: #f8fafc;
            color: #475569;
            font-weight: 700;
            font-size: 11.5px;
            border: none;
            border-bottom: 1.5px solid #e2e8f0;
            padding: 4px 0px;
        }
    )");
    calCardLayout->addWidget(cal);
    layout->addWidget(calCard);

    // Phần dưới: Chi tiết danh sách bệnh nhân
    auto *dayCard = UIHelper::panel();
    dayCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    dayCard->setMinimumHeight(240);
    dayCard->setStyleSheet("QFrame#panel { background: #ffffff; border: 1.5px solid #cbd5e1; border-radius: 12px; }");
    auto *dayCardLayout = new QVBoxLayout(dayCard);
    dayCardLayout->setContentsMargins(18, 14, 18, 14);
    dayCardLayout->setSpacing(8);

    auto *dayHeaderLayout = new QHBoxLayout;
    auto *dayTitleCol = new QVBoxLayout;
    dayTitleCol->setSpacing(2);
    auto *dayTitle = new QLabel;
    dayTitle->setStyleSheet("font-size: 17px; font-weight: 750; color: #102f43;");
    auto *daySub = new QLabel("Danh sách bệnh nhân đăng ký khám trong ngày đã chọn");
    daySub->setStyleSheet("font-size: 12.5px; color: #64748b; padding-left: 2px;");
    dayTitleCol->addWidget(dayTitle);
    dayTitleCol->addWidget(daySub);

    auto *dayCountBadge = new QLabel;
    dayCountBadge->setStyleSheet("background: #f0fdf4; color: #15803d; border: 1px solid #bbf7d0; font-size: 12px; font-weight: 700; border-radius: 12px; padding: 3px 12px;");
    dayHeaderLayout->addLayout(dayTitleCol, 1);
    dayHeaderLayout->addWidget(dayCountBadge, 0, Qt::AlignVCenter);
    dayCardLayout->addLayout(dayHeaderLayout);

    auto *t = UIHelper::table({"BỆNH NHÂN", "BÁC SĨ", "THỜI GIAN", "TRẠNG THÁI"});
    t->setObjectName("dayPatientTable");
    t->setShowGrid(false);
    t->setStyleSheet(R"(
        #dayPatientTable {
            background-color: transparent;
            border: none;
            gridline-color: transparent;
        }
        #dayPatientTable QHeaderView {
            border: none;
            background: transparent;
        }
        #dayPatientTable QHeaderView::section {
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
        #dayPatientTable::item {
            background: #ffffff;
            border: none;
            border-bottom: 1px solid #f1f5f9;
            border-left: none;
            border-right: none;
        }
        #dayPatientTable::item:hover { background: #f0fdfa; }
    )");
    t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    t->setColumnWidth(2, 145);
    t->setColumnWidth(3, 145);
    t->verticalHeader()->setDefaultSectionSize(54);
    if (t->horizontalHeaderItem(0)) t->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (t->horizontalHeaderItem(1)) t->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (t->horizontalHeaderItem(2)) t->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    if (t->horizontalHeaderItem(3)) t->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignCenter);
    dayCardLayout->addWidget(t, 1);

    auto *emptyWidget = new QWidget;
    emptyWidget->setObjectName("emptyCard");
    emptyWidget->setStyleSheet("#emptyCard { background-color: #f8fafc; border: 1.5px dashed #cbd5e1; border-radius: 10px; }");
    auto *emptyLayout = new QVBoxLayout(emptyWidget);
    emptyLayout->setContentsMargins(20, 30, 20, 30);
    emptyLayout->setAlignment(Qt::AlignCenter);
    emptyLayout->setSpacing(10);
    auto *emptyIcon = new QLabel("📋");
    emptyIcon->setStyleSheet("font-size: 38px; color: #94a3b8;");
    emptyIcon->setAlignment(Qt::AlignCenter);
    auto *emptyTitle = new QLabel("Chưa có lịch khám nào trong ngày này");
    emptyTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #475569;");
    emptyTitle->setAlignment(Qt::AlignCenter);
    auto *emptySub = new QLabel("Bấm nút 'Đặt lịch' ở góc trên để tạo lịch hẹn cho bệnh nhân.");
    emptySub->setStyleSheet("font-size: 12.5px; color: #94a3b8;");
    emptySub->setAlignment(Qt::AlignCenter);
    auto *quickBookBtn = new QPushButton("＋  Đặt lịch khám cho ngày này");
    quickBookBtn->setCursor(Qt::PointingHandCursor);
    quickBookBtn->setStyleSheet("QPushButton { background-color: #0d9488; color: white; border: none; border-radius: 8px; padding: 9px 20px; font-weight: 700; font-size: 13px; } QPushButton:hover { background-color: #0f766e; }");
    emptyLayout->addWidget(emptyIcon);
    emptyLayout->addWidget(emptyTitle);
    emptyLayout->addWidget(emptySub);
    emptyLayout->addWidget(quickBookBtn, 0, Qt::AlignCenter);
    dayCardLayout->addWidget(emptyWidget, 1);

    // Thanh phân trang
    auto *pagerRow = new QHBoxLayout;
    pagerRow->addStretch();
    auto *prevBtn = UIHelper::button("‹  Trang trước", "secondary");
    auto *nextBtn = UIHelper::button("Trang sau  ›", "secondary");
    auto *pageStatus = new QLabel("Trang 1 / 1");
    pageStatus->setStyleSheet("font-size: 13px; font-weight: 750; color: #0f172a; padding: 0 12px;");
    pagerRow->addWidget(prevBtn);
    pagerRow->addWidget(pageStatus);
    pagerRow->addWidget(nextBtn);
    dayCardLayout->addLayout(pagerRow);

    layout->addWidget(dayCard, 1);
    scroll->setWidget(w);

    // Trạng thái cục bộ
    struct BookingPageState {
        QDate selectedDate;
        QList<Appointment*> appointments;
        int pageNumber = 0;
        enum { PAGE_SIZE = 8 };
    };
    auto *state = new BookingPageState;
    state->selectedDate = QDate::currentDate();

    auto reloadDayView = [=, this]() {
        state->appointments = getAppointmentsForDate(state->selectedDate);
        int total = state->appointments.size();

        static const char *weekdayNames[] = {"", "Thứ Hai", "Thứ Ba", "Thứ Tư", "Thứ Năm", "Thứ Sáu", "Thứ Bảy", "Chủ Nhật"};
        int dow = state->selectedDate.dayOfWeek();
        QString dayName = (dow >= 1 && dow <= 7) ? weekdayNames[dow] : "";

        bool isHol = mainWindow->isHoliday(state->selectedDate);
        bool isPast = (state->selectedDate < QDate::currentDate());

        bookBtn->setEnabled(!isPast && !isHol);
        if (isPast || isHol) {
            bookBtn->setCursor(Qt::ForbiddenCursor);
        } else {
            bookBtn->setCursor(Qt::PointingHandCursor);
        }

        if (isHol) {
            QString holNote = mainWindow->holidayNote(state->selectedDate);
            dayTitle->setText(QStringLiteral("Lịch khám %1, ngày %2 - Nghỉ lễ: %3").arg(dayName, state->selectedDate.toString("dd/MM/yyyy"), holNote));
            dayCountBadge->setText(QStringLiteral("🗓 Nghỉ lễ: %1").arg(holNote));
            dayCountBadge->setStyleSheet("background: #fee2e2; color: #b91c1c; border: 1px solid #fca5a5; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;");
        } else {
            dayTitle->setText(QStringLiteral("Lịch khám %1, ngày %2").arg(dayName, state->selectedDate.toString("dd/MM/yyyy")));
            dayCountBadge->setText(QStringLiteral("%1 bệnh nhân").arg(total));
            dayCountBadge->setStyleSheet("background: #f1f5f9; color: #475569; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;");
        }

        if (total == 0) {
            t->hide();
            pagerRow->parentWidget(); // keep
            prevBtn->hide();
            nextBtn->hide();
            pageStatus->hide();
            emptyWidget->show();
            if (isHol) {
                emptyIcon->setText("🗓");
                emptyTitle->setText(QStringLiteral("Phòng khám nghỉ lễ vào ngày này (%1)").arg(mainWindow->holidayNote(state->selectedDate)));
                emptySub->setText("Phòng khám đóng cửa trong các ngày nghỉ lễ. Vui lòng chọn ngày làm việc khác.");
                quickBookBtn->hide();
            } else if (isPast) {
                emptyIcon->setText("⏳");
                emptyTitle->setText("Không thể đặt lịch cho ngày trước thời gian hiện tại");
                emptySub->setText("Thời gian đã qua, phòng khám không tiếp nhận đặt lịch mới cho ngày này.");
                quickBookBtn->hide();
            } else {
                emptyIcon->setText("📋");
                emptyTitle->setText("Chưa có lịch khám nào trong ngày này");
                emptySub->setText("Bấm nút 'Đặt lịch' ở góc trên để tạo lịch hẹn cho bệnh nhân.");
                quickBookBtn->show();
            }
            return;
        }

        emptyWidget->hide();
        t->show();
        prevBtn->show();
        nextBtn->show();
        pageStatus->show();

        int totalPages = (total + BookingPageState::PAGE_SIZE - 1) / BookingPageState::PAGE_SIZE;
        state->pageNumber = qBound(0, state->pageNumber, totalPages - 1);
        pageStatus->setText(QStringLiteral("Trang %1 / %2").arg(state->pageNumber + 1).arg(totalPages));
        prevBtn->setEnabled(state->pageNumber > 0);
        nextBtn->setEnabled(state->pageNumber < totalPages - 1);

        t->setRowCount(0);
        int start = state->pageNumber * BookingPageState::PAGE_SIZE;
        int end = qMin(start + BookingPageState::PAGE_SIZE, total);

        for (int i = start; i < end; ++i) {
            auto *a = state->appointments[i];
            int r = t->rowCount();
            t->insertRow(r);
            t->setRowHeight(r, 54);

            Patient *p = mainWindow->findPatientForAppointment(*a);
            Doctor *d = mainWindow->findDoctor(a->getDoctorId());
            Date ad = a->getDateGet();

            auto *pw = new QWidget;
            auto *pwl = new QVBoxLayout(pw);
            pwl->setContentsMargins(14, 4, 14, 4);
            pwl->setSpacing(2);
            auto *nl = new QLabel(p ? UIHelper::qs(p->getfullName()) : "—");
            nl->setStyleSheet("font-weight: 700; font-size: 13px; color: #0f172a;");
            nl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            QString pSub = p ? QStringLiteral("%1 tuổi (%2)").arg(p->getAge()).arg(p->getGender() ? "Nam" : "Nữ") : "";
            auto *sl = new QLabel(pSub);
            sl->setStyleSheet("font-size: 11.5px; color: #64748b;");
            sl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            pwl->addWidget(nl);
            pwl->addWidget(sl);
            t->setCellWidget(r, 0, pw);

            auto *dw = new QWidget;
            auto *dwl = new QVBoxLayout(dw);
            dwl->setContentsMargins(14, 4, 14, 4);
            dwl->setSpacing(2);
            auto *dnl = new QLabel(d ? QStringLiteral("BS. %1").arg(UIHelper::qs(d->getfullName())) : UIHelper::qs(a->getDoctorId()));
            dnl->setStyleSheet("font-weight: 700; font-size: 13px; color: #0f172a;");
            dnl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            QString dSub = d ? QStringLiteral("%1 · %2").arg(mainWindow->specialty(d->getSpId()), UIHelper::formatRoom(UIHelper::qs(d->getRoomId()))) : "";
            auto *docSub = new QLabel(dSub);
            docSub->setStyleSheet("font-size: 11.5px; color: #0284c7; font-weight: 600;");
            docSub->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            dwl->addWidget(dnl);
            dwl->addWidget(docSub);
            t->setCellWidget(r, 1, dw);

            auto *tw = new QWidget;
            auto *twl = new QVBoxLayout(tw);
            twl->setContentsMargins(6, 4, 6, 4);
            twl->setSpacing(2);
            auto *hourLabel = new QLabel(QString::asprintf("%02d:%02d", ad.getHour(), ad.getMinute()));
            hourLabel->setStyleSheet("font-weight: 700; font-size: 13px; color: #0f172a;");
            hourLabel->setAlignment(Qt::AlignCenter);
            auto *dayLabel = new QLabel(state->selectedDate == QDate::currentDate() ? QStringLiteral("Hôm nay, %1/%2").arg(ad.getDay()).arg(ad.getMonth()) : state->selectedDate.toString("dd/MM/yyyy"));
            dayLabel->setStyleSheet("font-size: 11.5px; color: #64748b;");
            dayLabel->setAlignment(Qt::AlignCenter);
            twl->addWidget(hourLabel);
            twl->addWidget(dayLabel);
            t->setCellWidget(r, 2, tw);

            int st = a->getState();
            if (st == 0) {
                Date adg = a->getDateGet();
                QDateTime now = QDateTime::currentDateTime();
                QDateTime apptDt(QDate(adg.getYear(), adg.getMonth(), adg.getDay()), QTime(adg.getHour(), adg.getMinute()));
                if (apptDt < now) {
                    st = -1;
                }
            }
            QString badgeText = "● Chưa khám";
            QString badgeStyle = "background: #fffbeb; color: #d97706; border: 1px solid #fde68a; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;";
            if (st == 1) {
                badgeText = "● Đã khám";
                badgeStyle = "background: #ecfdf5; color: #059669; border: 1px solid #a7f3d0; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;";
            } else if (st == -1) {
                badgeText = "● Trễ hẹn";
                badgeStyle = "background: #fff7ed; color: #ea580c; border: 1px solid #ffedd5; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;";
            }
            auto *badge = new QLabel(badgeText);
            badge->setStyleSheet(badgeStyle);
            badge->setAlignment(Qt::AlignCenter);
            badge->setFixedHeight(26);

            auto *bb = new QWidget;
            auto *bbl = new QHBoxLayout(bb);
            bbl->setContentsMargins(6, 0, 6, 0);
            bbl->setAlignment(Qt::AlignCenter);
            bbl->addWidget(badge);
            t->setCellWidget(r, 3, bb);

            auto makeRowClickable = [&](QWidget *widget, Appointment *appt) {
                widget->setCursor(Qt::PointingHandCursor);
                widget->installEventFilter(new AppointmentRowClickFilter(appt, this, widget));
                const auto children = widget->findChildren<QWidget*>();
                for (auto *c : children) {
                    c->setCursor(Qt::PointingHandCursor);
                    c->installEventFilter(new AppointmentRowClickFilter(appt, this, c));
                }
            };
            makeRowClickable(pw, a);
            makeRowClickable(dw, a);
            makeRowClickable(tw, a);
            makeRowClickable(bb, a);
        }
    };

    connect(t, &QTableWidget::cellClicked, w, [=, this](int r, int) {
        int idx = state->pageNumber * BookingPageState::PAGE_SIZE + r;
        if (idx >= 0 && idx < static_cast<int>(state->appointments.size())) {
            showAppointmentDetailDialog(state->appointments[idx]);
            reloadDayView();
        }
    });

    connect(holidayBtn, &QPushButton::clicked, w, [=, this]() {
        showHolidayManagementDialog(w);
        highlightCalendarAppointments(cal, yearSpin->value(), monthCombo->currentData().toInt());
        reloadDayView();
    });

    connect(cal, &QCalendarWidget::clicked, w, [=](const QDate &d) {
        state->selectedDate = d;
        state->pageNumber = 0;
        reloadDayView();
    });

    QDate cur = QDate::currentDate();
    monthCombo->setCurrentIndex(cur.month() - 1);
    setComboYear(cur.year());
    cal->setSelectedDate(cur);
    highlightCalendarAppointments(cal, cur.year(), cur.month());
    reloadDayView();

    connect(prevBtn, &QPushButton::clicked, w, [=]() {
        if (state->pageNumber > 0) {
            state->pageNumber--;
            reloadDayView();
        }
    });
    connect(nextBtn, &QPushButton::clicked, w, [=]() {
        int totalPages = state->appointments.empty() ? 1 : (static_cast<int>(state->appointments.size()) + 7) / 8;
        if (state->pageNumber < totalPages - 1) {
            state->pageNumber++;
            reloadDayView();
        }
    });

    auto onPickerChanged = [=, this]() {
        int m = monthCombo->currentData().toInt();
        int y = yearSpin->value();
        cal->blockSignals(true);
        cal->setCurrentPage(qMax(1, y), m);
        cal->blockSignals(false);
        highlightCalendarAppointments(cal, y, m);
    };
    connect(monthCombo, qOverload<int>(&QComboBox::currentIndexChanged), w, onPickerChanged);
    connect(yearSpin, qOverload<int>(&QSpinBox::valueChanged), w, onPickerChanged);

    connect(prevMonthBtn, &QPushButton::clicked, w, [=]() {
        int m = monthCombo->currentData().toInt();
        int y = yearSpin->value();
        if (m == 1) { m = 12; y--; } else { m--; }
        setComboYear(y);
        monthCombo->setCurrentIndex(m - 1);
    });
    connect(nextMonthBtn, &QPushButton::clicked, w, [=]() {
        int m = monthCombo->currentData().toInt();
        int y = yearSpin->value();
        if (m == 12) { m = 1; y++; } else { m++; }
        setComboYear(y);
        monthCombo->setCurrentIndex(m - 1);
    });

    connect(prevYearBtn, &QPushButton::clicked, w, [=]() {
        yearSpin->setValue(qMax(0, yearSpin->value() - 1));
    });
    connect(nextYearBtn, &QPushButton::clicked, w, [=]() {
        yearSpin->setValue(yearSpin->value() + 1);
    });

    connect(todayBtn, &QPushButton::clicked, w, [=, this]() {
        QDate today = QDate::currentDate();
        yearSpin->blockSignals(true);
        setComboYear(today.year());
        yearSpin->blockSignals(false);
        monthCombo->blockSignals(true);
        monthCombo->setCurrentIndex(today.month() - 1);
        monthCombo->blockSignals(false);
        cal->setCurrentPage(today.year(), today.month());
        cal->setSelectedDate(today);
        state->selectedDate = today;
        state->pageNumber = 0;
        highlightCalendarAppointments(cal, today.year(), today.month());
        reloadDayView();
    });

    connect(cal, &QCalendarWidget::currentPageChanged, w, [=, this](int year, int month) {
        yearSpin->blockSignals(true);
        setComboYear(year);
        yearSpin->blockSignals(false);
        monthCombo->blockSignals(true);
        monthCombo->setCurrentIndex(month - 1);
        monthCombo->blockSignals(false);
        highlightCalendarAppointments(cal, year, month);
    });

    auto openBooking = [=, this]() {
        if (state->selectedDate < QDate::currentDate()) {
            QMessageBox::warning(w, "Không thể đặt lịch", "Không thể đặt lịch khám cho ngày trước thời gian hiện tại.");
            return;
        }
        if (mainWindow->isHoliday(state->selectedDate)) {
            QMessageBox::warning(w, "Ngày nghỉ lễ", "Không thể đặt lịch khám vào ngày nghỉ lễ.");
            return;
        }
        if (showBookingDialog(state->selectedDate)) {
            highlightCalendarAppointments(cal, yearSpin->value(), monthCombo->currentData().toInt());
            reloadDayView();
        }
    };
    connect(bookBtn, &QPushButton::clicked, w, openBooking);
    connect(quickBookBtn, &QPushButton::clicked, w, openBooking);

    return scroll;
}

QWidget *AppointmentView::createAppointmentListWidget(QWidget *parent) {
    QWidget *w = UIHelper::page("Quản lý lịch khám", "Theo dõi lịch theo bác sĩ, ngày hẹn và trạng thái khám.", parent);
    auto *layout = qobject_cast<QVBoxLayout*>(w->layout());

    // Filter controls inside a neat card
    auto *filterCard = new QFrame;
    filterCard->setObjectName("filterCard");
    filterCard->setStyleSheet(R"(
        QFrame#filterCard {
            background-color: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 10px;
        }
        QLabel {
            font-size: 12px;
            font-weight: 700;
            color: #475569;
        }
        QComboBox, QDateEdit {
            background: #f8fafc;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 5px 20px 5px 8px;
            font-size: 13px;
            color: #0f172a;
            min-height: 22px;
        }
        QComboBox:disabled, QDateEdit:disabled {
            background: #f1f5f9;
            color: #94a3b8;
            border-color: #e2e8f0;
        }
        QComboBox:focus, QDateEdit:focus {
            border-color: #0d9488;
            background: #ffffff;
        }
        QComboBox::drop-down, QDateEdit::drop-down {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 24px;
            border: none;
            background: transparent;
        }
        QComboBox::drop-down:disabled, QDateEdit::drop-down:disabled {
            width: 0px;
        }
        QComboBox::down-arrow, QDateEdit::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 10px;
            height: 10px;
            margin-right: 6px;
        }
        QComboBox::down-arrow:disabled, QDateEdit::down-arrow:disabled {
            image: none;
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
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #f0fdfa;
            color: #0f766e;
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: #0d9488;
            color: #ffffff;
        }
    )");

    auto *filters = new QHBoxLayout(filterCard);
    filters->setContentsMargins(14, 8, 14, 8);
    filters->setSpacing(12);

    // 1. Cột Bác sĩ
    auto *docCol = new QVBoxLayout;
    docCol->setSpacing(3);
    auto *lblDoctor = new QLabel("Bác sĩ:");
    lblDoctor->setStyleSheet("font-size: 11.5px; font-weight: 700; color: #475569;");
    auto *doctor = new QComboBox;
    doctor->view()->setTextElideMode(Qt::ElideRight);
    doctor->addItem("Tất cả bác sĩ", "");
    auto *doctors = mainWindow ? mainWindow->getDoctors() : nullptr;
    if (doctors) {
        auto *d = doctors->getHeader()->next;
        while (d && d->next) {
            doctor->addItem(QStringLiteral("BS. %1").arg(UIHelper::qs(d->info.getfullName())), UIHelper::qs(d->info.getID()));
            d = d->next;
        }
    }
    doctor->setMinimumWidth(160);
    doctor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    docCol->addWidget(lblDoctor);
    docCol->addWidget(doctor);

    // 2. Cột Ngày khám
    auto *dateCol = new QVBoxLayout;
    dateCol->setSpacing(3);
    auto *lblDate = new QLabel("Ngày khám:");
    lblDate->setStyleSheet("font-size: 11.5px; font-weight: 700; color: #475569;");
    dateCol->addWidget(lblDate);

    auto *dateRow = new QHBoxLayout;
    dateRow->setSpacing(8);
    dateRow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *date = new QDateEdit(QDate::currentDate());
    date->setCalendarPopup(true);
    date->setDisplayFormat("dd/MM/yyyy");
    date->setFixedWidth(125);

    // Hoàn thiện Calendar Widget cho input Date
    if (auto *calPopup = date->calendarWidget()) {
        calPopup->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
        calPopup->setFirstDayOfWeek(Qt::Monday);
        calPopup->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        calPopup->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
        calPopup->setFixedSize(350, 280);
        calPopup->setStyleSheet(R"(
            QCalendarWidget {
                background-color: #ffffff;
                border: 1.5px solid #cbd5e1;
                border-radius: 12px;
                font-family: 'Segoe UI', sans-serif;
            }
            QCalendarWidget QWidget#qt_calendar_navigationbar {
                background-color: #f8fafc;
                border-top-left-radius: 11px;
                border-top-right-radius: 11px;
                border-bottom: 1.5px solid #e2e8f0;
                min-height: 38px;
            }
            QCalendarWidget QToolButton {
                color: #0f172a;
                font-weight: 750;
                font-size: 13px;
                background: transparent;
                border: none;
                border-radius: 6px;
                padding: 4px 8px;
                margin: 2px;
            }
            QCalendarWidget QToolButton:hover {
                background-color: #f0fdfa;
                color: #0d9488;
            }
            QCalendarWidget QToolButton#qt_calendar_prevmonth {
                qproperty-icon: none;
                text: "‹";
                font-size: 18px;
                font-weight: bold;
            }
            QCalendarWidget QToolButton#qt_calendar_nextmonth {
                qproperty-icon: none;
                text: "›";
                font-size: 18px;
                font-weight: bold;
            }
            QCalendarWidget QMenu {
                background-color: #ffffff;
                color: #0f172a;
                border: 1px solid #cbd5e1;
                border-radius: 8px;
                padding: 4px;
            }
            QCalendarWidget QSpinBox {
                background: #ffffff;
                border: 1.5px solid #cbd5e1;
                border-radius: 6px;
                color: #0f172a;
                font-weight: 700;
                padding: 2px 6px;
            }
            QCalendarWidget QTableView {
                background-color: #ffffff;
                selection-background-color: #0d9488;
                selection-color: #ffffff;
                color: #1e293b;
                font-size: 12.5px;
                font-weight: 600;
                outline: none;
                border: none;
            }
            QCalendarWidget QTableView:item:hover {
                background-color: #ccfbf1;
                color: #0f766e;
                border-radius: 4px;
            }
            QCalendarWidget QTableView:item:selected {
                background-color: #0d9488;
                color: #ffffff;
                border-radius: 4px;
                font-weight: bold;
            }
            QCalendarWidget QWidget#qt_calendar_calendarview {
                border-bottom-left-radius: 11px;
                border-bottom-right-radius: 11px;
            }
        )");
    }

    auto *allDates = new ToggleSwitch;
    allDates->setChecked(true);
    allDates->setToolTip(QString::fromUtf8("Tất cả các ngày (Bật để xem mọi ngày, Tắt để chọn ngày khám)"));
    date->setEnabled(false);

    dateRow->addWidget(date);
    dateRow->addWidget(allDates);
    dateCol->addLayout(dateRow);

    // 3. Cột Trạng thái
    auto *statusCol = new QVBoxLayout;
    statusCol->setSpacing(3);
    auto *lblStatus = new QLabel("Trạng thái:");
    lblStatus->setStyleSheet("font-size: 11.5px; font-weight: 700; color: #475569;");
    auto *status = new QComboBox;
    status->view()->setTextElideMode(Qt::ElideRight);
    status->addItems({"Tất cả trạng thái", "Chưa khám", "Đã khám", "Trễ hẹn"});
    status->setMinimumWidth(150);
    status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    statusCol->addWidget(lblStatus);
    statusCol->addWidget(status);

    filters->addLayout(docCol, 4);
    filters->addSpacing(16);
    filters->addLayout(dateCol, 0);
    filters->addSpacing(16);
    filters->addLayout(statusCol, 2);
    layout->addWidget(filterCard);

    // Actions bar
    auto *actions = new QHBoxLayout;
    auto *invite = UIHelper::button("Mời bệnh nhân đến khám");

    if (mainWindow && !mainWindow->hasPermission("Per_4")) {
        invite->hide();
    }

    actions->addWidget(invite);
    actions->addStretch();
    layout->addLayout(actions);

    // Table: Bỏ mã lịch, Bỏ triệu chứng, Tách Bác sĩ & Chuyên khoa, Đổi Ngày giờ hẹn thành Ngày khám
    auto *t = UIHelper::table({"BỆNH NHÂN", "BÁC SĨ", "CHUYÊN KHOA", "NGÀY KHÁM", "TRẠNG THÁI"});
    t->setCursor(Qt::PointingHandCursor);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setFocusPolicy(Qt::StrongFocus);
    t->setStyleSheet(R"(
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
        QTableWidget::item:selected {
            background-color: #ccfbf1;
            color: #0f766e;
            font-weight: 700;
        }
    )");

    t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    t->setColumnWidth(1, 180);
    t->setColumnWidth(2, 180);
    t->setColumnWidth(3, 170);
    t->setColumnWidth(4, 140);
    t->verticalHeader()->setDefaultSectionSize(48);

    if (t->horizontalHeaderItem(0)) t->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (t->horizontalHeaderItem(1)) t->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (t->horizontalHeaderItem(2)) t->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (t->horizontalHeaderItem(3)) t->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (t->horizontalHeaderItem(4)) t->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignCenter);

    layout->addWidget(t, 1);

    // Pagination: 50 stack 1 trang
    const int PAGE_SIZE = 50;
    auto currentPage = std::make_shared<int>(0);

    auto *pagination = new QHBoxLayout;
    pagination->setContentsMargins(4, 4, 4, 0);

    auto *totalLabel = new QLabel("Tổng số: 0 lịch hẹn");
    totalLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #475569;");
    pagination->addWidget(totalLabel);

    pagination->addStretch();

    auto *prevBtn = UIHelper::button("‹  Trang trước", "secondary");
    auto *nextBtn = UIHelper::button("Trang sau  ›", "secondary");
    auto *pageStatus = new QLabel("Trang 1 / 1");
    pageStatus->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #0f172a; padding: 0 12px;");

    pagination->addWidget(prevBtn);
    pagination->addWidget(pageStatus);
    pagination->addWidget(nextBtn);
    layout->addLayout(pagination);

    auto computeApptStatus = [](const Appointment &a, Doctor *d) -> QString {
        (void)d;
        if (a.getState() == 1) return QStringLiteral("Đã khám");
        if (a.getState() == -1) return QStringLiteral("Trễ hẹn");
        Date ad = a.getDateGet();
        QDateTime now = QDateTime::currentDateTime();
        QDateTime apptDt(QDate(ad.getYear(), ad.getMonth(), ad.getDay()), QTime(ad.getHour(), ad.getMinute()));
        if (apptDt < now) {
            return QStringLiteral("Trễ hẹn");
        }
        return QStringLiteral("Chưa khám");
    };

    auto fill = [this, t, doctor, date, allDates, status, totalLabel, prevBtn, nextBtn, pageStatus, currentPage, PAGE_SIZE, computeApptStatus] {
        t->setRowCount(0);
        auto *appointments = mainWindow ? mainWindow->getAppointments() : nullptr;
        if (!appointments) {
            totalLabel->setText("Tổng số: 0 lịch hẹn");
            pageStatus->setText("Trang 1 / 1");
            prevBtn->setEnabled(false);
            nextBtn->setEnabled(false);
            return;
        }

        QList<Appointment*> ordered;
        auto *n = appointments->getHeader()->next;
        while (n && n->next) {
            if (!n->info.isDefault()) ordered.append(&n->info);
            n = n->next;
        }

        AppointmentController::sortAppointmentsByDate(ordered);

        QList<Appointment*> filtered;
        for (auto *item : ordered) {
            auto &a = *item;
            Doctor *d = mainWindow->findDoctor(a.getDoctorId());
            Date ad = a.getDateGet();

            bool matchesDoctor = doctor->currentData().toString().isEmpty() || UIHelper::qs(a.getDoctorId()) == doctor->currentData().toString();
            bool matchesDate = allDates->isChecked() || (ad.getYear() == date->date().year() && ad.getMonth() == date->date().month() && ad.getDay() == date->date().day());
            QString sText = computeApptStatus(a, d);
            bool matchesStatus = (status->currentIndex() == 0) || (status->currentText() == sText);

            if (matchesDoctor && matchesDate && matchesStatus) {
                filtered.append(item);
            }
        }

        totalLabel->setText(QStringLiteral("Tổng số: %1 lịch hẹn").arg(filtered.size()));

        int totalPages = (filtered.size() + PAGE_SIZE - 1) / PAGE_SIZE;
        if (totalPages < 1) totalPages = 1;
        if (*currentPage >= totalPages) *currentPage = totalPages - 1;
        if (*currentPage < 0) *currentPage = 0;

        pageStatus->setText(QStringLiteral("Trang %1 / %2").arg(*currentPage + 1).arg(totalPages));
        prevBtn->setEnabled(*currentPage > 0);
        nextBtn->setEnabled(*currentPage + 1 < totalPages);

        int startIdx = *currentPage * PAGE_SIZE;
        int endIdx = qMin(startIdx + PAGE_SIZE, filtered.size());

        for (int i = startIdx; i < endIdx; ++i) {
            auto &a = *filtered[i];
            Doctor *d = mainWindow->findDoctor(a.getDoctorId());
            Patient *p = mainWindow->findPatientForAppointment(a);
            Date ad = a.getDateGet();
            QString sText = computeApptStatus(a, d);

            int r = t->rowCount();
            t->insertRow(r);

            auto *pItem = new QTableWidgetItem(p ? UIHelper::qs(p->getfullName()) : "—");
            pItem->setData(Qt::UserRole, UIHelper::qs(a.getId()));
            pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            t->setItem(r, 0, pItem);

            UIHelper::cell(t, r, 1, d ? QStringLiteral("BS. %1").arg(UIHelper::qs(d->getfullName())) : UIHelper::qs(a.getDoctorId()), Qt::AlignLeft | Qt::AlignVCenter);
            UIHelper::cell(t, r, 2, d ? mainWindow->specialty(d->getSpId()) : "—", Qt::AlignLeft | Qt::AlignVCenter);

            QString dateStr = QString::asprintf("%02d:%02d · %02d/%02d/%04d", ad.getHour(), ad.getMinute(), ad.getDay(), ad.getMonth(), ad.getYear());
            UIHelper::cell(t, r, 3, dateStr, Qt::AlignLeft | Qt::AlignVCenter);

            auto *statusItem = new QTableWidgetItem(sText);
            statusItem->setTextAlignment(Qt::AlignCenter);
            if (sText == "Đã khám") {
                statusItem->setForeground(QColor("#059669"));
            } else if (sText == "Chưa khám") {
                statusItem->setForeground(QColor("#d97706"));
            } else if (sText == "Trễ hẹn") {
                statusItem->setForeground(QColor("#ea580c"));
            }
            QFont f = statusItem->font();
            f.setBold(true);
            statusItem->setFont(f);
            t->setItem(r, 4, statusItem);
        }
    };

    connect(doctor, qOverload<int>(&QComboBox::currentIndexChanged), w, [fill, currentPage] { *currentPage = 0; fill(); });
    connect(date, &QDateEdit::dateChanged, w, [fill, currentPage] { *currentPage = 0; fill(); });
    connect(allDates, &QAbstractButton::toggled, w, [fill, currentPage, date](bool c) {
        *currentPage = 0;
        date->setEnabled(!c);
        fill();
    });
    connect(status, qOverload<int>(&QComboBox::currentIndexChanged), w, [fill, currentPage] { *currentPage = 0; fill(); });

    connect(prevBtn, &QPushButton::clicked, w, [fill, currentPage] {
        if (*currentPage > 0) {
            (*currentPage)--;
            fill();
        }
    });

    connect(nextBtn, &QPushButton::clicked, w, [fill, currentPage] {
        (*currentPage)++;
        fill();
    });

    connect(invite, &QPushButton::clicked, w, [this, t, fill] {
        int row = t->currentRow();
        if (row < 0 || !t->item(row, 0)) {
            QMessageBox::information(mainWindow, "Chọn lịch", "Hãy chọn lịch hẹn cần mời.");
            return;
        }
        std::string id = UIHelper::ss(t->item(row, 0)->data(Qt::UserRole).toString());
        auto *appointments = mainWindow->getAppointments();
        if (appointments) {
            auto *n = appointments->getHeader()->next;
            while (n && n->next) {
                if (n->info.getId() == id) {
                    if (n->info.getState()) {
                        QMessageBox::information(mainWindow, "Lịch đã hoàn tất", "Lịch này đã ở trạng thái đã khám.");
                        return;
                    }
                    Patient *p = mainWindow->findPatientForAppointment(n->info);
                    Doctor *d = mainWindow->findDoctor(n->info.getDoctorId());
                    std::string patientId = mainWindow->getPatientIdForAppointment(n->info);
                    QString patientName = p ? UIHelper::qs(p->getfullName()) : "Bệnh nhân";
                    QString room = d ? UIHelper::formatRoom(UIHelper::qs(d->getRoomId())) : QStringLiteral("chưa xác định");

                    QMessageBox::information(mainWindow, "Mời bệnh nhân đến khám", QStringLiteral("Mời bệnh nhân %1 đến phòng %2 để khám bệnh.").arg(patientName, room));
                    n->info.setState(1);
                    mainWindow->updatePatientSymptomStatus(patientId);
                    mainWindow->saveAll();
                    fill();
                    mainWindow->refreshPage(2);
                    return;
                }
                n = n->next;
            }
        }
    });


    connect(t, &QTableWidget::cellDoubleClicked, w, [this, t, fill](int row, int) {
        if (row >= 0 && t->item(row, 0)) {
            std::string id = UIHelper::ss(t->item(row, 0)->data(Qt::UserRole).toString());
            auto *appointments = mainWindow ? mainWindow->getAppointments() : nullptr;
            if (appointments) {
                auto *n = appointments->getHeader()->next;
                while (n && n->next) {
                    if (n->info.getId() == id) {
                        showAppointmentDetailDialog(&n->info);
                        fill();
                        return;
                    }
                    n = n->next;
                }
            }
        }
    });

    fill();
    return w;
}

void AppointmentView::showAppointmentDetailDialog(Appointment *a) {
    if (!mainWindow || !a) return;

    bool canEdit = mainWindow->hasPermission("Per_4");

    Patient *p = mainWindow->findPatientForAppointment(*a);
    std::string patientId = mainWindow->getPatientIdForAppointment(*a);
    Doctor *d = mainWindow->findDoctor(a->getDoctorId());
    Date ad = a->getDateGet();
    Date sd = a->getDateSet();

    QDialog dialog(mainWindow);
    dialog.setWindowTitle("Chi tiết lịch khám bệnh");
    dialog.setFixedWidth(610);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
        }
        QComboBox, QDateTimeEdit {
            background: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 6px 28px 6px 10px;
            font-size: 13px;
            color: #0f172a;
            min-height: 20px;
        }
        QComboBox:focus, QDateTimeEdit:focus {
            border: 1.5px solid #0d9488;
            background: #f0fdfa;
        }
        QComboBox::drop-down, QDateTimeEdit::drop-down {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 26px;
            border: none;
            background: transparent;
        }
        QComboBox::down-arrow, QDateTimeEdit::down-arrow {
            image: url(:/assets/textures/arrow_down_dark.png);
            width: 10px;
            height: 10px;
            margin-right: 6px;
        }
        QComboBox::down-arrow:hover, QDateTimeEdit::down-arrow:hover {
            image: url(:/assets/textures/arrow_down_teal.png);
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
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    // 1. Header Banner
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(12);

    auto *icon = new QLabel("📋");
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 22px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel("Chi tiết lịch khám bệnh");
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    titleCol->addWidget(title);

    auto *roleBadge = new QLabel(canEdit ? "● Có quyền sửa" : "● Chỉ xem");
    roleBadge->setStyleSheet(canEdit
        ? "background: #eff6ff; color: #2563eb; border: 1px solid #bfdbfe; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;"
        : "background: #f1f5f9; color: #475569; border: 1px solid #cbd5e1; border-radius: 12px; font-size: 11.5px; font-weight: 700; padding: 4px 10px;");

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    headerBox->addWidget(roleBadge, 0, Qt::AlignVCenter);
    root->addLayout(headerBox);

    auto makeSectionCard = [](const QString &iconTitle) {
        auto *f = new QFrame;
        f->setStyleSheet("QFrame#card { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
        f->setObjectName("card");
        auto *fl = new QVBoxLayout(f);
        fl->setContentsMargins(16, 12, 16, 12);
        fl->setSpacing(8);

        auto *h = new QLabel(iconTitle);
        h->setStyleSheet("font-size: 11.5px; font-weight: 800; color: #0f766e; letter-spacing: 0.6px; text-transform: uppercase;");
        fl->addWidget(h);
        return qMakePair(f, fl);
    };

    auto makeField = [](const QString &label, const QString &val, bool isBold = false) {
        auto *col = new QVBoxLayout;
        col->setSpacing(2);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748b;");
        auto *v = new QLabel(val);
        v->setStyleSheet(isBold
            ? "font-size: 13.5px; font-weight: 700; color: #0f172a;"
            : "font-size: 13px; font-weight: 600; color: #1e293b;");
        col->addWidget(lbl);
        col->addWidget(v);
        return col;
    };

    // 2. Card Thông tin bệnh nhân (TRỪ MÃ RA)
    auto pCardPair = makeSectionCard("👤  Thông tin bệnh nhân");
    auto *pGrid = new QGridLayout;
    pGrid->setHorizontalSpacing(20);
    pGrid->setVerticalSpacing(8);
    QString pName = p ? UIHelper::qs(p->getfullName()) : UIHelper::qs(mainWindow->getPatientIdForAppointment(*a));
    QString pGenderAge = p ? QStringLiteral("%1 tuổi · %2").arg(p->getAge()).arg(p->getGender() ? "Nam" : "Nữ") : "—";
    QString pPhone = (p && !p->getPhone().empty()) ? UIHelper::qs(p->getPhone()) : "Chưa cập nhật";
    QString pCccd = (p && !p->getCCCD().empty()) ? UIHelper::qs(p->getCCCD()) : "Chưa cập nhật";

    pGrid->addLayout(makeField("Họ và tên", pName, true), 0, 0);
    pGrid->addLayout(makeField("Tuổi & Giới tính", pGenderAge), 0, 1);
    pGrid->addLayout(makeField("Số điện thoại", pPhone), 1, 0);
    pGrid->addLayout(makeField("Số CCCD", pCccd), 1, 1);
    pCardPair.second->addLayout(pGrid);
    root->addWidget(pCardPair.first);

    // 3. Card Triệu chứng lâm sàng
    auto sCardPair = makeSectionCard("🩺  Triệu chứng lâm sàng");
    QString apptSymptomName = mainWindow->getSymptomNamesForAppointment(*a);
    if (apptSymptomName.isEmpty()) {
        apptSymptomName = "Chưa ghi nhận";
    }

    auto *allSymRow = new QHBoxLayout;
    auto *allSymLbl = new QLabel("Các triệu chứng đã ghi nhận:");
    allSymLbl->setStyleSheet("font-size: 11.5px; font-weight: 600; color: #64748b;");
    auto *allSymVal = new QLabel(apptSymptomName);
    allSymVal->setStyleSheet("font-size: 12.5px; font-weight: 600; color: #1e293b;");
    allSymVal->setWordWrap(true);
    allSymRow->addWidget(allSymLbl);
    allSymRow->addWidget(allSymVal, 1);
    sCardPair.second->addLayout(allSymRow);
    root->addWidget(sCardPair.first);

    // 4. Card Bác sĩ nhận ca & Thông tin bác sĩ
    auto dCardPair = makeSectionCard("👨‍⚕️  Bác sĩ phụ trách");
    auto *dGrid = new QGridLayout;
    dGrid->setHorizontalSpacing(20);
    dGrid->setVerticalSpacing(8);

    QLabel *dSpecVal = new QLabel(d ? mainWindow->specialty(d->getSpId()) : "—");
    dSpecVal->setStyleSheet("font-size: 13px; font-weight: 600; color: #1e293b;");
    QLabel *dRoomVal = new QLabel(d ? UIHelper::formatRoom(UIHelper::qs(d->getRoomId())) : "Chưa xác định");
    dRoomVal->setStyleSheet("font-size: 13.5px; font-weight: 700; color: #0f172a;");
    QLabel *dPhoneVal = new QLabel((d && !d->getPhone().empty()) ? UIHelper::qs(d->getPhone()) : "Chưa cập nhật");
    dPhoneVal->setStyleSheet("font-size: 13px; font-weight: 600; color: #1e293b;");
    QLabel *dRatingVal = new QLabel(d ? QStringLiteral("⭐ %1 / 5.0").arg(QString::number(d->getRating(), 'f', 1)) : "—");
    dRatingVal->setStyleSheet("font-size: 13px; font-weight: 600; color: #1e293b;");
    QLabel *dFeeVal = new QLabel(d ? (QLocale(QLocale::Vietnamese).toString(d->getFee(), 'f', 0) + " đ") : "—");
    dFeeVal->setStyleSheet("font-size: 13px; font-weight: 600; color: #1e293b;");

    auto wrapField = [](const QString &label, QLabel *valWidget) {
        auto *col = new QVBoxLayout;
        col->setSpacing(2);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 11px; font-weight: 600; color: #64748b;");
        col->addWidget(lbl);
        col->addWidget(valWidget);
        return col;
    };

    QComboBox *docCombo = nullptr;
    if (canEdit) {
        auto *docCol = new QVBoxLayout;
        docCol->setSpacing(3);
        auto *docLbl = new QLabel("Bác sĩ nhận ca");
        docLbl->setStyleSheet("font-size: 11px; font-weight: 700; color: #334155;");
        docCombo = new QComboBox;
        auto *doctors = mainWindow->getDoctors();
        if (doctors) {
            auto *docNode = doctors->getHeader()->next;
            while (docNode && docNode->next) {
                docCombo->addItem(QStringLiteral("BS. %1").arg(UIHelper::qs(docNode->info.getfullName())), UIHelper::qs(docNode->info.getID()));
                docNode = docNode->next;
            }
        }
        int docIdx = docCombo->findData(UIHelper::qs(a->getDoctorId()));
        if (docIdx >= 0) docCombo->setCurrentIndex(docIdx);
        docCol->addWidget(docLbl);
        docCol->addWidget(docCombo);
        dGrid->addLayout(docCol, 0, 0);

        QObject::connect(docCombo, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [this, docCombo, dSpecVal, dRoomVal, dPhoneVal, dRatingVal, dFeeVal] {
            std::string selDocId = UIHelper::ss(docCombo->currentData().toString());
            Doctor *currDoc = mainWindow ? mainWindow->findDoctor(selDocId) : nullptr;
            if (currDoc) {
                dSpecVal->setText(mainWindow->specialty(currDoc->getSpId()));
                dRoomVal->setText(UIHelper::formatRoom(UIHelper::qs(currDoc->getRoomId())));
                dPhoneVal->setText(!currDoc->getPhone().empty() ? UIHelper::qs(currDoc->getPhone()) : "Chưa cập nhật");
                dRatingVal->setText(QStringLiteral("⭐ %1 / 5.0").arg(QString::number(currDoc->getRating(), 'f', 1)));
                dFeeVal->setText(QLocale(QLocale::Vietnamese).toString(currDoc->getFee(), 'f', 0) + " đ");
            } else {
                dSpecVal->setText("—");
                dRoomVal->setText("Chưa xác định");
                dPhoneVal->setText("Chưa cập nhật");
                dRatingVal->setText("—");
                dFeeVal->setText("—");
            }
        });
    } else {
        QString dName = d ? QStringLiteral("BS. %1").arg(UIHelper::qs(d->getfullName())) : UIHelper::qs(a->getDoctorId());
        dGrid->addLayout(makeField("Bác sĩ nhận ca", dName, true), 0, 0);
    }

    dGrid->addLayout(wrapField("Chuyên khoa", dSpecVal), 0, 1);
    dGrid->addLayout(wrapField("Phòng khám chỉ định", dRoomVal), 1, 0);
    dGrid->addLayout(wrapField("Số điện thoại bác sĩ", dPhoneVal), 1, 1);
    dGrid->addLayout(wrapField("Đánh giá chuyên môn", dRatingVal), 2, 0);
    dGrid->addLayout(wrapField("Phí khám tham chiếu", dFeeVal), 2, 1);
    dCardPair.second->addLayout(dGrid);
    root->addWidget(dCardPair.first);

    // 5. Card Thời gian & Trạng thái appointment
    auto aCardPair = makeSectionCard("📅  Thời gian & Trạng thái lịch khám");
    auto *aGrid = new QGridLayout;
    aGrid->setHorizontalSpacing(20);
    aGrid->setVerticalSpacing(8);

    QString setDateStr = QString::asprintf("%02d/%02d/%04d %02d:%02d", sd.getDay(), sd.getMonth(), sd.getYear(), sd.getHour(), sd.getMinute());
    aGrid->addLayout(makeField("Ngày lên lịch hẹn", setDateStr), 0, 0);

    QDateTimeEdit *dateEdit = nullptr;
    QComboBox *statCombo = nullptr;

    if (canEdit) {
        auto *dateCol = new QVBoxLayout;
        dateCol->setSpacing(3);
        auto *dateLbl = new QLabel("Ngày đi khám");
        dateLbl->setStyleSheet("font-size: 11px; font-weight: 700; color: #334155;");
        dateEdit = new QDateTimeEdit(QDateTime(QDate(ad.getYear(), ad.getMonth(), ad.getDay()), QTime(ad.getHour(), ad.getMinute())));
        dateEdit->setDisplayFormat("dd/MM/yyyy HH:mm");
        dateEdit->setCalendarPopup(true);
        dateCol->addWidget(dateLbl);
        dateCol->addWidget(dateEdit);
        aGrid->addLayout(dateCol, 0, 1);

        auto *statCol = new QVBoxLayout;
        statCol->setSpacing(3);
        auto *statLbl = new QLabel("Trạng thái ca khám");
        statLbl->setStyleSheet("font-size: 11px; font-weight: 700; color: #334155;");
        statCombo = new QComboBox;
        statCombo->addItem("Chưa khám", 0);
        statCombo->addItem("Đã khám", 1);
        statCombo->addItem("Trễ hẹn", -1);
        int curSt = a->getState();
        int statIdx = statCombo->findData(curSt);
        if (statIdx >= 0) statCombo->setCurrentIndex(statIdx);
        statCol->addWidget(statLbl);
        statCol->addWidget(statCombo);
        aGrid->addLayout(statCol, 1, 0);
    } else {
        QString apptDateStr = QString::asprintf("%02d:%02d · Ngày %02d/%02d/%04d", ad.getHour(), ad.getMinute(), ad.getDay(), ad.getMonth(), ad.getYear());
        aGrid->addLayout(makeField("Ngày đi khám", apptDateStr, true), 0, 1);
        QString stText = "Chưa khám (Đang chờ)";
        if (a->getState() == 1) stText = "Đã khám";
        else if (a->getState() == -1) stText = "Trễ hẹn";
        aGrid->addLayout(makeField("Trạng thái ca khám", stText), 1, 0);
    }

    aCardPair.second->addLayout(aGrid);
    root->addWidget(aCardPair.first);

    // Footer Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);
    btnRow->addStretch();

    if (canEdit) {
        auto *cancelBtn = new QPushButton("Hủy");
        cancelBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setStyleSheet("QPushButton { background: #ffffff; color: #475569; border: 1.5px solid #cbd5e1; border-radius: 8px; padding: 9px 22px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #f1f5f9; color: #0f172a; }");
        QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

        auto *saveBtn = new QPushButton("✓  Lưu thay đổi");
        saveBtn->setCursor(Qt::PointingHandCursor);
        saveBtn->setStyleSheet("QPushButton { background: #0d9488; color: #ffffff; border: none; border-radius: 8px; padding: 9px 24px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #0f766e; }");

        QObject::connect(saveBtn, &QPushButton::clicked, &dialog, [this, &dialog, a, docCombo, dateEdit, statCombo] {
            if (docCombo && docCombo->currentIndex() < 0) {
                QMessageBox::warning(&dialog, "Thiếu thông tin", "Hãy chọn bác sĩ phụ trách.");
                return;
            }
            if (dateEdit && mainWindow->isHoliday(dateEdit->date())) {
                QMessageBox::warning(&dialog, "Ngày nghỉ lễ", "Không thể xếp lịch vào ngày nghỉ lễ.");
                return;
            }

            if (docCombo) {
                a->setDoctorId(UIHelper::ss(docCombo->currentData().toString()));
            }

            if (dateEdit) {
                a->setDateGet(Date::loadFromString(UIHelper::ss(dateEdit->dateTime().toString("yyyy-MM-dd HH:mm"))));
            }

            if (statCombo) {
                int newSt = statCombo->currentData().toInt();
                a->setState(newSt);
                SymptomPatient *sp = mainWindow->findSymptomPatient(a->getSymptomPatientId());
                if (sp) {
                    sp->setResolved(newSt);
                }
            }

            std::string patientId = mainWindow->getPatientIdForAppointment(*a);
            if (!patientId.empty()) {
                mainWindow->updatePatientSymptomStatus(patientId);
            }
            mainWindow->saveAll();
            dialog.accept();
        });

        btnRow->addWidget(cancelBtn);
        btnRow->addWidget(saveBtn);
    } else {
        auto *closeBtn = new QPushButton("Đóng");
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setStyleSheet("QPushButton { background: #0d9488; color: #ffffff; border: none; border-radius: 8px; padding: 9px 28px; font-weight: 700; font-size: 13px; } QPushButton:hover { background: #0f766e; }");
        QObject::connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
        btnRow->addWidget(closeBtn);
    }

    root->addLayout(btnRow);
    dialog.setFocusPolicy(Qt::StrongFocus);
    dialog.setFocus();
    QTimer::singleShot(0, [&dialog]() { dialog.setFocus(); });
    dialog.exec();
}

void AppointmentView::editAppointmentDialog(const std::string &id) {
    if (!mainWindow || !mainWindow->getAppointments()) return;
    auto *n = mainWindow->getAppointments()->getHeader()->next;
    while (n && n->next) {
        if (n->info.getId() == id) {
            showAppointmentDetailDialog(&n->info);
            return;
        }
        n = n->next;
    }
}

QWidget *AppointmentView::createHolidayWidget(QWidget *parent) {
    QWidget *w = UIHelper::page("Lịch nghỉ", "Quản lý ngày nghỉ lễ áp dụng cho lịch đặt khám.", parent);
    auto *layout = qobject_cast<QVBoxLayout*>(w->layout());

    auto *actions = new QHBoxLayout;
    auto *add = UIHelper::button("＋ Thêm ngày nghỉ");
    auto *edit = UIHelper::button("Sửa lịch nghỉ", "secondary");
    auto *remove = UIHelper::button("Xóa ngày nghỉ", "danger");

    if (mainWindow && !mainWindow->hasPermission("Per_5")) {
        add->hide();
        edit->hide();
        remove->hide();
    }

    actions->addWidget(add);
    actions->addWidget(edit);
    actions->addWidget(remove);
    actions->addStretch();
    layout->addLayout(actions);

    auto *t = UIHelper::table({"NGÀY NGHỈ", "GHI CHÚ"});
    layout->addWidget(t, 1);

    auto fill = [t] {
        t->setRowCount(0);
        auto list = AppointmentController::loadHolidays("Config/Lichnghile.txt");
        for (const auto &item : list) {
            int row = t->rowCount();
            t->insertRow(row);
            UIHelper::cell(t, row, 0, item.date, Qt::AlignCenter);
            UIHelper::cell(t, row, 1, item.note);
        }
    };

    auto persist = [t] {
        QList<HolidayEntry> list;
        for (int r = 0; r < t->rowCount(); ++r) {
            list.append({t->item(r, 0)->text(), t->item(r, 1)->text()});
        }
        AppointmentController::saveHolidays("Config/Lichnghile.txt", list);
    };

    auto editHoliday = [this, t, fill, persist](int row, bool isNew) {
        QString oldDate = isNew ? QString() : t->item(row, 0)->text();
        QString oldNote = isNew ? QString() : t->item(row, 1)->text();

        QDialog dialog(mainWindow);
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

        auto *save = UIHelper::button("Lưu");
        v->addWidget(save);

        connect(save, &QPushButton::clicked, &dialog, [&dialog, t, date, note, row, isNew, fill, persist] {
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

    connect(add, &QPushButton::clicked, w, [editHoliday] { editHoliday(0, true); });
    connect(edit, &QPushButton::clicked, w, [t, editHoliday] {
        int r = t->currentRow();
        if (r >= 0) editHoliday(r, false);
    });
    connect(remove, &QPushButton::clicked, w, [this, t, fill, persist] {
        int r = t->currentRow();
        if (r < 0) return;
        if (QMessageBox::question(mainWindow, "Xóa ngày nghỉ", "Xóa ngày nghỉ này khỏi lịch?") != QMessageBox::Yes) {
            return;
        }
        t->removeRow(r);
        persist();
        fill();
    });

    fill();
    return w;
}


void AppointmentView::showHolidayManagementDialog(QWidget *parent) {
    if (!mainWindow) return;

    QDialog dialog(parent ? parent : mainWindow);
    dialog.setWindowTitle("Quản lý lịch nghỉ lễ");
    dialog.setMinimumWidth(660);
    dialog.setMinimumHeight(460);
    dialog.setStyleSheet(R"(
        QDialog { background-color: #ffffff; border-radius: 12px; }
        QLabel#dialogTitle { font-size: 18px; font-weight: 750; color: #0f172a; }
        QLabel#dialogSubtitle { font-size: 13px; color: #64748b; }
        QTableWidget {
            background-color: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 10px;
            gridline-color: transparent;
        }
        QHeaderView { border: none; background: transparent; }
        QHeaderView::section {
            background: #f8fafc;
            color: #475569;
            font-weight: 700;
            font-size: 11.5px;
            padding: 10px 14px;
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
        }
        QTableWidget::item:hover { background: #f0fdfa; }
    )");

    auto *rootLayout = new QVBoxLayout(&dialog);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    // Header layout
    auto *headerLayout = new QHBoxLayout;
    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(3);
    auto *title = new QLabel("Quản lý lịch nghỉ lễ", &dialog);
    title->setObjectName("dialogTitle");
    auto *sub = new QLabel("Lịch nghỉ lễ cố định và dịp Tết Nguyên Đán (nghỉ 5 ngày theo lịch Âm).", &dialog);
    sub->setObjectName("dialogSubtitle");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);
    headerLayout->addLayout(titleCol, 1);

    auto *addBtn = UIHelper::button("＋  Thêm ngày nghỉ", "primary");
    addBtn->setStyleSheet("QPushButton { background-color: #0d9488; color: white; border: none; border-radius: 8px; padding: 8px 16px; font-weight: 700; font-size: 13px; } QPushButton:hover { background-color: #0f766e; }");
    headerLayout->addWidget(addBtn, 0, Qt::AlignVCenter);
    rootLayout->addLayout(headerLayout);

    // Table
    auto *t = UIHelper::table({"NGÀY NGHỈ", "GHI CHÚ / THỜI GIAN", "THAO TÁC"});
    t->setShowGrid(false);
    t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    t->setColumnWidth(0, 160);
    t->setColumnWidth(2, 140);
    t->verticalHeader()->setDefaultSectionSize(48);
    rootLayout->addWidget(t, 1);

    auto fill = [this, t, &dialog]() {
        t->setRowCount(0);
        auto list = AppointmentController::loadHolidays("Config/Lichnghile.txt");
        for (int i = 0; i < list.size(); ++i) {
            const auto &item = list[i];
            int r = t->rowCount();
            t->insertRow(r);

            QString displayDate = item.date;
            QString displayNote = item.note;

            bool isTet = item.date.compare("Tet", Qt::CaseInsensitive) == 0;
            if (isTet) {
                displayDate = "Tết Nguyên Đán";
                int curYear = QDate::currentDate().year();
                QDate mung1 = AppointmentController::getLunarNewYear(curYear);
                displayNote = QStringLiteral("Nghỉ 5 ngày (Mùng 1 Tết: %1)").arg(mung1.toString("dd/MM/yyyy"));
            }

            auto *dateItem = new QTableWidgetItem(displayDate);
            dateItem->setTextAlignment(Qt::AlignCenter);
            if (isTet) {
                dateItem->setForeground(QColor("#b91c1c"));
                QFont font = dateItem->font();
                font.setBold(true);
                dateItem->setFont(font);
            }
            t->setItem(r, 0, dateItem);

            auto *noteItem = new QTableWidgetItem(displayNote);
            if (isTet) {
                noteItem->setForeground(QColor("#b91c1c"));
                QFont font = noteItem->font();
                font.setBold(true);
                noteItem->setFont(font);
            }
            t->setItem(r, 1, noteItem);

            // Action widget
            auto *aw = new QWidget;
            auto *al = new QHBoxLayout(aw);
            al->setContentsMargins(4, 4, 4, 4);
            al->setSpacing(6);
            al->setAlignment(Qt::AlignCenter);

            auto *editBtn = new QPushButton("Sửa");
            editBtn->setCursor(Qt::PointingHandCursor);
            editBtn->setStyleSheet("QPushButton { background: #f1f5f9; color: #0d9488; border: 1px solid #cbd5e1; border-radius: 6px; padding: 4px 10px; font-weight: 700; font-size: 12px; } QPushButton:hover { background: #e2e8f0; }");

            auto *delBtn = new QPushButton("Xóa");
            delBtn->setCursor(Qt::PointingHandCursor);
            delBtn->setStyleSheet("QPushButton { background: #fee2e2; color: #dc2626; border: 1px solid #fca5a5; border-radius: 6px; padding: 4px 10px; font-weight: 700; font-size: 12px; } QPushButton:hover { background: #fecaca; }");

            al->addWidget(editBtn);
            al->addWidget(delBtn);
            t->setCellWidget(r, 2, aw);

            QObject::connect(editBtn, &QPushButton::clicked, &dialog, [this, &dialog, i, item, isTet]() {
                QDialog editDlg(&dialog);
                editDlg.setWindowTitle("Chỉnh sửa ngày nghỉ");
                editDlg.setMinimumWidth(380);
                auto *ev = new QVBoxLayout(&editDlg);
                ev->setContentsMargins(20, 20, 20, 20);
                ev->setSpacing(14);

                auto *ef = new QFormLayout;
                auto *dateEdit = new QLineEdit(item.date);
                auto *noteEdit = new QLineEdit(item.note);
                if (isTet) {
                    dateEdit->setEnabled(false);
                    noteEdit->setPlaceholderText("Số ngày nghỉ Tết (mặc định 5)");
                }
                ef->addRow("Mã / Ngày nghỉ:", dateEdit);
                ef->addRow("Ghi chú / Số ngày:", noteEdit);
                ev->addLayout(ef);

                auto *btnBox = new QHBoxLayout;
                btnBox->addStretch();
                auto *saveB = new QPushButton("Lưu");
                saveB->setCursor(Qt::PointingHandCursor);
                saveB->setStyleSheet("QPushButton { background-color: #0d9488; color: white; border: none; border-radius: 7px; padding: 8px 18px; font-weight: 700; } QPushButton:hover { background-color: #0f766e; }");
                btnBox->addWidget(saveB);
                ev->addLayout(btnBox);

                QObject::connect(saveB, &QPushButton::clicked, &editDlg, [&editDlg, dateEdit, noteEdit, i]() {
                    auto curList = AppointmentController::loadHolidays("Config/Lichnghile.txt");
                    if (i < curList.size()) {
                        curList[i].date = dateEdit->text().trimmed();
                        curList[i].note = noteEdit->text().trimmed();
                        AppointmentController::saveHolidays("Config/Lichnghile.txt", curList);
                    }
                    editDlg.accept();
                });

                if (editDlg.exec() == QDialog::Accepted) {
                    editDlg.close();
                }
            });

            QObject::connect(delBtn, &QPushButton::clicked, &dialog, [this, &dialog, i, item]() {
                if (QMessageBox::question(&dialog, "Xác nhận xóa", QStringLiteral("Bạn có chắc chắn muốn xóa ngày nghỉ '%1' khỏi lịch?").arg(item.date)) == QMessageBox::Yes) {
                    auto curList = AppointmentController::loadHolidays("Config/Lichnghile.txt");
                    if (i < curList.size()) {
                        curList.removeAt(i);
                        AppointmentController::saveHolidays("Config/Lichnghile.txt", curList);
                    }
                }
            });
        }
    };

    QObject::connect(addBtn, &QPushButton::clicked, &dialog, [this, &dialog, fill]() {
        QDialog addDlg(&dialog);
        addDlg.setWindowTitle("Thêm ngày nghỉ mới");
        addDlg.setMinimumWidth(380);
        auto *ev = new QVBoxLayout(&addDlg);
        ev->setContentsMargins(20, 20, 20, 20);
        ev->setSpacing(14);

        auto *ef = new QFormLayout;
        auto *dateType = new QComboBox;
        dateType->addItem("Ngày lễ cố định (ngày/tháng)");
        dateType->addItem("Tết Nguyên Đán (Âm lịch - 5 ngày)");

        auto *dateEdit = new QLineEdit;
        dateEdit->setPlaceholderText("Ví dụ: 2/9, 30/4, 1/1...");
        auto *noteEdit = new QLineEdit;
        noteEdit->setPlaceholderText("Ghi chú ngày nghỉ lễ");

        ef->addRow("Loại ngày nghỉ:", dateType);
        ef->addRow("Ngày nghỉ:", dateEdit);
        ef->addRow("Ghi chú:", noteEdit);
        ev->addLayout(ef);

        QObject::connect(dateType, qOverload<int>(&QComboBox::currentIndexChanged), &addDlg, [dateEdit, noteEdit](int idx) {
            if (idx == 1) {
                dateEdit->setText("Tet");
                dateEdit->setEnabled(false);
                noteEdit->setText("5");
            } else {
                dateEdit->clear();
                dateEdit->setEnabled(true);
                noteEdit->clear();
            }
        });

        auto *btnBox = new QHBoxLayout;
        btnBox->addStretch();
        auto *saveB = new QPushButton("Thêm");
        saveB->setCursor(Qt::PointingHandCursor);
        saveB->setStyleSheet("QPushButton { background-color: #0d9488; color: white; border: none; border-radius: 7px; padding: 8px 18px; font-weight: 700; } QPushButton:hover { background-color: #0f766e; }");
        btnBox->addWidget(saveB);
        ev->addLayout(btnBox);

        QObject::connect(saveB, &QPushButton::clicked, &addDlg, [&addDlg, dateEdit, noteEdit]() {
            if (dateEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(&addDlg, "Thiếu thông tin", "Vui lòng nhập ngày nghỉ.");
                return;
            }
            auto curList = AppointmentController::loadHolidays("Config/Lichnghile.txt");
            curList.append({dateEdit->text().trimmed(), noteEdit->text().trimmed()});
            AppointmentController::saveHolidays("Config/Lichnghile.txt", curList);
            addDlg.accept();
        });

        if (addDlg.exec() == QDialog::Accepted) {
            fill();
        }
    });

    fill();
    dialog.exec();
}
