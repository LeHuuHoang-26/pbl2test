@echo off
setlocal
cd /d "%~dp0"

:: Thiet lap moi truong thu vien Qt6 va MinGW
set "PATH=C:\msys64\ucrt64\bin;%PATH%"

echo ========================================================
echo        KHOI DONG HE THONG QUAN LY PHONG KHAM AN TAM      
echo ========================================================
echo.

if exist "bin\AnTamMng.exe" (
    echo Dang khoi chay ung dung tu bin\AnTamMng.exe...
    start "" "bin\AnTamMng.exe"
    exit /b 0
)

if exist "AnTamMng.exe" (
    echo Dang khoi chay ung dung tu AnTamMng.exe...
    start "" "AnTamMng.exe"
    exit /b 0
)

echo [LOI] Khong tim thay file thuc thi!
echo He thong se tien hanh bien dich tu dong...
if exist "PBL_2.pro" (
    qmake PBL_2.pro
) else if exist "clinic.pro" (
    qmake clinic.pro
)
mingw32-make -j8
if %ERRORLEVEL% equ 0 (
    if exist "bin\AnTamMng.exe" (
        start "" "bin\AnTamMng.exe"
        exit /b 0
    )
)

echo.
echo [LOI] Khong the khoi chay ung dung. Vui long kiem tra lai trinh bien dich!
pause
exit /b 1
