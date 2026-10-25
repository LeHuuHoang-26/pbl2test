#include "Login.h"
#include "libPBL2.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QAction>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QCursor>
#include <QResizeEvent>
#include <QtMath>

static QIcon createUserIcon() {
    int size = 56;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(2.0, 2.0); // 28x28 logical canvas

    // Màu icon Teal thương hiệu An Tâm
    QColor iconColor("#0D9488");
    p.setPen(Qt::NoPen);
    p.setBrush(iconColor);

    // Đầu hình tròn (đường kính 10px, to rõ ràng)
    p.drawEllipse(QPointF(14.0, 8.5), 5.0, 5.0);

    // Thân vai hình vòm nở rộng cân bằng (rộng 19px)
    QPainterPath body;
    body.moveTo(4.5, 23.5);
    body.quadTo(4.5, 16.5, 10.0, 15.5);
    body.lineTo(18.0, 15.5);
    body.quadTo(23.5, 16.5, 23.5, 23.5);
    body.closeSubpath();
    p.drawPath(body);

    pix.setDevicePixelRatio(2.0);
    return QIcon(pix);
}

static QIcon createLockIcon() {
    int size = 56;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(2.0, 2.0); // 28x28 logical canvas

    QColor iconColor("#0D9488");

    // Càng khóa (Shackle) to hơn, nét dày 2.4px
    QPen shacklePen(iconColor, 2.4);
    shacklePen.setCapStyle(Qt::RoundCap);
    p.setPen(shacklePen);
    p.setBrush(Qt::NoBrush);
    QPainterPath shackle;
    shackle.moveTo(9.0, 12.0);
    shackle.lineTo(9.0, 8.0);
    shackle.arcTo(9.0, 3.8, 10.0, 8.4, 180, -180);
    shackle.lineTo(19.0, 12.0);
    p.drawPath(shackle);

    // Thân ổ khóa hình chữ nhật bo góc cân đối (19px rộng x 12.0px cao)
    p.setPen(Qt::NoPen);
    p.setBrush(iconColor);
    p.drawRoundedRect(QRectF(4.5, 11.5, 19.0, 12.0), 3.0, 3.0);

    // Lỗ khóa màu trắng tinh tế
    p.setBrush(QColor("#FFFFFF"));
    p.drawEllipse(QPointF(14.0, 15.8), 1.6, 1.6);
    p.drawRect(QRectF(13.2, 15.8, 1.6, 3.0));

    pix.setDevicePixelRatio(2.0);
    return QIcon(pix);
}

