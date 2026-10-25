#ifndef STATISTICVIEW_H
#define STATISTICVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QProgressBar>

class MainWindow;

class StatisticView : public QWidget {
    Q_OBJECT
public:
    explicit StatisticView(MainWindow *mainWindow, QWidget *parent = nullptr);
    void refresh();

private:
    MainWindow *mainWindow = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *contentWidget = nullptr;
    QVBoxLayout *contentLayout = nullptr;

    void setupUI();
};

#endif // STATISTICVIEW_H
