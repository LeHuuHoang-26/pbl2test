#include "UIHelper.h"
#include <QDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QTableView>
#include <QHeaderView>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <QCalendarWidget>
#include <QLocale>
#include <QToolButton>
#include <QTextCharFormat>
#include <QMenu>
#include <QSpinBox>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {

class TableColumnScaler : public QObject {
public:
    TableColumnScaler(QTableWidget *table, const QVector<double> &ratios, int fixedLastColWidth, QObject *parent = nullptr)
        : QObject(parent), m_table(table), m_ratios(ratios), m_fixedLast(fixedLastColWidth) {
        if (m_table) {
            m_table->installEventFilter(this);
            if (auto *vp = m_table->viewport()) {
                vp->installEventFilter(this);
            }
            if (auto *header = m_table->horizontalHeader()) {
                header->installEventFilter(this);
            }
        }
    }

    void scale() {
        if (!m_table || m_isScaling) return;
        m_isScaling = true;

        int totalW = m_table->viewport()->width();
        if (totalW <= 0) totalW = m_table->width();
        if (totalW <= 50) {
            m_isScaling = false;
            return;
        }

        int remainW = totalW - m_fixedLast;
        if (remainW <= 0) {
            m_isScaling = false;
            return;
        }

        int allocated = 0;
        int n = m_ratios.size();
        for (int i = 0; i < n; ++i) {
            int w = 0;
            if (i == n - 1) {
                w = remainW - allocated;
            } else {
                w = static_cast<int>(remainW * m_ratios[i]);
                allocated += w;
            }
            if (w < 20) w = 20;
            m_table->setColumnWidth(i, w);
        }
        if (m_fixedLast > 0 && n < m_table->columnCount()) {
            m_table->setColumnWidth(n, m_fixedLast);
        }
        m_isScaling = false;
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
            scale();
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QTableWidget *m_table = nullptr;
    QVector<double> m_ratios;
    int m_fixedLast = 0;
    bool m_isScaling = false;
};

class DigitsOnlyFilter : public QObject {
public:
    explicit DigitsOnlyFilter(int maxLen, QObject *parent = nullptr)
        : QObject(parent), m_maxLen(maxLen) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::FocusIn || event->type() == QEvent::Show) {
#ifdef _WIN32
            auto *w = qobject_cast<QWidget*>(obj);
            if (w) {
                HWND hwnd = (HWND)w->winId();
                if (hwnd) {
                    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
                    if ((style & ES_NUMBER) != ES_NUMBER) {
                        SetWindowLongPtr(hwnd, GWL_STYLE, style | ES_NUMBER);
                    }
                }
            }
#endif
        } else if (event->type() == QEvent::InputMethod) {
            return true; // Chặn hoàn toàn IME composition
        } else if (event->type() == QEvent::KeyPress) {
            auto *ke = static_cast<QKeyEvent*>(event);

            // Xử lý phím Backspace: Chặn phím Backspace giả lập do bộ gõ tiếng Việt (UniKey/EVKey) tự động phát sinh khi gõ chữ (ví dụ: a f, s, w, dd...)
            if (ke->key() == Qt::Key_Backspace) {
#ifdef _WIN32
                if ((GetAsyncKeyState(VK_BACK) & 0x8000) == 0) {
                    return true; // Phím Backspace vật lý không được bấm -> Đây là Backspace của bộ gõ -> Nuốt sự kiện, không xóa số!
                }
#endif
                return false; // Người dùng chủ động nhấn Backspace vật lý -> Cho phép xóa bình thường
            }

            if (ke->key() == Qt::Key_Delete) {
#ifdef _WIN32
                if ((GetAsyncKeyState(VK_DELETE) & 0x8000) == 0) {
                    return true; // Nuốt phím Delete giả lập
                }
#endif
                return false;
            }

            if (ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right ||
                ke->key() == Qt::Key_Home || ke->key() == Qt::Key_End ||
                ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Return ||
                ke->key() == Qt::Key_Enter ||
                (ke->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
                return false;
            }

            QString txt = ke->text();
            if (!txt.isEmpty()) {
                if (!txt[0].isDigit()) {
                    return true; // Chặn mọi ký tự không phải số, không cho bộ gõ xóa số phía trước
                }
                auto *le = qobject_cast<QLineEdit*>(obj);
                if (le && le->text().length() >= m_maxLen && !le->hasSelectedText()) {
                    return true; // Chặn nhập quá số ký tự quy định
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }

private:
    int m_maxLen = 10;
};

class CalendarNoTriggerFilter : public QObject {
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject *obj, QEvent *ev) override {
        if (ev->type() == QEvent::MouseButtonPress ||
            ev->type() == QEvent::MouseButtonRelease ||
            ev->type() == QEvent::MouseButtonDblClick ||
            ev->type() == QEvent::ContextMenu) {
            return true; // Nuốt sự kiện chuột hoàn toàn, không kích hoạt popup danh sách tháng
        }
        return QObject::eventFilter(obj, ev);
    }
};

class CalendarPopupAligner : public QObject {
public:
    CalendarPopupAligner(QWidget *anchor, QCalendarWidget *cal, QObject *parent = nullptr)
        : QObject(parent), m_anchor(anchor), m_cal(cal) {}

    void align() {
        if (!m_cal || !m_anchor) return;
        QWidget *popup = m_cal->window();
        if (!popup) return;

        popup->setAttribute(Qt::WA_TranslucentBackground, true);
        popup->setStyleSheet("background: transparent; border: none;");

        if (popup->layout()) {
            popup->layout()->setContentsMargins(0, 0, 0, 0);
            popup->layout()->setSpacing(0);
        }

        int targetW = m_anchor->width();
        if (targetW < 220) targetW = 220;
        int targetH = 220;

        m_cal->resize(targetW, targetH);
        m_cal->setFixedWidth(targetW);
        m_cal->setFixedHeight(targetH);
        popup->resize(targetW, targetH);
        popup->setFixedWidth(targetW);
        popup->setFixedHeight(targetH);

        if (auto *view = m_cal->findChild<QTableView*>("qt_calendar_calendarview")) {
            if (view->horizontalHeader()) {
                view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            }
        }

        // Căn lề trái của popup thẳng hàng với mép trái của ô dateEdit
        QPoint anchorGlobal = m_anchor->mapToGlobal(QPoint(0, m_anchor->height() + 2));
        int py = popup->y();
        if (py <= 10) py = anchorGlobal.y();
        popup->move(anchorGlobal.x(), py);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Show) {
            align();
            QTimer::singleShot(0, this, [this]() { align(); });
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QWidget *m_anchor = nullptr;
    QCalendarWidget *m_cal = nullptr;
};

} // namespace

QWidget *UIHelper::page(const QString &title, const QString &subtitle, QWidget *parent) {
    auto *w = new QWidget(parent);
    auto *layout = new QVBoxLayout(w);
    layout->setContentsMargins(36, 32, 36, 32);
    layout->setSpacing(20);

    auto *h = new QVBoxLayout;
    h->setSpacing(4);
    auto *t = new QLabel(title);
    t->setObjectName("pageTitle");
    auto *s = new QLabel(subtitle);
    s->setObjectName("pageSubtitle");
    h->addWidget(t);
    h->addWidget(s);
    layout->addLayout(h);
    return w;
}

QFrame *UIHelper::panel() {
    auto *f = new QFrame;
    f->setObjectName("panel");
    auto *shadow = new QGraphicsDropShadowEffect(f);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(20, 48, 65, 12));
    f->setGraphicsEffect(shadow);
    return f;
}

QPushButton *UIHelper::button(const QString &text, const QString &kind) {
    auto *b = new QPushButton(text);
    b->setObjectName(kind);
    b->setCursor(Qt::PointingHandCursor);
    b->setProperty("kind", kind);
    return b;
}

QTableWidget *UIHelper::table(const QStringList &headers) {
    auto *t = new QTableWidget;
    t->setColumnCount(headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->verticalHeader()->setVisible(false);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setShowGrid(false);
    t->setAlternatingRowColors(false);
    t->horizontalHeader()->setStretchLastSection(false);
    t->horizontalHeader()->setSectionsClickable(true);
    t->horizontalHeader()->setSortIndicatorShown(false);
    t->horizontalHeader()->setHighlightSections(false);
    t->horizontalHeader()->setCursor(Qt::ArrowCursor);
    t->setCursor(Qt::ArrowCursor);
    if (t->viewport()) {
        t->viewport()->setCursor(Qt::ArrowCursor);
    }
    t->setFocusPolicy(Qt::NoFocus);
    t->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    t->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (t->viewport()) {
        t->viewport()->setAutoFillBackground(false);
        t->viewport()->setStyleSheet("background: transparent;");
    }
    t->setStyleSheet(R"(
        QTableWidget {
            background-color: transparent;
            border: none;
            gridline-color: transparent;
            outline: none;
        }
        QHeaderView {
            border: none;
            background: transparent;
            border-top-left-radius: 9px;
            border-top-right-radius: 9px;
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
        QHeaderView::section:first {
            border-top-left-radius: 9px;
        }
        QHeaderView::section:last {
            border-top-right-radius: 9px;
        }
        QHeaderView::up-arrow, QHeaderView::down-arrow {
            image: none;
            width: 0px;
            height: 0px;
        }
        QTableWidget::item {
            background: transparent;
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
        QTableWidget QScrollBar:vertical,
        QTableWidget QScrollBar:horizontal {
            width: 0px !important;
            height: 0px !important;
            border: none;
            background: transparent;
        }
    )");
    return t;
}

QFrame *UIHelper::createTableCard(QTableWidget *table, QWidget *parent) {
    auto *card = new QFrame(parent);
    card->setObjectName("tableCard");
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet(R"(
        QFrame#tableCard {
            background-color: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 10px;
        }
    )");
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(0, 0, 0, 4);
    lay->setSpacing(0);
    if (table) {
        lay->addWidget(table);
    }
    return card;
}

void UIHelper::setupTableScaling(QTableWidget *table, const QVector<double> &ratios, int fixedLastColWidth) {
    if (!table) return;
    table->horizontalHeader()->setStretchLastSection(false);
    for (int i = 0; i < table->columnCount(); ++i) {
        table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    }
    auto *scaler = new TableColumnScaler(table, ratios, fixedLastColWidth, table);
    QTimer::singleShot(0, table, [scaler]() { scaler->scale(); });
}

void UIHelper::cell(QTableWidget *t, int row, int column, const QString &text, Qt::Alignment align) {
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(align);
    t->setItem(row, column, item);
}

QString UIHelper::formatRoom(const QString &rawRoom) {
    QString r = rawRoom.trimmed();
    if (r.isEmpty()) {
        return "";
    }
    if (r.startsWith("Room_", Qt::CaseInsensitive)) {
        return "Room " + r.mid(5).trimmed();
    }
    if (r.startsWith("Room ", Qt::CaseInsensitive)) {
        return r;
    }
    if (r.startsWith("Room", Qt::CaseInsensitive)) {
        return "Room " + r.mid(4).trimmed();
    }
    if (r.startsWith("R_", Qt::CaseInsensitive)) {
        return "Room " + r.mid(2).trimmed();
    }
    if (r.startsWith("R", Qt::CaseInsensitive) && r.length() > 1 && r[1].isDigit()) {
        return "Room " + r.mid(1).trimmed();
    }
    return "Room " + r;
}

QPixmap UIHelper::createAvatar(const QString &fullName, int size, const QColor &bgColor, const QColor &textColor) {
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

QIcon UIHelper::createEyeIcon(bool isVisible) {
    int size = 56;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(2.0, 2.0); // 28x28 logical canvas

    if (!isVisible) {
        QColor color("#94A3B8");
        QPen pen(color, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        QPainterPath eyePath;
        eyePath.moveTo(3.5, 14.0);
        eyePath.quadTo(14.0, 6.5, 24.5, 14.0);
        eyePath.quadTo(14.0, 21.5, 3.5, 14.0);
        p.drawPath(eyePath);

        p.drawEllipse(QPointF(14.0, 14.0), 3.0, 3.0);
        p.drawLine(QPointF(5.0, 5.0), QPointF(23.0, 23.0));
    } else {
        QColor color("#0D9488");
        QPen pen(color, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        QPainterPath eyePath;
        eyePath.moveTo(3.5, 14.0);
        eyePath.quadTo(14.0, 6.5, 24.5, 14.0);
        eyePath.quadTo(14.0, 21.5, 3.5, 14.0);
        p.drawPath(eyePath);

        p.setBrush(color);
        p.drawEllipse(QPointF(14.0, 14.0), 3.0, 3.0);
    }
    return QIcon(pix);
}

QIcon UIHelper::trashIcon(const QColor &color) {
    int size = 16;
    int scaleFactor = 2; // HiDPI
    auto drawTrash = [&](const QColor &c) {
        QPixmap pixmap(size * scaleFactor, size * scaleFactor);
        pixmap.fill(Qt::transparent);
        {
            QPainter p(&pixmap);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.scale(scaleFactor, scaleFactor);

            QPen pen(c, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);

            // Nắp thùng rác: Quai cầm (Handle)
            p.drawRoundedRect(QRectF(size * 0.35, size * 0.08, size * 0.30, size * 0.12), 0.8, 0.8);

            // Thanh ngang nắp (Lid bar)
            p.drawLine(QPointF(size * 0.15, size * 0.24), QPointF(size * 0.85, size * 0.24));

            // Thân thùng rác (Body)
            QPainterPath bodyPath;
            bodyPath.moveTo(size * 0.24, size * 0.30);
            bodyPath.lineTo(size * 0.29, size * 0.85);
            bodyPath.quadTo(size * 0.30, size * 0.92, size * 0.38, size * 0.92);
            bodyPath.lineTo(size * 0.62, size * 0.92);
            bodyPath.quadTo(size * 0.70, size * 0.92, size * 0.71, size * 0.85);
            bodyPath.lineTo(size * 0.76, size * 0.30);
            p.drawPath(bodyPath);

            // 2 vạch đứng trang trí bên trong thân thùng
            p.drawLine(QPointF(size * 0.42, size * 0.42), QPointF(size * 0.43, size * 0.78));
            p.drawLine(QPointF(size * 0.58, size * 0.42), QPointF(size * 0.57, size * 0.78));
        }
        pixmap.setDevicePixelRatio(scaleFactor);
        return pixmap;
    };

    QIcon icon;
    icon.addPixmap(drawTrash(color), QIcon::Normal);
    icon.addPixmap(drawTrash(QColor("#b91c1c")), QIcon::Active);
    return icon;
}

QPair<QWidget*, QPushButton*> UIHelper::createActionCell(const QIcon &icon, const QString &tooltip) {
    auto *widget = new QWidget;
    auto *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    auto *btn = new QPushButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(16, 16));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setToolTip(tooltip);
    btn->setFixedSize(26, 26);
    btn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: #fee2e2;
            border-color: #fca5a5;
        }
        QPushButton:pressed {
            background-color: #fecaca;
        }
    )");

    layout->addWidget(btn);
    return qMakePair(widget, btn);
}

QString UIHelper::dialogFormStyle() {
    return QStringLiteral(R"(
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
        QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled {
            background: #f1f5f9;
            color: #64748b;
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
        QComboBox QAbstractItemView,
        QComboBox QAbstractItemView:viewport,
        QListView {
            background-color: #ffffff;
            color: #0f172a;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            outline: none;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item, QListView::item {
            min-height: 28px;
            padding: 4px 10px;
            border-radius: 6px;
            color: #0f172a;
            background-color: #ffffff;
        }
        QComboBox QAbstractItemView::item:hover, QListView::item:hover {
            background-color: #f0fdfa;
            color: #0f766e;
        }
        QComboBox QAbstractItemView::item:selected, QListView::item:selected {
            background-color: #0d9488;
            color: #ffffff;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            width: 0px;
            height: 0px;
            background: transparent;
            border: none;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: transparent;
            border: none;
        }
        QScrollBar::add-line, QScrollBar::sub-line,
        QScrollBar::add-page, QScrollBar::sub-page {
            background: transparent;
            border: none;
            width: 0px;
            height: 0px;
        }
        QMessageBox {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            border-radius: 12px;
        }
        QMessageBox QLabel {
            color: #0f172a;
            font-size: 13.5px;
            background: transparent;
        }
        QMessageBox QPushButton {
            background-color: #f1f5f9;
            color: #334155;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            padding: 8px 18px;
            font-size: 13px;
            font-weight: 600;
            min-width: 80px;
            min-height: 22px;
        }
        QMessageBox QPushButton:hover {
            background-color: #e2e8f0;
            color: #0f172a;
            border-color: #94a3b8;
        }
        QMessageBox QPushButton:pressed {
            background-color: #cbd5e1;
        }
        QMessageBox QPushButton:default {
            background-color: #0d9488;
            color: #ffffff;
            border: none;
            font-weight: 700;
        }
        QMessageBox QPushButton:default:hover {
            background-color: #0f766e;
        }
    )");
}

QHBoxLayout *UIHelper::createDialogHeader(const QString &iconText, const QString &titleText, const QString &subtitleText) {
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(12);

    auto *icon = new QLabel(iconText);
    icon->setFixedSize(46, 46);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: #f0fdfa; border: 1.5px solid #99f6e4; border-radius: 12px; font-size: 22px;");

    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    auto *title = new QLabel(titleText);
    title->setStyleSheet("font-size: 18px; font-weight: 800; color: #0f172a;");
    auto *sub = new QLabel(subtitleText);
    sub->setStyleSheet("font-size: 12.5px; color: #64748b;");
    titleCol->addWidget(title);
    titleCol->addWidget(sub);

    headerBox->addWidget(icon);
    headerBox->addLayout(titleCol, 1);
    return headerBox;
}

void UIHelper::applyCalendarStyle(QCalendarWidget *cal) {
    if (!cal) return;
    // Cài đặt ngôn ngữ tiếng Anh: Sun, Mon, Tue, Wed, Thu, Fri, Sat và Tháng bằng tiếng Anh (October, ...)
    cal->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    cal->setFirstDayOfWeek(Qt::Sunday);
    cal->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    // 1. Tối ưu thanh điều hướng: Bỏ spacer thừa giữa Month và Year, gom gọn vào giữa
    if (auto *navBar = cal->findChild<QWidget*>("qt_calendar_navigationbar")) {
        if (auto *lay = qobject_cast<QHBoxLayout*>(navBar->layout())) {
            // Layout chuẩn của Qt: prev(0), spacer(1), month(2), spacer(3), year(4), spacer(5), next(6)
            if (lay->count() == 7) {
                auto *midSpacer = lay->itemAt(3);
                if (midSpacer && midSpacer->spacerItem()) {
                    lay->removeItem(midSpacer);
                    delete midSpacer;
                }
            }
            lay->setContentsMargins(6, 2, 6, 2);
            lay->setSpacing(4);
        }
    }

    // 2. Cấu hình nút chuyển tháng hiển thị rõ ràng ký tự ‹ và ›
    if (auto *prevBtn = cal->findChild<QToolButton*>("qt_calendar_prevmonth")) {
        prevBtn->setIcon(QIcon());
        prevBtn->setText(QString::fromUtf8("‹"));
        prevBtn->setCursor(Qt::PointingHandCursor);
    }
    if (auto *nextBtn = cal->findChild<QToolButton*>("qt_calendar_nextmonth")) {
        nextBtn->setIcon(QIcon());
        nextBtn->setText(QString::fromUtf8("›"));
        nextBtn->setCursor(Qt::PointingHandCursor);
    }

    // 3. Bỏ trigger phần tháng (bỏ menu hiển thị 12 tháng) & bỏ spinbox năm
    if (auto *monthBtn = cal->findChild<QToolButton*>("qt_calendar_monthbutton")) {
        monthBtn->setMenu(nullptr);
        monthBtn->setPopupMode(QToolButton::DelayedPopup);
        monthBtn->setCursor(Qt::ArrowCursor);
        monthBtn->setFocusPolicy(Qt::NoFocus);
        monthBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        monthBtn->installEventFilter(new CalendarNoTriggerFilter(monthBtn));
    }
    if (auto *monthMenu = cal->findChild<QMenu*>("qt_calendar_monthmenu")) {
        monthMenu->clear();
        monthMenu->setEnabled(false);
        monthMenu->hide();
    }
    if (auto *yearBtn = cal->findChild<QToolButton*>("qt_calendar_yearbutton")) {
        yearBtn->disconnect(SIGNAL(clicked()));
        yearBtn->setCursor(Qt::ArrowCursor);
        yearBtn->setFocusPolicy(Qt::NoFocus);
        yearBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        yearBtn->installEventFilter(new CalendarNoTriggerFilter(yearBtn));
    }
    if (auto *yearEdit = cal->findChild<QSpinBox*>("qt_calendar_yearedit")) {
        yearEdit->hide();
        yearEdit->setEnabled(false);
    }

    // 4. Chuẩn hóa tiêu đề thứ trong tuần (Sun..Sat) sang màu slate thanh lịch, bỏ màu đỏ gắt
    QTextCharFormat weekdayFmt;
    weekdayFmt.setForeground(QColor("#475569"));
    weekdayFmt.setFontWeight(QFont::DemiBold);
    weekdayFmt.setFontPointSize(8.5);
    for (int dow = 1; dow <= 7; ++dow) {
        cal->setWeekdayTextFormat(static_cast<Qt::DayOfWeek>(dow), weekdayFmt);
    }

    // 5. Cấu hình bảng ngày: kéo dãn đều 7 cột theo bề rộng của lịch
    if (auto *view = cal->findChild<QTableView*>("qt_calendar_calendarview")) {
        if (view->horizontalHeader()) {
            view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        }
    }

    cal->setStyleSheet(R"(
        QCalendarWidget {
            background-color: #ffffff;
            border: 1.5px solid #cbd5e1;
            border-radius: 8px;
            font-family: 'Segoe UI', -apple-system, sans-serif;
            font-size: 11px;
        }
        QCalendarWidget QWidget {
            alternate-background-color: #ffffff;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background-color: #f8fafc;
            border-top-left-radius: 7px;
            border-top-right-radius: 7px;
            border-bottom: 1px solid #e2e8f0;
            min-height: 30px;
            max-height: 30px;
        }
        QCalendarWidget QToolButton {
            color: #0f172a;
            font-weight: 700;
            font-size: 11px;
            background: transparent;
            border: none;
            padding: 2px 4px;
            margin: 0px;
        }
        QCalendarWidget QToolButton#qt_calendar_prevmonth,
        QCalendarWidget QToolButton#qt_calendar_nextmonth {
            font-size: 16px;
            font-weight: bold;
            color: #0d9488;
            min-width: 24px;
            max-width: 24px;
            min-height: 22px;
            max-height: 22px;
            border-radius: 4px;
        }
        QCalendarWidget QToolButton#qt_calendar_prevmonth:hover,
        QCalendarWidget QToolButton#qt_calendar_nextmonth:hover {
            background-color: #e6fffa;
            color: #0f766e;
        }
        QCalendarWidget QToolButton#qt_calendar_monthbutton,
        QCalendarWidget QToolButton#qt_calendar_yearbutton {
            color: #0f172a;
            font-weight: 700;
            font-size: 12px;
            padding: 0px 3px;
            background: transparent;
            border: none;
        }
        QCalendarWidget QToolButton#qt_calendar_monthbutton:hover,
        QCalendarWidget QToolButton#qt_calendar_yearbutton:hover {
            background: transparent;
            color: #0f172a;
        }
        QCalendarWidget QToolButton::menu-indicator,
        QCalendarWidget QToolButton#qt_calendar_monthbutton::menu-indicator {
            image: none;
            width: 0px;
            height: 0px;
        }
        QCalendarWidget QSpinBox#qt_calendar_yearedit {
            display: none;
            width: 0px;
            height: 0px;
            border: none;
        }
        QCalendarWidget QTableView {
            background-color: #ffffff;
            selection-background-color: #0d9488;
            selection-color: #ffffff;
            color: #1e293b;
            font-size: 11px;
            font-weight: 600;
            outline: none;
            border: none;
            gridline-color: transparent;
        }
        QCalendarWidget QTableView QHeaderView::section {
            background-color: #f8fafc;
            color: #475569;
            font-weight: 700;
            font-size: 9.5px;
            border: none;
            border-bottom: 1px solid #e2e8f0;
            padding: 2px 0px;
            margin: 0px;
        }
        QCalendarWidget QTableView::item {
            border-radius: 4px;
            padding: 1px;
            margin: 1px;
            color: #1e293b;
            background-color: #ffffff;
        }
        QCalendarWidget QTableView::item:hover {
            background-color: #ccfbf1;
            color: #0f766e;
        }
        QCalendarWidget QTableView::item:selected {
            background-color: #0d9488;
            color: #ffffff;
            font-weight: bold;
        }
        QCalendarWidget QTableView::item:disabled {
            color: #94a3b8;
            background-color: #ffffff;
        }
        QCalendarWidget QWidget#qt_calendar_calendarview {
            background-color: #ffffff;
            border-bottom-left-radius: 7px;
            border-bottom-right-radius: 7px;
        }
    )");
}

