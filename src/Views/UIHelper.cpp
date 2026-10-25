#include "UIHelper.h"
#include <QGraphicsDropShadowEffect>

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
    t->horizontalHeader()->setStretchLastSection(true);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->setFocusPolicy(Qt::NoFocus);
    t->setStyleSheet(
        "QTableWidget { background-color: #ffffff; border: 1.5px solid #cbd5e1; border-radius: 10px; gridline-color: transparent; } "
        "QHeaderView { border: none; background: transparent; } "
        "QHeaderView::section { background: #f8fafc; color: #475569; font-weight: 700; font-size: 11.5px; padding: 10px 14px; border: none; border-bottom: 2px solid #e2e8f0; border-left: none; border-right: none; } "
        "QTableWidget::item { background: #ffffff; border: none; border-bottom: 1px solid #f1f5f9; border-left: none; border-right: none; } "
        "QTableWidget::item:hover { background: #f0fdfa; }"
    );
    return t;
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
