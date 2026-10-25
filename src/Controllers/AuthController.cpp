#include "AuthController.h"

Manager *AuthController::authenticate(LinkList<Manager> *managers, const std::string &username, const std::string &password) {
    if (!managers || !managers->getHeader()) return nullptr;
    auto *node = managers->getHeader()->next;
    while (node && node->next) {
        if (node->info.isActive() && node->info.isValid(username, password)) return &node->info;
        node = node->next;
    }
    return nullptr;
}

bool AuthController::login(LinkList<Manager> *managers, const QString &username, const QString &password, QString &errorMessage, Manager *&outManager) {
    outManager = nullptr;
    QString u = username.trimmed();
    if (u.isEmpty()) {
        errorMessage = QStringLiteral("Vui lòng nhập tên đăng nhập!");
        return false;
    }
    if (password.isEmpty()) {
        errorMessage = QStringLiteral("Vui lòng nhập mật khẩu!");
        return false;
    }
    if (!managers || !managers->getHeader()) {
        errorMessage = QStringLiteral("Không thể tải danh sách tài khoản!");
        return false;
    }

    std::string su = u.toStdString();
    std::string sp = password.toStdString();

    // Tìm kiếm tài khoản trong danh sách
    Manager *found = nullptr;
    auto *n = managers->getHeader()->next;
    while (n && n->next) {
        if (n->info.getUsername() == su) {
            found = &n->info;
            break;
        }
        n = n->next;
    }

    if (!found) {
        errorMessage = QStringLiteral("Tài khoản không tồn tại trên hệ thống!");
        return false;
    }

    if (!found->isActive()) {
        errorMessage = QStringLiteral("Tài khoản này hiện đang bị khóa!");
        return false;
    }

    if (!found->isValid(su, sp)) {
        errorMessage = QStringLiteral("Mật khẩu không chính xác, vui lòng thử lại!");
        return false;
    }

    outManager = found;
    return true;
}