void UIHelper::setupCalendarPopup(QDateTimeEdit *dateEdit, const QDate &minDate) {
    if (!dateEdit) return;
    dateEdit->setCalendarPopup(true);
    if (auto *cal = dateEdit->calendarWidget()) {
        UIHelper::applyCalendarStyle(cal);
        if (minDate.isValid()) {
            cal->setMinimumDate(minDate);
        }
        auto *aligner = new CalendarPopupAligner(dateEdit, cal, dateEdit);
        cal->installEventFilter(aligner);
        if (auto *pop = cal->window()) {
            pop->installEventFilter(aligner);
        }
    }
}

void UIHelper::setupPhoneInput(QLineEdit *lineEdit) {
    if (!lineEdit) return;
    lineEdit->setMaxLength(10);
    lineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    lineEdit->setInputMethodHints(Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText);
    lineEdit->installEventFilter(new DigitsOnlyFilter(10, lineEdit));

#ifdef _WIN32
    HWND hwnd = (HWND)lineEdit->winId();
    if (hwnd) {
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        SetWindowLongPtr(hwnd, GWL_STYLE, style | ES_NUMBER);
    }
#endif

    QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [lineEdit](const QString &text) {
        QString digits;
        for (const QChar &ch : text) {
            if (ch.isDigit()) {
                digits.append(ch);
                if (digits.length() == 10) break;
            }
        }
        if (digits != text) {
            int pos = lineEdit->cursorPosition();
            lineEdit->setText(digits);
            lineEdit->setCursorPosition(qMin(pos, digits.length()));
        }
    });
}

