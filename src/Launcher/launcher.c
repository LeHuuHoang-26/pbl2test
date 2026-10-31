#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#define BUFFER_SIZE 32768

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    // 1. Xác định đường dẫn thư mục hiện tại của file launcher (hỗ trợ đường dẫn dài và Unicode)
    wchar_t *dir = (wchar_t *)malloc(BUFFER_SIZE * sizeof(wchar_t));
    if (!dir) return 1;

    DWORD len = GetModuleFileNameW(NULL, dir, BUFFER_SIZE);
    if (len == 0 || len >= BUFFER_SIZE) {
        MessageBoxW(NULL, L"Lỗi: Không thể xác định đường dẫn thư mục ứng dụng!", L"Lỗi khởi động", MB_ICONERROR | MB_OK);
        free(dir);
        return 1;
    }

    wchar_t *lastSlash = wcsrchr(dir, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0';
    }

    // 2. Tạo đường dẫn tới thư mục bin và các file cốt lõi
    wchar_t *binDir = (wchar_t *)malloc(BUFFER_SIZE * sizeof(wchar_t));
    wchar_t *exePath = (wchar_t *)malloc(BUFFER_SIZE * sizeof(wchar_t));
    wchar_t *platformDll = (wchar_t *)malloc(BUFFER_SIZE * sizeof(wchar_t));
    if (!binDir || !exePath || !platformDll) {
        free(dir);
        if (binDir) free(binDir);
        if (exePath) free(exePath);
        if (platformDll) free(platformDll);
        return 1;
    }

    lstrcpyW(binDir, dir);
    lstrcatW(binDir, L"bin");

    lstrcpyW(exePath, dir);
    lstrcatW(exePath, L"bin\\AnTamMng.exe");

    lstrcpyW(platformDll, dir);
    lstrcatW(platformDll, L"bin\\platforms\\qwindows.dll");

    // 3. Kiểm tra tính toàn vẹn của gói portable trước khi khởi chạy
    DWORD exeAttrib = GetFileAttributesW(exePath);
    if (exeAttrib == INVALID_FILE_ATTRIBUTES || (exeAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
        MessageBoxW(NULL, 
            L"Không tìm thấy file: bin\\AnTamMng.exe\n\n"
            L"Vui lòng đảm bảo bạn đã giải nén đầy đủ ứng dụng và không xóa hoặc di chuyển thư mục 'bin'!", 
            L"Lỗi khởi động AnTamMng", 
            MB_ICONERROR | MB_OK);
        free(dir); free(binDir); free(exePath); free(platformDll);
        return 1;
    }

    DWORD platAttrib = GetFileAttributesW(platformDll);
    if (platAttrib == INVALID_FILE_ATTRIBUTES || (platAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
        MessageBoxW(NULL, 
            L"Thiếu thư viện nền tảng: bin\\platforms\\qwindows.dll\n\n"
            L"Ứng dụng cần thư viện này để hiển thị giao diện trên Windows.\n"
            L"Vui lòng giải nén trọn vẹn toàn bộ thư mục của ứng dụng!", 
            L"Thiếu thành phần hệ thống", 
            MB_ICONWARNING | MB_OK);
    }

    // 4. Thiết lập biến môi trường cục bộ để ứng dụng hoạt động trên MỌI máy tính Windows khác:
    // a) Ưu tiên nạp DLL từ thư mục bin cục bộ trước mọi PATH hệ thống
    DWORD curPathLen = GetEnvironmentVariableW(L"PATH", NULL, 0);
    if (curPathLen > 0) {
        wchar_t *curPath = (wchar_t *)malloc((curPathLen + 1) * sizeof(wchar_t));
        if (curPath) {
            GetEnvironmentVariableW(L"PATH", curPath, curPathLen);
            wchar_t *newPath = (wchar_t *)malloc((curPathLen + lstrlenW(binDir) + 4) * sizeof(wchar_t));
            if (newPath) {
                lstrcpyW(newPath, binDir);
                lstrcatW(newPath, L";");
                lstrcatW(newPath, curPath);
                SetEnvironmentVariableW(L"PATH", newPath);
                free(newPath);
            }
            free(curPath);
        }
    } else {
        SetEnvironmentVariableW(L"PATH", binDir);
    }

    // b) Cố định đường dẫn Qt Plugins để chống xung đột với các phiên bản Qt khác trên máy người dùng
    SetEnvironmentVariableW(L"QT_PLUGIN_PATH", binDir);

    wchar_t *platformsDir = (wchar_t *)malloc(BUFFER_SIZE * sizeof(wchar_t));
    if (platformsDir) {
        lstrcpyW(platformsDir, dir);
        lstrcatW(platformsDir, L"bin\\platforms");
        SetEnvironmentVariableW(L"QT_QPA_PLATFORM_PLUGIN_PATH", platformsDir);
        free(platformsDir);
    }

    // 5. Khởi tạo tiến trình bằng CreateProcessW với Working Directory là thư mục gốc dự án
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t *cmdLine = (wchar_t *)malloc(BUFFER_SIZE * sizeof(wchar_t));
    if (!cmdLine) {
        free(dir); free(binDir); free(exePath); free(platformDll);
        return 1;
    }
    wsprintfW(cmdLine, L"\"%s\"", exePath);

    BOOL success = CreateProcessW(
        NULL,               // Application name
        cmdLine,            // Command line arguments
        NULL,               // Process attributes
        NULL,               // Thread attributes
        FALSE,              // Inherit handles
        0,                  // Creation flags
        NULL,               // Environment (inherits current process environment with our SetEnvironmentVariable)
        dir,                // Current working directory (root project dir)
        &si,                // Startup info
        &pi                 // Process information
    );

    if (!success) {
        DWORD err = GetLastError();
        wchar_t errMsg[512];
        wsprintfW(errMsg, 
            L"Không thể khởi động tiến trình bin\\AnTamMng.exe!\n\n"
            L"Mã lỗi Windows: %lu\n"
            L"Vui lòng thử chạy trực tiếp bằng quyền Administrator hoặc kiểm tra phần mềm diệt virus.", 
            err);
        MessageBoxW(NULL, errMsg, L"Lỗi khởi chạy", MB_ICONERROR | MB_OK);

        free(dir); free(binDir); free(exePath); free(platformDll); free(cmdLine);
        return 1;
    }

    // Đóng handle tiến trình và luồng để launcher kết thúc ngay lập tức, nhường tài nguyên cho ứng dụng chính
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    free(dir);
    free(binDir);
    free(exePath);
    free(platformDll);
    free(cmdLine);

    return 0;
}
