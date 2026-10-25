#include "StatisticView.h"
#include "MainWindow.h"
#include "StatisticController.h"
#include "UIHelper.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QProgressBar>
#include <QLocale>
#include <QDate>
#include <QTime>
#include <QPainter>
#include <QPainterPath>

namespace {

// Widget vẽ thanh phân đoạn nhiều màu cho trạng thái lịch hẹn
class SegmentedStatusBar : public QWidget {
public:
    int completed = 0;
    int pending = 0;
    int delayed = 0;

    explicit SegmentedStatusBar(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(16);
    }

    void setCounts(int c, int p, int d) {
        completed = c;
        pending = p;
        delayed = d;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int total = completed + pending + delayed;
        if (total <= 0) {
            painter.setBrush(QColor("#e2e8f0"));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect(), 8, 8);
            return;
        }

        QPainterPath clipPath;
        clipPath.addRoundedRect(QRectF(rect()), 8, 8);
        painter.setClipPath(clipPath);

        double w = width();
        double x = 0.0;

        auto drawSegment = [&](int count, const QColor &color) {
            if (count <= 0) return;
            double segW = (static_cast<double>(count) / total) * w;
            painter.fillRect(QRectF(x, 0, segW, height()), color);
            x += segW;
        };

        drawSegment(completed, QColor("#0d9488")); // Teal (Đã khám)
        drawSegment(pending, QColor("#ea580c"));   // Orange (Chưa khám)
        drawSegment(delayed, QColor("#f59e0b"));   // Amber (Trễ hẹn)
    }
};

// Widget vẽ biểu đồ tròn (Donut chart) cho phân bổ độ tuổi bệnh nhân
class PatientDonutChart : public QWidget {
public:
    int c1 = 0, c2 = 0, c3 = 0, c4 = 0; // <18, 18-45, 46-60, >60

    explicit PatientDonutChart(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(115, 115);
    }