void UIHelper::setupCccdInput(QLineEdit *lineEdit) {
    if (!lineEdit) return;
    lineEdit->setMaxLength(12);
    lineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    lineEdit->setInputMethodHints(Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText);
    lineEdit->installEventFilter(new DigitsOnlyFilter(12, lineEdit));

#ifdef _WIN32
    HWND hwnd = (HWND)lineEdit->winId();
    if (hwnd) {
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        SetWindowLongPtr(hwnd, GWL_STYLE, style | ES_NUMBER);
    }
#endif

    QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [lineEdit](const QString &text) {
        QString digits;
        for (const QChar &ch : text) {
            if (ch.isDigit()) {
                digits.append(ch);
                if (digits.length() == 12) break;
            }
        }
        if (digits != text) {
            int pos = lineEdit->cursorPosition();
            lineEdit->setText(digits);
            lineEdit->setCursorPosition(qMin(pos, digits.length()));
        }
    });
}

bool UIHelper::isValidPhone(const QString &phone) {
    QString p = phone.trimmed();
    if (p.length() != 10) return false;
    if (!p.startsWith('0')) return false;
    for (const QChar &c : p) {
        if (!c.isDigit()) return false;
    }
    return true;
}

bool UIHelper::isValidCccd(const QString &cccd) {
    QString c = cccd.trimmed();
    if (c.length() != 12) return false;
    if (!c.startsWith('0')) return false;
    for (const QChar &ch : c) {
        if (!ch.isDigit()) return false;
    }
    return true;
}

