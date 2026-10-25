@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

:: Thiet lap moi truong Qt 6 va MinGW
set "QT_BIN=C:\msys64\ucrt64\bin"
if exist "%QT_BIN%" (
    set "PATH=%QT_BIN%;%PATH%"
)

echo ========================================================
echo        KHOI DONG HE THONG PHONG KHAM AN TAM (AnTamMng)
echo ========================================================
echo.

:: 1. Uu tien khoi chay ung dung da bien dich trong bin\AnTamMng.exe
if exist "bin\AnTamMng.exe" (
    echo [OK] Dang khoi chay ung dung tu bin\AnTamMng.exe...
    start "" "bin\AnTamMng.exe"
    exit /b 0
)

:: 2. Khoi chay tu launcher di dong AnTamMng.exe neu co
if exist "AnTamMng.exe" (
    echo [OK] Dang khoi chay ung dung tu AnTamMng.exe...
    start "" "AnTamMng.exe"
    exit /b 0
)

:: 3. Neu chua co file thuc thi, tien hanh bien dich tu dong
echo [THONG BAO] Chua tim thay file thuc thi, dang tien hanh bien dich...
set "QMAKE_CMD=qmake"
where qmake >nul 2>&1
if errorlevel 1 (
    if exist "%QT_BIN%\qmake.exe" (
        set "QMAKE_CMD=%QT_BIN%\qmake.exe"
    ) else if exist "%QT_BIN%\qmake6.exe" (
        set "QMAKE_CMD=%QT_BIN%\qmake6.exe"
    ) else (
        echo [LOI] Khong tim thay trinh bien dich qmake!
        echo Vui long kiem tra lai duong dan C:\msys64\ucrt64\bin
        pause
        exit /b 1
    )
)

if exist "PBL_2.pro" (
    echo Dang tao Makefile voi PBL_2.pro...
    "%QMAKE_CMD%" PBL_2.pro
) else if exist "clinic.pro" (
    echo Dang tao Makefile voi clinic.pro...
    "%QMAKE_CMD%" clinic.pro
)

echo Dang bien dich ma nguon...
mingw32-make -j8
if errorlevel 1 (
    echo.
    echo [LOI] Qua trinh bien dich gap loi. Vui long kiem tra lai!
    pause
    exit /b 1
)

:: Khoi chay sau khi bien dich thanh cong
if exist "bin\AnTamMng.exe" (
    echo [OK] Bien dich thanh cong! Dang khoi chay...
    start "" "bin\AnTamMng.exe"
    exit /b 0
)

echo.
echo [LOI] Khong tim thay file ung dung sau khi bien dich.
pause
exit /b 1