    void setData(int under18, int from18to45, int from46to60, int over60) {
        c1 = under18;
        c2 = from18to45;
        c3 = from46to60;
        c4 = over60;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int total = c1 + c2 + c3 + c4;
        QRectF outerRect(6, 6, width() - 12, height() - 12);
        QRectF innerRect(25, 25, width() - 50, height() - 50);

        if (total <= 0) {
            painter.setBrush(QColor("#e2e8f0"));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(outerRect);
            painter.setBrush(QColor("#ffffff"));
            painter.drawEllipse(innerRect);

            painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
            painter.setPen(QColor("#94a3b8"));
            painter.drawText(innerRect, Qt::AlignCenter, "0");
            return;
        }

        const int counts[4] = {c1, c2, c3, c4};
        const QColor colors[4] = {
            QColor("#38bdf8"), // <18 Sky blue
            QColor("#0d9488"), // 18-45 Teal
            QColor("#f59e0b"), // 46-60 Amber
            QColor("#8b5cf6")  // >60 Purple
        };

        int startAngle = 90 * 16;
        for (int i = 0; i < 4; ++i) {
            if (counts[i] <= 0) continue;
            int spanAngle = static_cast<int>(- (static_cast<double>(counts[i]) / total) * 360.0 * 16.0);
            painter.setBrush(colors[i]);
            painter.setPen(Qt::NoPen);
            painter.drawPie(outerRect, startAngle, spanAngle);
            startAngle += spanAngle;
        }

        // Lỗ rỗng ở giữa tạo hình Donut
        painter.setBrush(QColor("#ffffff"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(innerRect);

        // Hiển thị tổng số bệnh nhân ở giữa Donut
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.setPen(QColor("#0f172a"));
        QRectF textTopRect(innerRect.x(), innerRect.y() + 8, innerRect.width(), 20);
        painter.drawText(textTopRect, Qt::AlignCenter, QString::number(total));

        painter.setFont(QFont("Segoe UI", 8, QFont::Normal));
        painter.setPen(QColor("#64748b"));
        QRectF textSubRect(innerRect.x(), innerRect.y() + 27, innerRect.width(), 16);
        painter.drawText(textSubRect, Qt::AlignCenter, "Bệnh nhân");
    }
};

// Widget vẽ biểu đồ cột 7 ngày gần nhất
class WeeklyTrendChart : public QWidget {
public:
    int counts[7] = {0};
    QString dates[7];

    explicit WeeklyTrendChart(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(130);
    }

    void setData(const int c[7], const QString d[7]) {
        for (int i = 0; i < 7; ++i) {
            counts[i] = c[i];
            dates[i] = d[i];
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int maxVal = 1;
        for (int i = 0; i < 7; ++i) {
            if (counts[i] > maxVal) maxVal = counts[i];
        }

        int w = width();
        int h = height();
        if (w <= 32 || h <= 50) return;
        int colWidth = (w - 32) / 7;
        int barWidth = 32;
        int maxBarH = h - 50;

        for (int i = 0; i < 7; ++i) {
            int centerX = 16 + i * colWidth + colWidth / 2;
            int barH = static_cast<int>((static_cast<double>(counts[i]) / maxVal) * maxBarH);
            if (barH < 6 && counts[i] > 0) barH = 6;
            if (counts[i] == 0) barH = 4;

            int barX = centerX - barWidth / 2;
            int barY = h - 28 - barH;

            // Nền cột
            QRectF barRect(barX, barY, barWidth, barH);
            QColor colColor = (i == 6) ? QColor("#0d9488") : QColor("#99f6e4");
            if (counts[i] == 0) colColor = QColor("#e2e8f0");

            painter.setBrush(colColor);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(barRect, 5, 5);

            // Số lượng ca trên đầu cột
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
            painter.setPen(counts[i] > 0 ? QColor("#0f766e") : QColor("#94a3b8"));
            painter.drawText(QRect(barX - 10, barY - 18, barWidth + 20, 16), Qt::AlignCenter, QString::number(counts[i]));

            // Ngày tháng bên dưới cột
            painter.setFont(QFont("Segoe UI", 9, i == 6 ? QFont::Bold : QFont::Normal));
            painter.setPen(i == 6 ? QColor("#0d9488") : QColor("#64748b"));
            painter.drawText(QRect(barX - 15, h - 22, barWidth + 30, 20), Qt::AlignCenter, dates[i]);
        }
    }
};

} // namespace

StatisticView::StatisticView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), mainWindow(mainWindow) {
    setupUI();
    refresh();
}

void StatisticView::setupUI() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #f8fafc; border: none; }");

    rootLayout->addWidget(scrollArea);
}