void UIHelper::showWarning(QWidget *parent, const QString &title, const QString &message) {
    QMessageBox box(QMessageBox::Warning, title, message, QMessageBox::Ok, parent);
    box.setStyleSheet(R"(
        QMessageBox {
            background-color: #ffffff;
        }
        QLabel {
            color: #0f172a;
            font-size: 13.5px;
            background: transparent;
        }
        QPushButton {
            background-color: #0d9488;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 6px 18px;
            font-size: 13px;
            font-weight: 700;
            min-width: 75px;
            min-height: 22px;
        }
        QPushButton:hover {
            background-color: #0f766e;
        }
    )");
    box.exec();
}

void UIHelper::showInfo(QWidget *parent, const QString &title, const QString &message) {
    QMessageBox box(QMessageBox::Information, title, message, QMessageBox::Ok, parent);
    box.setStyleSheet(R"(
        QMessageBox {
            background-color: #ffffff;
        }
        QLabel {
            color: #0f172a;
            font-size: 13.5px;
            background: transparent;
        }
        QPushButton {
            background-color: #0d9488;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 6px 18px;
            font-size: 13px;
            font-weight: 700;
            min-width: 75px;
            min-height: 22px;
        }
        QPushButton:hover {
            background-color: #0f766e;
        }
    )");
    box.exec();
}

