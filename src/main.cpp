#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include <iostream>
#include "Views/MainWindow.h"

// Quản lý ghi log đồng bộ và an toàn đa luồng
static QMutex s_logMutex;

static QString getLogFilePath() {
    QString baseDir = QCoreApplication::applicationDirPath();
    if (baseDir.endsWith("/bin") || baseDir.endsWith("\\bin")) {
        baseDir += "/..";
    }
    return QDir(baseDir).filePath("logs/app.log");
}

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker locker(&s_logMutex);

    // Xác định cấp độ log
    const char *levelStr = "INFO";
    switch (type) {
    case QtDebugMsg:    levelStr = "DEBUG"; break;
    case QtInfoMsg:     levelStr = "INFO"; break;
    case QtWarningMsg:  levelStr = "WARNING"; break;
    case QtCriticalMsg: levelStr = "CRITICAL"; break;
    case QtFatalMsg:    levelStr = "FATAL"; break;
    }

    // Thời gian định dạng chuẩn đến mili-giây
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    // Lấy tên file mã nguồn rút gọn
    QString file = context.file ? QString::fromUtf8(context.file) : "";
    if (!file.isEmpty()) {
        int idx = qMax(file.lastIndexOf('/'), file.lastIndexOf('\\'));
        if (idx != -1) file = file.mid(idx + 1);
    }

    QString location = file.isEmpty() ? "" : QStringLiteral(" [%1:%2]").arg(file).arg(context.line);
    QString logLine = QStringLiteral("[%1] [%2]%3 %4\n").arg(timestamp, levelStr, location, msg);

    // Đảm bảo thư mục logs/ tồn tại
    QString logFilePath = getLogFilePath();
    QDir().mkpath(QFileInfo(logFilePath).dir().absolutePath());

    // Ghi nối tiếp vào file logs/app.log
    QFile outFile(logFilePath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        // Tự động xoay file nếu vượt quá 5MB
        if (outFile.size() > 5 * 1024 * 1024) {
            outFile.close();
            QString oldLogPath = logFilePath + ".old";
            QFile::remove(oldLogPath);
            QFile::rename(logFilePath, oldLogPath);
            if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                return;
            }
        }
        QTextStream ts(&outFile);
        ts << logLine;
        outFile.close();
    }

    // Đồng thời hiển thị ra cửa sổ console / terminal
    std::cerr << logLine.toUtf8().constData();
    std::cerr.flush();

    if (type == QtFatalMsg) {
        abort();
    }
}

int main(int argc, char *argv[]) {
    // Đảm bảo thư mục làm việc chuẩn nếu chạy từ bin/
    QCoreApplication::setApplicationName("Phòng khám Đa khoa An Tâm");
    QString appDir = QFileInfo(QString::fromLocal8Bit(argv[0])).dir().absolutePath();
    if (appDir.endsWith("/bin") || appDir.endsWith("\\bin")) {
        QDir::setCurrent(appDir + "/..");
    }

    // Kích hoạt bộ thu thập log tự động của Qt (Cách 1: qInstallMessageHandler)
    qInstallMessageHandler(customMessageHandler);

    QApplication app(argc, argv);

    qInfo() << "==================================================";
    qInfo() << "✦ AN TÂM · Hệ thống Quản lý Phòng khám Đa khoa";
    qInfo() << "Khởi động ứng dụng phiên bản Qt:" << qVersion();
    qInfo() << "Đường dẫn file log:" << getLogFilePath();
    qInfo() << "==================================================";

    // Cài đặt icon ứng dụng
    QIcon appIcon(":/assets/textures/icon_app.ico");
    if (appIcon.isNull()) appIcon = QIcon("assets/textures/icon_app.ico");
    app.setWindowIcon(appIcon);

    // Tải bộ font ứng dụng từ Resource
    if (QFontDatabase::addApplicationFont(":/assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Regular.ttf") < 0)
        QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Regular.ttf");
    if (QFontDatabase::addApplicationFont(":/assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Bold.ttf") < 0)
        QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-Bold.ttf");
    if (QFontDatabase::addApplicationFont(":/assets/fonts/Be_Vietnam_Pro/BeVietnamPro-SemiBold.ttf") < 0)
        QFontDatabase::addApplicationFont("assets/fonts/Be_Vietnam_Pro/BeVietnamPro-SemiBold.ttf");
    if (QFontDatabase::addApplicationFont(":/assets/fonts/PlayFair_Display/PlayfairDisplay-Regular.ttf") < 0)
        QFontDatabase::addApplicationFont("assets/fonts/PlayFair_Display/PlayfairDisplay-Regular.ttf");
    if (QFontDatabase::addApplicationFont(":/assets/fonts/PlayFair_Display/PlayfairDisplay-Bold.ttf") < 0)
        QFontDatabase::addApplicationFont("assets/fonts/PlayFair_Display/PlayfairDisplay-Bold.ttf");

    QFont appFont("Be Vietnam Pro", 10);
    app.setFont(appFont);

    // Khởi tạo cửa sổ chính và thực thi ứng dụng
    MainWindow window;
    window.showMaximized();

    int ret = app.exec();
    qInfo() << "Ứng dụng kết thúc với mã thoát:" << ret;
    return ret;
}
