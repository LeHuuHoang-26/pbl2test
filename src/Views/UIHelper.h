#ifndef UIHELPER_H
#define UIHELPER_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <string>

class UIHelper {
public:
    static inline QString qs(const std::string &s) {
        return QString::fromUtf8(s.c_str());
    }

    static inline std::string ss(const QString &s) {
        return s.toUtf8().toStdString();
    }

    static QWidget *page(const QString &title, const QString &subtitle, QWidget *parent = nullptr);
    static QFrame *panel();
    static QPushButton *button(const QString &text, const QString &kind = "primary");
    static QTableWidget *table(const QStringList &headers);
    static void cell(QTableWidget *t, int row, int column, const QString &text, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);
    static QString formatRoom(const QString &rawRoom);
    static QPixmap createAvatar(const QString &fullName, int size = 38, const QColor &bgColor = QColor("#0d9488"), const QColor &textColor = Qt::white);
};

#endif // UIHELPER_H