void UIHelper::showSuccess(QWidget *parent, const QString &title, const QString &message) {
    showInfo(parent, title, message);
}

bool UIHelper::confirmDelete(QWidget *parent, const QString &title, const QString &message,
                             const QString &confirmText, const QString &cancelText) {
    QMessageBox box(QMessageBox::Question, title, message, QMessageBox::NoButton, parent);
    QPushButton *confirmBtn = box.addButton(confirmText, QMessageBox::AcceptRole);
    QPushButton *cancelBtn = box.addButton(cancelText, QMessageBox::RejectRole);
    box.setDefaultButton(cancelBtn);
    box.setStyleSheet(R"(
        QMessageBox {
            background-color: #ffffff;
        }
        QLabel {
            color: #0f172a;
            font-size: 13.5px;
            background: transparent;
        }
        QPushButton {
            border-radius: 6px;
            padding: 6px 18px;
            font-size: 13px;
            font-weight: 700;
            min-width: 75px;
            min-height: 22px;
        }
        QPushButton[text="Xóa"], QPushButton[text="Hủy lịch"], QPushButton[text="Hủy lịch hẹn"] {
            background-color: #dc2626;
            color: #ffffff;
            border: none;
        }
        QPushButton[text="Xóa"]:hover, QPushButton[text="Hủy lịch"]:hover, QPushButton[text="Hủy lịch hẹn"]:hover {
            background-color: #b91c1c;
        }
        QPushButton[text="Không xóa"], QPushButton[text="Không hủy"], QPushButton[text="Quay lại"] {
            background-color: #f1f5f9;
            color: #475569;
            border: 1px solid #cbd5e1;
        }
        QPushButton[text="Không xóa"]:hover, QPushButton[text="Không hủy"]:hover, QPushButton[text="Quay lại"]:hover {
            background-color: #e2e8f0;
        }
    )");
    box.exec();
    return box.clickedButton() == confirmBtn;
}