void StatisticView::refresh() {
    if (!mainWindow || !scrollArea) return;

    // Xóa nội dung cũ một cách an toàn thông qua cấu trúc QObject
    if (contentWidget) {
        delete contentWidget;
        contentWidget = nullptr;
        contentLayout = nullptr;
    }

    contentWidget = new QWidget;
    contentWidget->setStyleSheet("background-color: #f8fafc;");
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(36, 28, 36, 32);
    contentLayout->setSpacing(20);
    scrollArea->setWidget(contentWidget);

    auto *patients = mainWindow->getPatients();
    auto *doctors = mainWindow->getDoctors();
    auto *appts = mainWindow->getAppointments();

    QDate curDate = QDate::currentDate();
    QTime curTime = QTime::currentTime();
    int curDay = curDate.day();
    int curMonth = curDate.month();
    int curYear = curDate.year();
    int curHour = curTime.hour();
    int curMin = curTime.minute();

    // 1. Dữ liệu tính toán từ StatisticController
    double totalRev = StatisticController::totalRevenue(appts, doctors);
    double todayRev = StatisticController::todayRevenue(appts, doctors, curDay, curMonth, curYear);
    int compDone = 0, compTotalUntilNow = 0;
    double compRate = StatisticController::completionRate(appts, curDay, curMonth, curYear, curHour, curMin, compDone, compTotalUntilNow);

    int completed = 0, pending = 0, delayed = 0;
    StatisticController::appointmentStatusCounts(appts, completed, pending, delayed, curDay, curMonth, curYear, curHour, curMin);
    int totalAppts = completed + pending + delayed;

    int totalToday = 0, completedToday = 0, pendingToday = 0;
    StatisticController::todayAppointmentCounts(appts, curDay, curMonth, curYear, totalToday, completedToday, pendingToday);

    int male = 0, female = 0;
    StatisticController::patientGenderCounts(patients, male, female);
    int totalPatients = male + female;

    int under18 = 0, from18to45 = 0, from46to60 = 0, over60 = 0;
    StatisticController::patientAgeDistribution(patients, under18, from18to45, from46to60, over60);

    // 2. Header Tiêu đề
    auto *headerBox = new QVBoxLayout;
    headerBox->setSpacing(4);
    auto *title = new QLabel("Thống kê & Báo cáo");
    title->setObjectName("pageTitle");
    auto *subtitle = new QLabel("Phân tích tổng hợp hiệu suất khám bệnh, doanh thu và đặc điểm nhân khẩu học bệnh nhân.");
    subtitle->setObjectName("pageSubtitle");
    headerBox->addWidget(title);
    headerBox->addWidget(subtitle);
    contentLayout->addLayout(headerBox);

    // 3. Hàng 4 Thẻ chỉ số nổi bật (Top KPI Cards)
    auto *kpiGrid = new QGridLayout;
    kpiGrid->setSpacing(16);

    auto makeKpiCard = [](const QString &valText, const QString &label, const QString &subText, const QString &color) {
        auto *card = new QFrame;
        card->setStyleSheet(QStringLiteral(
            "QFrame { "
            "    background: #ffffff; "
            "    border: 1px solid #e2e8f0; "
            "    border-left: 5px solid %1; "
            "    border-radius: 12px; "
            "}").arg(color));
        auto *v = new QVBoxLayout(card);
        v->setContentsMargins(20, 16, 20, 16);
        v->setSpacing(5);

        auto *valLbl = new QLabel(valText);
        valLbl->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: 800; color: %1;").arg(color));

        auto *nameLbl = new QLabel(label);
        nameLbl->setStyleSheet("font-size: 11px; font-weight: 700; color: #475569; letter-spacing: 0.5px;");

        auto *subLbl = new QLabel(subText);
        subLbl->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");

        v->addWidget(valLbl);
        v->addWidget(nameLbl);
        v->addWidget(subLbl);
        return card;
    };

    QString revStr = QLocale(QLocale::Vietnamese).toString(totalRev, 'f', 0) + " đ";
    QString todayRevStr = QStringLiteral("Hôm nay: %1 đ").arg(QLocale(QLocale::Vietnamese).toString(todayRev, 'f', 0));
    kpiGrid->addWidget(makeKpiCard(revStr, "DOANH THU THỰC TẾ", todayRevStr, "#0d9488"), 0, 0);

    QString compStr = QString::number(compRate, 'f', 1) + "%";
    QString compSub = QStringLiteral("%1 / %2 ca đến nay đã hoàn thành").arg(compDone).arg(compTotalUntilNow);
    kpiGrid->addWidget(makeKpiCard(compStr, "TỈ LỆ HOÀN THÀNH", compSub, "#0284c7"), 0, 1);

    QString todayStr = QStringLiteral("%1 ca").arg(totalToday);
    QString todaySub = QStringLiteral("%1 chờ khám · %2 đã xong").arg(pendingToday).arg(completedToday);
    kpiGrid->addWidget(makeKpiCard(todayStr, "LỊCH KHÁM HÔM NAY", todaySub, "#6366f1"), 0, 2);

    QString pendStr = QStringLiteral("%1 ca").arg(pending);
    kpiGrid->addWidget(makeKpiCard(pendStr, "ĐANG CHỜ TIẾP NHẬN", "Lịch hẹn sắp tới chưa khám", "#ea580c"), 0, 3);

    contentLayout->addLayout(kpiGrid);

    // 4. Hàng 2 Card: Trạng thái lịch khám & Nhân khẩu học bệnh nhân
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(16);

    // Card 1: Trạng thái lịch khám
    auto *statusCard = UIHelper::panel();
    statusCard->setStyleSheet("QFrame#panel { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *statusLayout = new QVBoxLayout(statusCard);
    statusLayout->setContentsMargins(22, 18, 22, 20);
    statusLayout->setSpacing(14);

    auto *statusTitle = new QLabel("Phân bổ trạng thái lịch khám");
    statusTitle->setStyleSheet("font-size: 14.5px; font-weight: 700; color: #1e293b;");
    statusLayout->addWidget(statusTitle);

    auto *statusBar = new SegmentedStatusBar;
    statusBar->setCounts(completed, pending, delayed);
    statusLayout->addWidget(statusBar);

    auto *chipsGrid = new QGridLayout;
    chipsGrid->setSpacing(10);

    auto makeStatusChip = [](const QString &dotColor, const QString &name, int count, int total) {
        auto *f = new QFrame;
        f->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 6px 10px; }");
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(6, 4, 6, 4);
        h->setSpacing(8);

        auto *dot = new QLabel("●");
        dot->setStyleSheet(QStringLiteral("color: %1; font-size: 14px;").arg(dotColor));

        auto *title = new QLabel(name);
        title->setStyleSheet("font-size: 12px; font-weight: 600; color: #334155;");

        double pct = (total > 0) ? (count * 100.0 / total) : 0.0;
        auto *val = new QLabel(QStringLiteral("<b>%1</b> (%2%)").arg(count).arg(QString::number(pct, 'f', 1)));
        val->setStyleSheet("font-size: 12px; color: #0f172a;");

        h->addWidget(dot);
        h->addWidget(title);
        h->addStretch();
        h->addWidget(val);
        return f;
    };

    chipsGrid->addWidget(makeStatusChip("#0d9488", "Đã khám", completed, totalAppts), 0, 0);
    chipsGrid->addWidget(makeStatusChip("#ea580c", "Chưa khám", pending, totalAppts), 0, 1);
    chipsGrid->addWidget(makeStatusChip("#f59e0b", "Trễ hẹn", delayed, totalAppts), 1, 0, 1, 2);
    statusLayout->addLayout(chipsGrid);

    row1->addWidget(statusCard, 1);

    // Card 2: Nhân khẩu học bệnh nhân (Giới tính & Độ tuổi với Donut chart)
    auto *demoCard = UIHelper::panel();
    demoCard->setStyleSheet("QFrame#panel { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *demoLayout = new QVBoxLayout(demoCard);
    demoLayout->setContentsMargins(22, 18, 22, 20);
    demoLayout->setSpacing(10);

    auto *demoTitle = new QLabel("Đặc điểm bệnh nhân");
    demoTitle->setStyleSheet("font-size: 14.5px; font-weight: 700; color: #1e293b;");
    demoLayout->addWidget(demoTitle);

    // Giới tính
    double malePct = (totalPatients > 0) ? (male * 100.0 / totalPatients) : 0.0;
    double femalePct = (totalPatients > 0) ? (female * 100.0 / totalPatients) : 0.0;

    auto *genderH = new QHBoxLayout;
    auto *maleLbl = new QLabel(QStringLiteral("Nam: <b>%1%</b> (%2)").arg(QString::number(malePct, 'f', 0)).arg(male));
    maleLbl->setStyleSheet("font-size: 11.5px; color: #0284c7; font-weight: 600;");
    auto *femaleLbl = new QLabel(QStringLiteral("Nữ: <b>%1%</b> (%2)").arg(QString::number(femalePct, 'f', 0)).arg(female));
    femaleLbl->setStyleSheet("font-size: 11.5px; color: #db2777; font-weight: 600;");
    genderH->addWidget(maleLbl);
    genderH->addStretch();
    genderH->addWidget(femaleLbl);
    demoLayout->addLayout(genderH);

    auto *genderBar = new QProgressBar;
    genderBar->setFixedHeight(6);
    genderBar->setTextVisible(false);
    genderBar->setRange(0, totalPatients > 0 ? totalPatients : 1);
    genderBar->setValue(male);
    genderBar->setStyleSheet("QProgressBar { background: #fce7f3; border-radius: 3px; border: none; } QProgressBar::chunk { background: #0284c7; border-radius: 3px; }");
    demoLayout->addWidget(genderBar);

    demoLayout->addSpacing(4);

    // Nhóm tuổi với sơ đồ hình tròn Donut
    auto *ageSection = new QHBoxLayout;
    ageSection->setSpacing(14);

    auto *donutChart = new PatientDonutChart;
    donutChart->setData(under18, from18to45, from46to60, over60);
    ageSection->addWidget(donutChart, 0, Qt::AlignCenter);

    auto *ageList = new QVBoxLayout;
    ageList->setSpacing(5);

    auto makeAgeLegendRow = [](const QString &dotColor, const QString &label, int count, int total) {
        auto *h = new QHBoxLayout;
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(6);

        auto *dot = new QLabel("●");
        dot->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;").arg(dotColor));

        auto *name = new QLabel(label);
        name->setStyleSheet("font-size: 11.5px; color: #475569; font-weight: 600;");

        double pct = (total > 0) ? (count * 100.0 / total) : 0.0;
        auto *val = new QLabel(QStringLiteral("<b>%1</b> (%2%)").arg(count).arg(QString::number(pct, 'f', 0)));
        val->setStyleSheet("font-size: 11.5px; color: #0f172a;");

        h->addWidget(dot);
        h->addWidget(name);
        h->addStretch();
        h->addWidget(val);
        return h;
    };

    ageList->addLayout(makeAgeLegendRow("#38bdf8", "Dưới 18 tuổi", under18, totalPatients));
    ageList->addLayout(makeAgeLegendRow("#0d9488", "18 – 45 tuổi", from18to45, totalPatients));
    ageList->addLayout(makeAgeLegendRow("#f59e0b", "46 – 60 tuổi", from46to60, totalPatients));
    ageList->addLayout(makeAgeLegendRow("#8b5cf6", "Trên 60 tuổi", over60, totalPatients));

    ageSection->addLayout(ageList, 1);
    demoLayout->addLayout(ageSection);

    row1->addWidget(demoCard, 1);
    contentLayout->addLayout(row1);

    // 5. Hàng 2 Card: Nhu cầu theo Chuyên khoa & Bảng xếp hạng Bác sĩ
    auto *row2 = new QHBoxLayout;
    row2->setSpacing(16);

    // Card Nhu cầu Chuyên khoa (bỏ % theo yêu cầu)
    auto *specCard = UIHelper::panel();
    specCard->setStyleSheet("QFrame#panel { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *specLayout = new QVBoxLayout(specCard);
    specLayout->setContentsMargins(22, 18, 22, 20);
    specLayout->setSpacing(12);

    auto *specTitle = new QLabel("Nhu cầu khám theo chuyên khoa");
    specTitle->setStyleSheet("font-size: 14.5px; font-weight: 700; color: #1e293b;");
    specLayout->addWidget(specTitle);

    SpecialtyDemandItem topSpecs[5];
    int specCount = StatisticController::getSpecialtyDemand(appts, doctors, topSpecs, 5);

    if (specCount > 0) {
        for (int i = 0; i < specCount; ++i) {
            QString spName = mainWindow->specialty(topSpecs[i].spId);
            auto *v = new QVBoxLayout;
            v->setSpacing(3);

            auto *h = new QHBoxLayout;
            auto *name = new QLabel(QStringLiteral("<b>%1. %2</b>").arg(i + 1).arg(spName));
            name->setStyleSheet("font-size: 12.5px; color: #1e293b;");

            auto *val = new QLabel(QStringLiteral("<b>%1</b> lượt khám").arg(topSpecs[i].count));
            val->setStyleSheet("font-size: 12px; color: #0d9488; font-weight: 600;");
            h->addWidget(name);
            h->addStretch();
            h->addWidget(val);
            v->addLayout(h);

            auto *bar = new QProgressBar;
            bar->setFixedHeight(7);
            bar->setTextVisible(false);
            bar->setRange(0, totalAppts > 0 ? totalAppts : 1);
            bar->setValue(topSpecs[i].count);
            bar->setStyleSheet("QProgressBar { background: #f1f5f9; border-radius: 3.5px; border: none; } QProgressBar::chunk { background: #0284c7; border-radius: 3.5px; }");
            v->addWidget(bar);

            specLayout->addLayout(v);
        }
    } else {
        auto *emptyLbl = new QLabel("Chưa có số liệu chuyên khoa.");
        emptyLbl->setStyleSheet("color: #94a3b8; font-style: italic;");
        specLayout->addWidget(emptyLbl);
    }
    specLayout->addStretch();
    row2->addWidget(specCard, 1);

    // Card Top Bác sĩ
    auto *docCard = UIHelper::panel();
    docCard->setStyleSheet("QFrame#panel { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *docLayout = new QVBoxLayout(docCard);
    docLayout->setContentsMargins(22, 18, 22, 20);
    docLayout->setSpacing(10);

    auto *docTitle = new QLabel("Bác sĩ tiếp nhận nhiều bệnh nhân nhất");
    docTitle->setStyleSheet("font-size: 14.5px; font-weight: 700; color: #1e293b;");
    docLayout->addWidget(docTitle);

    DoctorLeaderboardItem topDocs[5];
    int docRankCount = StatisticController::getTopDoctors(appts, doctors, topDocs, 5);

    const QString badges[5] = {"🥇", "🥈", "🥉", "4", "5"};

    for (int i = 0; i < docRankCount; ++i) {
        auto *itemFrame = new QFrame;
        itemFrame->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 4px 8px; }");
        auto *h = new QHBoxLayout(itemFrame);
        h->setContentsMargins(8, 6, 8, 6);
        h->setSpacing(10);

        auto *badge = new QLabel(badges[i]);
        badge->setStyleSheet(i < 3 ? "font-size: 16px;" : "font-size: 12px; font-weight: bold; color: #64748b; padding: 0 4px;");

        auto *infoCol = new QVBoxLayout;
        infoCol->setSpacing(2);
        auto *dName = new QLabel(QStringLiteral("BS. %1").arg(UIHelper::qs(topDocs[i].doctorName)));
        dName->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #0f172a;");
        QString specStr = mainWindow->specialty(topDocs[i].specialtyId);
        QString roomStr = UIHelper::formatRoom(UIHelper::qs(topDocs[i].roomId));
        auto *dSub = new QLabel(QStringLiteral("%1 · %2").arg(specStr, roomStr));
        dSub->setStyleSheet("font-size: 11px; color: #64748b;");
        infoCol->addWidget(dName);
        infoCol->addWidget(dSub);

        auto *ratingLbl = new QLabel(QStringLiteral("⭐ %1").arg(QString::number(topDocs[i].rating, 'f', 1)));
        ratingLbl->setStyleSheet("font-size: 12px; font-weight: bold; color: #d97706;");

        auto *countLbl = new QLabel(QStringLiteral("<b>%1</b> ca").arg(topDocs[i].appointmentCount));
        countLbl->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #0d9488;");

        h->addWidget(badge);
        h->addLayout(infoCol, 1);
        h->addWidget(ratingLbl);
        h->addWidget(countLbl);

        docLayout->addWidget(itemFrame);
    }
    docLayout->addStretch();
    row2->addWidget(docCard, 1);

    contentLayout->addLayout(row2);

    // 6. Xu hướng lịch khám 7 ngày gần nhất (Weekly Trend)
    auto *trendCard = UIHelper::panel();
    trendCard->setStyleSheet("QFrame#panel { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto *trendLayout = new QVBoxLayout(trendCard);
    trendLayout->setContentsMargins(22, 18, 22, 20);
    trendLayout->setSpacing(12);

    auto *trendTitle = new QLabel("Lưu lượng lịch khám 7 ngày gần nhất");
    trendTitle->setStyleSheet("font-size: 14.5px; font-weight: 700; color: #1e293b;");
    trendLayout->addWidget(trendTitle);

    int weekCounts[7] = {0};
    StatisticController::getWeeklyTrend(appts, curDay, curMonth, curYear, weekCounts);

    QString weekDates[7];
    for (int i = 0; i < 7; ++i) {
        QDate d = curDate.addDays(i - 6);
        weekDates[i] = (i == 6) ? "Hôm nay" : d.toString("dd/MM");
    }

    auto *trendChart = new WeeklyTrendChart;
    trendChart->setData(weekCounts, weekDates);
    trendLayout->addWidget(trendChart);

    contentLayout->addWidget(trendCard);
}