static QIcon createEyeIcon(bool isVisible) {
    int size = 56;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(2.0, 2.0); // 28x28 logical canvas

    if (!isVisible) {
        // --- TRẠNG THÁI MẬT KHẨU ĐANG ẨN (Lucide/Feather Eye-Off tối giản, thanh lịch) ---
        QColor color("#94A3B8"); // Màu xám slate nhẹ nhàng, đồng bộ màu viền input
        QPen pen(color, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        // Vòng mí mắt
        QPainterPath eyePath;
        eyePath.moveTo(3.5, 14.0);
        eyePath.quadTo(14.0, 6.5, 24.5, 14.0);
        eyePath.quadTo(14.0, 21.5, 3.5, 14.0);
        p.drawPath(eyePath);

        // Con ngươi ở trung tâm
        p.drawEllipse(QPointF(14.0, 14.0), 3.0, 3.0);

        // Đường gạch chéo thanh lịch cùng màu slate
        p.drawLine(QPointF(5.0, 5.0), QPointF(23.0, 23.0));
    } else {
        // --- TRẠNG THÁI MẬT KHẨU ĐANG HIỆN (Eye-Open màu Teal sáng rõ) ---
        QColor color("#0D9488");
        QPen pen(color, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        // Mí mắt mở to tròn
        QPainterPath eyePath;
        eyePath.moveTo(3.0, 14.0);
        eyePath.quadTo(14.0, 6.0, 25.0, 14.0);
        eyePath.quadTo(14.0, 22.0, 3.0, 14.0);
        p.drawPath(eyePath);

        // Con ngươi tròn đầy đặn
        p.setBrush(color);
        p.drawEllipse(QPointF(14.0, 14.0), 4.2, 4.2);

        // Đốm sáng phản chiếu tạo sức sống cho mắt
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#FFFFFF"));
        p.drawEllipse(QPointF(15.2, 12.8), 1.2, 1.2);
    }

    pix.setDevicePixelRatio(2.0);
    return QIcon(pix);
}

static QPixmap loadAssetPixmap(const QString &qrcPath, const QString &diskRelPath) {
    if (QFile::exists(qrcPath)) {
        return QPixmap(qrcPath);
    }
    if (QFile::exists(diskRelPath)) {
        return QPixmap(diskRelPath);
    }
    return QPixmap();
}

Login::Login(QWidget *parent)
    : QWidget(parent),
      m_cardWidget(nullptr),
      m_lblIllustration(nullptr),
      m_txtUsername(nullptr),
      m_txtPassword(nullptr),
      m_btnLogin(nullptr),
      m_lblMessage(nullptr),
      m_actionTogglePassword(nullptr),
      m_isPasswordVisible(false)
{
    setupUI();
    setupConnections();
}

void Login::setupUI() {
    // 1. Layout tổng thể căn giữa Card
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(30, 20, 30, 20);
    rootLayout->setAlignment(Qt::AlignCenter);

    // 2. White Card Widget
    m_cardWidget = new QWidget(this);
    m_cardWidget->setObjectName("cardWidget");
    m_cardWidget->setFixedSize(1040, 610);
    m_cardWidget->setStyleSheet(
        "QWidget#cardWidget {"
        "   background-color: #FFFFFF;"
        "   border-radius: 26px;"
        "}"
    );

    // Hiệu ứng bóng đổ mềm mại, sâu sắc
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(m_cardWidget);
    shadow->setBlurRadius(55);
    shadow->setOffset(0, 18);
    shadow->setColor(QColor(15, 23, 42, 75));
    m_cardWidget->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_cardWidget);
    cardLayout->setContentsMargins(48, 30, 48, 24);
    cardLayout->setSpacing(0);

    // ==========================================
    // A. HEADER TRONG CARD (Logo & Thương hiệu An Tâm Clinic)
    // ==========================================
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);

    QLabel *lblIcon = new QLabel(m_cardWidget);
    QPixmap appIcon = loadAssetPixmap(":/assets/textures/icon_app.png", "assets/textures/icon_app.png");
    if (!appIcon.isNull()) {
        qreal dpr = qMax(qreal(2.0), devicePixelRatioF());
        int targetH = 46;
        QPixmap scaledIcon = appIcon.scaledToHeight(qRound(targetH * dpr), Qt::SmoothTransformation);
        scaledIcon.setDevicePixelRatio(dpr);
        lblIcon->setPixmap(scaledIcon);
    }
    headerLayout->addWidget(lblIcon);

    QLabel *lblLogo = new QLabel(m_cardWidget);
    lblLogo->setObjectName("lblLogo");
    lblLogo->setText("<div style='line-height: 115%;'>"
                     "<span style='color:#0F172A; font-weight:900; font-size:22px; letter-spacing:0.3px;'>AN TÂM </span>"
                     "<span style='color:#0D9488; font-weight:800; font-size:18px; letter-spacing:0.5px;'>MNG</span><br>"
                     "<span style='color:#64748B; font-weight:700; font-size:11px; letter-spacing:1px;'>PHÒNG KHÁM ĐA KHOA</span>"
                     "</div>");
    headerLayout->addWidget(lblLogo);
    headerLayout->addStretch();

    cardLayout->addLayout(headerLayout);
    cardLayout->addSpacing(30);

    // ==========================================
    // B. BODY: 2 CỘT (Form bên trái, Illustration bên phải)
    // ==========================================
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(40);

    // --- CỘT TRÁI: FORM ĐĂNG NHẬP NÂNG CẤP ĐẸP MẮT ---
    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setContentsMargins(0, 0, 10, 0);
    leftCol->setSpacing(0);

    QWidget *formContainer = new QWidget(m_cardWidget);
    formContainer->setMaximumWidth(440);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(0);

    // Tiêu đề Sign in to account (Font chủ đề Be Vietnam Pro hiện đại, ăn khớp nút Sign in)
    QLabel *lblTitle = new QLabel("Sign in to account", formContainer);
    QFont titleFont("Be Vietnam Pro", 22, QFont::Bold);
    titleFont.setStyleStrategy(QFont::PreferAntialias);
    lblTitle->setFont(titleFont);
    lblTitle->setStyleSheet("font-family: 'Be Vietnam Pro', 'Segoe UI', sans-serif; color: #0F172A; font-size: 28px; font-weight: 700; letter-spacing: -0.4px;");

    // Thanh gạch chỉ báo gradient hiện đại dưới tiêu đề
    QWidget *titleAccent = new QWidget(formContainer);
    titleAccent->setFixedSize(52, 4);
    titleAccent->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0D9488, stop:1 #14B8A6); border-radius: 2px;");

    formLayout->addWidget(lblTitle);
    formLayout->addSpacing(8);
    formLayout->addWidget(titleAccent);
    formLayout->addSpacing(32);

    // Style cao cấp cho các ô input (Viền mượt, focus glow, phông chữ chuẩn)
    QString inputStyle =
        "QLineEdit {"
        "   background-color: #F8FAFC;"
        "   border: 1.5px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 0px 14px 0px 14px;"
        "   font-size: 14px;"
        "   color: #0F172A;"
        "   min-height: 48px;"
        "}"
        "QLineEdit:hover {"
        "   background-color: #FFFFFF;"
        "   border: 1.5px solid #CBD5E1;"
        "}"
        "QLineEdit:focus {"
        "   background-color: #FFFFFF;"
        "   border: 2px solid #0D9488;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #94A3B8;"
        "   font-size: 13.5px;"
        "}";

    // Nhãn & Input Username
    QLabel *lblUser = new QLabel("Username", formContainer);
    lblUser->setStyleSheet("color: #334155; font-size: 13.5px; font-weight: 700; margin-bottom: 6px;");
    formLayout->addWidget(lblUser);

    m_txtUsername = new QLineEdit(formContainer);
    m_txtUsername->setPlaceholderText("Nhập Username");
    m_txtUsername->setStyleSheet(inputStyle);
    m_txtUsername->addAction(createUserIcon(), QLineEdit::LeadingPosition);
    formLayout->addWidget(m_txtUsername);

    formLayout->addSpacing(18);

    // Nhãn & Input Password
    QLabel *lblPass = new QLabel("Password", formContainer);
    lblPass->setStyleSheet("color: #334155; font-size: 13.5px; font-weight: 700; margin-bottom: 6px;");
    formLayout->addWidget(lblPass);

    m_txtPassword = new QLineEdit(formContainer);
    m_txtPassword->setPlaceholderText("Nhập mật khẩu");
    m_txtPassword->setEchoMode(QLineEdit::Password);
    m_txtPassword->setStyleSheet(inputStyle);
    m_txtPassword->addAction(createLockIcon(), QLineEdit::LeadingPosition);
    m_actionTogglePassword = m_txtPassword->addAction(createEyeIcon(false), QLineEdit::TrailingPosition);
    formLayout->addWidget(m_txtPassword);

    formLayout->addSpacing(12);

    // Thông báo lỗi / thành công dạng Card Alert
    m_lblMessage = new QLabel(formContainer);
    m_lblMessage->setWordWrap(true);
    m_lblMessage->hide();
    formLayout->addWidget(m_lblMessage);

    formLayout->addSpacing(22);

    // Nút Sign in tone xanh Teal đồng bộ với ứng dụng
    m_btnLogin = new QPushButton("Sign in", formContainer);
    m_btnLogin->setObjectName("btnLogin");
    m_btnLogin->setCursor(Qt::PointingHandCursor);
    QFont btnFont("Be Vietnam Pro", 11, QFont::DemiBold);
    btnFont.setStyleStrategy(QFont::PreferAntialias);
    m_btnLogin->setFont(btnFont);
    m_btnLogin->setStyleSheet(
        "QPushButton#btnLogin {"
        "   font-family: 'Be Vietnam Pro', 'Segoe UI', sans-serif;"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0D9488, stop:0.55 #0F766E, stop:1 #115E59);"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 14px;"
        "   min-height: 50px;"
        "   font-size: 15.5px;"
        "   font-weight: 600;"
        "   letter-spacing: 0.3px;"
        "}"
        "QPushButton#btnLogin:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #14B8A6, stop:0.55 #0D9488, stop:1 #0F766E);"
        "}"
        "QPushButton#btnLogin:pressed {"
        "   background: #134E4A;"
        "}"
        "QPushButton#btnLogin:disabled {"
        "   background-color: #CBD5E1;"
        "   color: #94A3B8;"
        "}"
    );

    formLayout->addWidget(m_btnLogin);
    formLayout->addStretch();

    leftCol->addWidget(formContainer);
    leftCol->addStretch();

    bodyLayout->addLayout(leftCol, 5);

    // --- CỘT PHẢI: MINH HỌA Y TẾ ĐẶT LỊCH ĐỘ NÉT CAO (HiDPI RETINA) ---
    QVBoxLayout *rightCol = new QVBoxLayout();
    rightCol->setContentsMargins(0, 0, 0, 0);
    rightCol->setAlignment(Qt::AlignCenter);

    m_lblIllustration = new QLabel(m_cardWidget);
    m_lblIllustration->setAlignment(Qt::AlignCenter);
    updateIllustrationScale();
    rightCol->addWidget(m_lblIllustration);

    bodyLayout->addLayout(rightCol, 5);

    cardLayout->addLayout(bodyLayout);
    cardLayout->addSpacing(15);

    // ==========================================
    // C. FOOTER TRONG CARD
    // ==========================================
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 10, 0, 0);

    QLabel *lblCopy = new QLabel("© 2026 Nhà Khám An Tâm. Hệ thống Quản lý Đặt lịch Khám bệnh.", m_cardWidget);
    lblCopy->setStyleSheet("color: #94A3B8; font-size: 11.5px;");

    QLabel *lblSupport = new QLabel("Hỗ trợ kỹ thuật: antam@support.vn", m_cardWidget);
    lblSupport->setStyleSheet("color: #94A3B8; font-size: 11.5px;");

    QHBoxLayout *dotsLayout = new QHBoxLayout();
    dotsLayout->setSpacing(6);

    QLabel *dot1 = new QLabel(m_cardWidget);
    dot1->setFixedSize(8, 8);
    dot1->setStyleSheet("background-color: #10B981; border-radius: 4px;"); // Xanh lá online

    QLabel *dot2 = new QLabel(m_cardWidget);
    dot2->setFixedSize(8, 8);
    dot2->setStyleSheet("background-color: #0D9488; border-radius: 4px;"); // Xanh Teal

    QLabel *dot3 = new QLabel(m_cardWidget);
    dot3->setFixedSize(8, 8);
    dot3->setStyleSheet("background-color: #14B8A6; border-radius: 4px;"); // Xanh Teal sáng

    dotsLayout->addWidget(dot1);
    dotsLayout->addWidget(dot2);
    dotsLayout->addWidget(dot3);

    footerLayout->addWidget(lblCopy);
    footerLayout->addStretch();
    footerLayout->addWidget(lblSupport);
    footerLayout->addSpacing(16);
    footerLayout->addLayout(dotsLayout);

    cardLayout->addLayout(footerLayout);

    rootLayout->addWidget(m_cardWidget);
}

