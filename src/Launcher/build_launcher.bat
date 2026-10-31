@echo off
setlocal
cd /d "%~dp0"
echo ========================================================
echo       BIEN DICH PORTABLE C LAUNCHER (AnTamMng.exe)
echo ========================================================
echo.

set "PATH=C:\msys64\ucrt64\bin;%PATH%"

echo 1. Dang bien dich resource icon...
windres launcher.rc -o launcher_res.o
if errorlevel 1 (
    echo [LOI] Khong the bien dich resource icon!
    pause
    exit /b 1
)

echo 2. Dang bien dich launcher C thuan (static runtime, khong phu thuoc DLL)...
gcc -O2 -s -mwindows -static launcher.c launcher_res.o -o ../../AnTamMng.exe
if errorlevel 1 (
    echo [LOI] Bien dich that bai!
    del launcher_res.o >nul 2>&1
    pause
    exit /b 1
)

del launcher_res.o >nul 2>&1
echo.
echo [OK] Hoan tat! Da tao thanh cong file launcher doc lap tai thu muc goc: AnTamMng.exe
echo File nay khong phu thuoc bat ky DLL ngoai nao va hoat dong tren moi may tinh Windows.
exit /b 0
