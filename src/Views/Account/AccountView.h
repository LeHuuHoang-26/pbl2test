#ifndef ACCOUNTVIEW_H
#define ACCOUNTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <string>

class MainWindow;

class AccountView : public QWidget {
    Q_OBJECT

public:
    explicit AccountView(MainWindow *mainWindow, QWidget *parent = nullptr);
    void refresh();
    void resetToFirstPage();

protected:
    void showEvent(QShowEvent *event) override;

private:
    MainWindow *mainWindow = nullptr;
    QPushButton *addBtn = nullptr;
    QTableWidget *table = nullptr;

    QLabel *totalLabel = nullptr;
    QLabel *pageStatus = nullptr;
    QPushButton *prevBtn = nullptr;
    QPushButton *nextBtn = nullptr;

    int currentPage = 0;
    static const int PAGE_SIZE = 8;

    void setupUI();
    void addManagerDialog();
    void editManagerDialog(const std::string &managerId);
};

#endif // ACCOUNTVIEW_H