void Login::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (!m_cardWidget) return;

    // Kích thước card cân đối khi phóng to
    int targetW = qBound(980, static_cast<int>(width() * 0.65), 1140);
    int targetH = qBound(580, static_cast<int>(height() * 0.72), 640);
    m_cardWidget->setFixedSize(targetW, targetH);

    updateIllustrationScale();
}

void Login::updateIllustrationScale() {
    if (!m_lblIllustration || !m_cardWidget) return;
    QPixmap illPix = loadAssetPixmap(":/assets/textures/login_illustration.png", "assets/textures/login_illustration.png");
    if (illPix.isNull()) return;

    // HiDPI Supersampling 2.0x hoặc theo devicePixelRatio của màn hình
    qreal dpr = qMax(qreal(2.0), devicePixelRatioF());
    int targetH = m_cardWidget->height() - 130;
    int targetW = (m_cardWidget->width() / 2) + 30;

    int physW = qRound(targetW * dpr);
    int physH = qRound(targetH * dpr);

    QPixmap scaledPix = illPix.scaled(physW, physH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledPix.setDevicePixelRatio(dpr);
    m_lblIllustration->setPixmap(scaledPix);
}

void Login::setupConnections() {
    // Nút Log in & phím Enter tại các ô nhập
    connect(m_btnLogin, &QPushButton::clicked, this, &Login::loginClicked);
    connect(m_txtUsername, &QLineEdit::returnPressed, this, &Login::loginClicked);
    connect(m_txtPassword, &QLineEdit::returnPressed, this, &Login::loginClicked);

    // Toggle ẩn/hiện mật khẩu
    connect(m_actionTogglePassword, &QAction::triggered, this, &Login::onTogglePasswordVisibility);
}

void Login::onTogglePasswordVisibility() {
    m_isPasswordVisible = !m_isPasswordVisible;
    m_txtPassword->setEchoMode(m_isPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);
    m_actionTogglePassword->setIcon(createEyeIcon(m_isPasswordVisible));
}

void Login::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // 1. Nền Gradient chuyển sắc đồng bộ với ứng dụng An Tâm (Dark Teal Navy)
    QLinearGradient bgGrad(0, 0, w, h);
    bgGrad.setColorAt(0.0, QColor("#0F2A3A"));  // Dark Teal Navy (đồng bộ sidebar ứng dụng)
    bgGrad.setColorAt(0.5, QColor("#0A1F2C"));  // Midnight Teal
    bgGrad.setColorAt(1.0, QColor("#06131C"));  // Deep Dark Teal Slate
    painter.fillRect(rect(), bgGrad);

    // 2. Các quầng sáng Ambient Lighting tạo chiều sâu không gian (Tone Teal/Cyan/Emerald)
    // Quầng sáng trung tâm chiếu sau thẻ Card
    QRadialGradient centerGlow(w * 0.5, h * 0.5, qMax(w, h) * 0.52);
    centerGlow.setColorAt(0.0, QColor(13, 148, 136, 55));
    centerGlow.setColorAt(0.65, QColor(15, 118, 110, 20));
    centerGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), centerGlow);

    // Quầng sáng Cyan góc trên trái
    QRadialGradient cyanGlow(w * 0.15, h * 0.22, w * 0.38);
    cyanGlow.setColorAt(0.0, QColor(20, 184, 166, 45));
    cyanGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), cyanGlow);

    // Quầng sáng Emerald góc dưới phải
    QRadialGradient emeraldGlow(w * 0.85, h * 0.78, w * 0.38);
    emeraldGlow.setColorAt(0.0, QColor(16, 185, 129, 40));
    emeraldGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), emeraldGlow);

    // 3. Đường sóng nhịp tim ECG (Cardiogram Waveforms) hai nhịp liên tục hai bên lề
    auto drawECGWave = [&](qreal startX, qreal endX, qreal midY, qreal amp, const QColor &color) {
        if (endX <= startX + 60) return;
        QPainterPath path;
        qreal totalLen = endX - startX;
        path.moveTo(startX, midY);

        // Nhịp 1 (ở 35% độ dài)
        path.lineTo(startX + totalLen * 0.15, midY);
        path.lineTo(startX + totalLen * 0.20, midY - amp * 0.25); // P
        path.lineTo(startX + totalLen * 0.24, midY);
        path.lineTo(startX + totalLen * 0.28, midY + amp * 0.20); // Q
        path.lineTo(startX + totalLen * 0.33, midY - amp);        // R
        path.lineTo(startX + totalLen * 0.38, midY + amp * 0.50); // S
        path.lineTo(startX + totalLen * 0.43, midY);
        path.lineTo(startX + totalLen * 0.48, midY - amp * 0.35); // T
        path.lineTo(startX + totalLen * 0.54, midY);

        // Nhịp 2 (ở 75% độ dài)
        path.lineTo(startX + totalLen * 0.62, midY);
        path.lineTo(startX + totalLen * 0.66, midY - amp * 0.20); // P
        path.lineTo(startX + totalLen * 0.70, midY);
        path.lineTo(startX + totalLen * 0.73, midY + amp * 0.18); // Q
        path.lineTo(startX + totalLen * 0.77, midY - amp * 0.90); // R
        path.lineTo(startX + totalLen * 0.81, midY + amp * 0.45); // S
        path.lineTo(startX + totalLen * 0.85, midY);
        path.lineTo(startX + totalLen * 0.89, midY - amp * 0.30); // T
        path.lineTo(endX, midY);

        QLinearGradient fadeGrad(startX, midY, endX, midY);
        QColor c0 = color; c0.setAlpha(0);
        QColor c1 = color;
        QColor c2 = color; c2.setAlpha(0);
        fadeGrad.setColorAt(0.0, c0);
        fadeGrad.setColorAt(0.5, c1);
        fadeGrad.setColorAt(1.0, c2);

        QPen pen(fadeGrad, 2.0);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);

        // Điểm sáng nhịp tim tại đỉnh R nhịp 1
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(45, 212, 191, 230));
        painter.drawEllipse(QPointF(startX + totalLen * 0.33, midY - amp), 3.0, 3.0);
    };

    // Tọa độ mép của Card
    qreal cardLeft = (m_cardWidget) ? m_cardWidget->x() : (w * 0.5 - 520);
    qreal cardRight = (m_cardWidget) ? (m_cardWidget->x() + m_cardWidget->width()) : (w * 0.5 + 520);

    // Nếu khoảng trống hai bên đủ rộng (khi phóng to), vẽ sóng nhịp tim sinh động
    if (cardLeft > 130) {
        drawECGWave(20, cardLeft - 20, h * 0.38, 28, QColor(20, 184, 166, 140));
        drawECGWave(35, cardLeft - 35, h * 0.65, 22, QColor(13, 148, 136, 110));
    }
    if (w - cardRight > 130) {
        drawECGWave(cardRight + 20, w - 20, h * 0.42, 28, QColor(20, 184, 166, 140));
        drawECGWave(cardRight + 35, w - 35, h * 0.68, 22, QColor(13, 148, 136, 110));
    }

    // 4. Biểu tượng chữ thập y tế (Medical Crosses) bo tròn sang trọng
    auto drawMedicalCross = [&](qreal cx, qreal cy, qreal size, const QColor &color, qreal angle) {
        painter.save();
        painter.translate(cx, cy);
        if (angle != 0.0) painter.rotate(angle);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        qreal armW = size * 0.34;
        qreal halfArm = armW / 2.0;
        qreal halfSize = size / 2.0;
        painter.drawRoundedRect(QRectF(-halfArm, -halfSize, armW, size), armW * 0.35, armW * 0.35);
        painter.drawRoundedRect(QRectF(-halfSize, -halfArm, size, armW), armW * 0.35, armW * 0.35);
        painter.restore();
    };

    // Phân bổ chữ thập y tế hai bên lề và trên/dưới
    drawMedicalCross(w * 0.07, h * 0.18, 38, QColor(20, 184, 166, 70), 0);
    drawMedicalCross(w * 0.12, h * 0.82, 28, QColor(255, 255, 255, 55), 15);
    drawMedicalCross(w * 0.05, h * 0.52, 24, QColor(13, 148, 136, 65), -10);

    drawMedicalCross(w * 0.93, h * 0.22, 36, QColor(20, 184, 166, 70), -5);
    drawMedicalCross(w * 0.88, h * 0.76, 30, QColor(255, 255, 255, 55), 12);
    drawMedicalCross(w * 0.95, h * 0.88, 24, QColor(13, 148, 136, 65), 0);

    // Chữ thập y tế trên và dưới card
    drawMedicalCross(w * 0.28, h * 0.06, 20, QColor(255, 255, 255, 45), 0);
    drawMedicalCross(w * 0.72, h * 0.94, 22, QColor(20, 184, 166, 60), 0);
    drawMedicalCross(w * 0.50, h * 0.05, 18, QColor(20, 184, 166, 45), 0);

    // 5. Biểu tượng viên thuốc con nhộng (Pill Capsule) hiện đại
    auto drawCapsulePill = [&](qreal cx, qreal cy, qreal len, qreal thick, qreal angle, const QColor &c1, const QColor &c2) {
        painter.save();
        painter.translate(cx, cy);
        painter.rotate(angle);
        painter.setPen(Qt::NoPen);

        // Nửa trái
        painter.setBrush(c1);
        QPainterPath leftP;
        leftP.moveTo(0, -thick / 2.0);
        leftP.lineTo(-len / 2.0 + thick / 2.0, -thick / 2.0);
        leftP.arcTo(-len / 2.0, -thick / 2.0, thick, thick, 90, 180);
        leftP.lineTo(0, thick / 2.0);
        leftP.closeSubpath();
        painter.drawPath(leftP);

        // Nửa phải
        painter.setBrush(c2);
        QPainterPath rightP;
        rightP.moveTo(0, -thick / 2.0);
        rightP.lineTo(len / 2.0 - thick / 2.0, -thick / 2.0);
        rightP.arcTo(len / 2.0 - thick, -thick / 2.0, thick, thick, 90, -180);
        rightP.lineTo(0, thick / 2.0);
        rightP.closeSubpath();
        painter.drawPath(rightP);

        painter.restore();
    };

    drawCapsulePill(w * 0.09, h * 0.32, 34, 15, -35, QColor(255, 255, 255, 55), QColor(20, 184, 166, 75));
    drawCapsulePill(w * 0.91, h * 0.60, 32, 14, 40, QColor(13, 148, 136, 70), QColor(255, 255, 255, 50));

    // 6. Lưới điểm chấm công nghệ y tế (Medical Tech Dot Matrix)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 45));
    // Lưới góc trên trái
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 6; ++c) {
            painter.drawEllipse(QPointF(w * 0.035 + c * 16, h * 0.065 + r * 16), 1.8, 1.8);
        }
    }
    // Lưới góc dưới phải
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 8; ++c) {
            painter.drawEllipse(QPointF(w * 0.88 + c * 15, h * 0.85 + r * 15), 1.8, 1.8);
        }
    }

    // 7. Các vòng tròn hào quang tinh tế (Glass Rings & Bokeh Halos)
    QPen ringPen(QColor(255, 255, 255, 30), 1.5);
    painter.setPen(ringPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(w * 0.09, h * 0.88), 75, 75);
    painter.drawEllipse(QPointF(w * 0.09, h * 0.88), 45, 45);

    painter.drawEllipse(QPointF(w * 0.92, h * 0.14), 60, 60);
    painter.drawEllipse(QPointF(w * 0.92, h * 0.14), 35, 35);

    // Vòng nét đứt hiện đại
    QPen dashRing(QColor(20, 184, 166, 60), 1.2, Qt::DashLine);
    painter.setPen(dashRing);
    painter.drawEllipse(QPointF(w * 0.18, h * 0.15), 34, 34);
    painter.drawEllipse(QPointF(w * 0.82, h * 0.88), 44, 44);

    // 8. Các điểm sáng lấp lánh (Sparkle Points)
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 212, 191, 200));
    painter.drawEllipse(QPointF(w * 0.22, h * 0.28), 3.5, 3.5);
    painter.drawEllipse(QPointF(w * 0.78, h * 0.72), 3.5, 3.5);
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.drawEllipse(QPointF(w * 0.15, h * 0.48), 2.5, 2.5);
    painter.drawEllipse(QPointF(w * 0.85, h * 0.52), 2.5, 2.5);
}