bool UIHelper::confirmWarning(QWidget *parent, const QString &title, const QString &message,
                              const QString &confirmText, const QString &cancelText) {
    QMessageBox box(QMessageBox::Warning, title, message, QMessageBox::NoButton, parent);
    QPushButton *confirmBtn = box.addButton(confirmText, QMessageBox::AcceptRole);
    QPushButton *cancelBtn = box.addButton(cancelText, QMessageBox::RejectRole);
    box.setDefaultButton(cancelBtn);
    box.setStyleSheet(R"(
        QMessageBox {
            background-color: #ffffff;
        }
        QLabel {
            color: #0f172a;
            font-size: 13.5px;
            background: transparent;
        }
        QPushButton {
            border-radius: 6px;
            padding: 6px 18px;
            font-size: 13px;
            font-weight: 700;
            min-width: 75px;
            min-height: 22px;
        }
        QPushButton[text="Tiếp tục"] {
            background-color: #d97706;
            color: #ffffff;
            border: none;
        }
        QPushButton[text="Tiếp tục"]:hover {
            background-color: #b45309;
        }
        QPushButton[text="Quay lại"] {
            background-color: #f1f5f9;
            color: #475569;
            border: 1px solid #cbd5e1;
        }
        QPushButton[text="Quay lại"]:hover {
            background-color: #e2e8f0;
        }
    )");
    box.exec();
    return box.clickedButton() == confirmBtn;
}

bool UIHelper::confirmCancel(QWidget *parent, const QString &title, const QString &message,
                             const QString &confirmText, const QString &cancelText) {
    return confirmDelete(parent, title, message, confirmText, cancelText);
}
