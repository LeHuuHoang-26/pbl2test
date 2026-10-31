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
#include <QIcon>
#include <QHBoxLayout>
#include <QVector>
#include <QDate>
#include <string>

class QCalendarWidget;
class QLineEdit;
class QDateTimeEdit;

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
    static QFrame *createTableCard(QTableWidget *table, QWidget *parent = nullptr);
    static void setupTableScaling(QTableWidget *table, const QVector<double> &ratios, int fixedLastColWidth = 0);
    static void cell(QTableWidget *t, int row, int column, const QString &text, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);
    static QString formatRoom(const QString &rawRoom);
    static QPixmap createAvatar(const QString &fullName, int size = 38, const QColor &bgColor = QColor("#0d9488"), const QColor &textColor = Qt::white);
    static QIcon createEyeIcon(bool isVisible);
    static QIcon trashIcon(const QColor &color = QColor("#ef4444"));
    static QPair<QWidget*, QPushButton*> createActionCell(const QIcon &icon, const QString &tooltip);
    static QString dialogFormStyle();
    static QHBoxLayout *createDialogHeader(const QString &iconText, const QString &titleText, const QString &subtitleText);
    static void applyCalendarStyle(QCalendarWidget *cal);
    static void setupCalendarPopup(QDateTimeEdit *dateEdit, const QDate &minDate = QDate());
    static void setupPhoneInput(QLineEdit *lineEdit);
    static void setupCccdInput(QLineEdit *lineEdit);
    static bool isValidPhone(const QString &phone);
    static bool isValidCccd(const QString &cccd);

    static bool confirmCancel(QWidget *parent, const QString &title, const QString &message,
                             const QString &confirmText = "Hủy lịch hẹn", const QString &cancelText = "Không hủy");
    static bool confirmDelete(QWidget *parent, const QString &title, const QString &message,
                             const QString &confirmText = "Xóa", const QString &cancelText = "Không xóa");
    static bool confirmWarning(QWidget *parent, const QString &title, const QString &message,
                              const QString &confirmText = "Tiếp tục", const QString &cancelText = "Quay lại");
    static void showWarning(QWidget *parent, const QString &title, const QString &message);
    static void showInfo(QWidget *parent, const QString &title, const QString &message);
    static void showSuccess(QWidget *parent, const QString &title, const QString &message);
};

#endif // UIHELPER_H