QString Login::getUsername() const {
    return m_txtUsername->text();
}

QString Login::getPassword() const {
    return m_txtPassword->text();
}

void Login::setUsername(const QString &username) {
    m_txtUsername->setText(username);
}

void Login::setPassword(const QString &password) {
    m_txtPassword->setText(password);
}

void Login::clearInputs() {
    m_txtUsername->clear();
    m_txtPassword->clear();
    m_txtUsername->setFocus();
}

void Login::showErrorMessage(const QString &message) {
    m_lblMessage->setText(message);
    m_lblMessage->setStyleSheet("background-color: #FEF2F2; border: 1px solid #FECACA; border-radius: 10px; color: #DC2626; font-size: 12.5px; font-weight: 600; padding: 8px 14px;");
    m_lblMessage->show();
}

void Login::showSuccessMessage(const QString &message) {
    m_lblMessage->setText(message);
    m_lblMessage->setStyleSheet("background-color: #ECFDF5; border: 1px solid #A7F3D0; border-radius: 10px; color: #059669; font-size: 12.5px; font-weight: 700; padding: 8px 14px;");
    m_lblMessage->show();
}

void Login::clearMessage() {
    m_lblMessage->clear();
    m_lblMessage->hide();
}

void Login::setInputEnabled(bool enabled) {
    m_txtUsername->setEnabled(enabled);
    m_txtPassword->setEnabled(enabled);
    m_btnLogin->setEnabled(enabled);
}
