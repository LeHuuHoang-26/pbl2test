#define UNICODE
#define _UNICODE
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Xác định thư mục hiện tại của file launcher
    wchar_t dir[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, dir, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        MessageBoxW(NULL, L"Lỗi xác định đường dẫn thư mục ứng dụng!", L"Lỗi", MB_ICONERROR | MB_OK);
        return 1;
    }

    wchar_t *lastSlash = wcsrchr(dir, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0';
    }

    // 2. Tạo đường dẫn tương đối tới bin\AnTamMng.exe
    wchar_t exePath[MAX_PATH];
    lstrcpyW(exePath, dir);
    lstrcatW(exePath, L"bin\\AnTamMng.exe");

    // 3. Kiểm tra file bin\AnTamMng.exe có tồn tại không
    DWORD attrib = GetFileAttributesW(exePath);
    if (attrib == INVALID_FILE_ATTRIBUTES || (attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        MessageBoxW(NULL, 
            L"Không tìm thấy file: bin\\AnTamMng.exe\n\nVui lòng đảm bảo bạn không xóa hoặc di chuyển thư mục bin!", 
            L"Lỗi khởi động AnTamMng", 
            MB_ICONERROR | MB_OK);
        return 1;
    }

    // 4. Khởi chạy ứng dụng với Working Directory là thư mục gốc dự án
    HINSTANCE res = ShellExecuteW(NULL, L"open", exePath, NULL, dir, SW_SHOWNORMAL);
    if ((INT_PTR)res <= 32) {
        MessageBoxW(NULL, L"Không thể khởi động tiến trình bin\\AnTamMng.exe!", L"Lỗi hệ thống", MB_ICONERROR | MB_OK);
        return 1;
    }

    return 0;
}
