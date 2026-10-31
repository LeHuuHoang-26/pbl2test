@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

echo ===============================================================================
echo        TU DONG TAO BAN PORTABLE HOAN CHINH - PHONG KHAM AN TAM (AnTamMng)
echo ===============================================================================
echo.

REM 1. Thiet lap moi truong MinGW va Qt
set "QT_BIN=C:\msys64\ucrt64\bin"
if exist "%QT_BIN%" (
    set "PATH=%QT_BIN%;%PATH%"
)

where gcc >nul 2>&1
if errorlevel 1 (
    echo [LOI] Khong tim thay trinh bien dich GCC trong PATH hoac C:\msys64\ucrt64\bin!
    pause
    exit /b 1
)

REM 2. Kiem tra va bien dich ung dung chinh
echo [BUOC 1/4] Kiem tra va bien dich ung dung chinh Qt...
if not exist "bin\AnTamMng.exe" (
    echo Chua co bin\AnTamMng.exe, tien hanh bien dich tu ma nguon...
    if exist "PBL_2.pro" (
        qmake PBL_2.pro
    ) else if exist "clinic.pro" (
        qmake clinic.pro
    )
    mingw32-make -j8
    if errorlevel 1 (
        echo [LOI] Qua trinh bien dich ma nguon that bai!
        pause
        exit /b 1
    )
) else (
    echo [OK] Da co san file thuc thi loi: bin\AnTamMng.exe
)

REM 3. Dong bo thu vien DLL va Qt Plugins
echo.
echo [BUOC 2/4] Kiem tra thu vien runtime va plugins cho moi truong Portable...
if not exist "bin" mkdir "bin"
if not exist "bin\platforms" mkdir "bin\platforms"

where windeployqt >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Dang dong bo Qt plugins bang windeployqt...
    windeployqt --no-compiler-runtime --no-translations bin\AnTamMng.exe >nul 2>&1
)

for %%D in (libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    if not exist "bin\%%D" (
        if exist "%QT_BIN%\%%D" (
            echo Copy %%D vao bin\...
            copy "%QT_BIN%\%%D" "bin\%%D" >nul
        )
    )
)

if not exist "bin\platforms\qwindows.dll" (
    if exist "%QT_BIN%\..\share\qt6\plugins\platforms\qwindows.dll" (
        copy "%QT_BIN%\..\share\qt6\plugins\platforms\qwindows.dll" "bin\platforms\qwindows.dll" >nul
    ) else if exist "%QT_BIN%\platforms\qwindows.dll" (
        copy "%QT_BIN%\platforms\qwindows.dll" "bin\platforms\qwindows.dll" >nul
    )
)

REM 4. Bien dich Portable C Launcher tai thu muc goc
echo.
echo [BUOC 3/4] Bien dich Portable C Launcher (AnTamMng.exe o thu muc goc)...
cd src\Launcher
windres launcher.rc -o launcher_res.o
gcc -O2 -s -mwindows -static launcher.c launcher_res.o -o ../../AnTamMng.exe
if errorlevel 1 (
    echo [LOI] Bien dich launcher C that bai!
    del launcher_res.o >nul 2>&1
    cd /d "%~dp0"
    pause
    exit /b 1
)
del launcher_res.o >nul 2>&1
cd /d "%~dp0"

echo [OK] Da tao thanh cong launcher doc lap: AnTamMng.exe

REM 5. Kiem tra tinh toan ven toan bo he thong
echo.
echo [BUOC 4/4] Kiem tra tinh toan ven cua goi Portable...
set "CHECK_FAIL=0"

if not exist "AnTamMng.exe" (
    echo [THIEU] AnTamMng.exe o thu muc goc
    set "CHECK_FAIL=1"
)
if not exist "bin\AnTamMng.exe" (
    echo [THIEU] bin\AnTamMng.exe
    set "CHECK_FAIL=1"
)
if not exist "bin\platforms\qwindows.dll" (
    echo [CANH BAO] bin\platforms\qwindows.dll chua co!
)

if "%CHECK_FAIL%"=="0" (
    echo.
    echo ===============================================================================
    echo      TAO BAN PORTABLE THANH CONG VA SAN SANG CHAY TREN MOI MAY WINDOWS!
    echo ===============================================================================
    echo.
    echo Huong dan su dung:
    echo   1. Nhap dup chuot truc tiep vao file [AnTamMng.exe] o thu muc goc de khoi chay.
    echo   2. Khi chia se sang may tinh khac: Chi can copy toan bo thu muc pbl2_v1.1 nay.
    echo   3. Launcher C thuan tu dong thiet lap moi truong co lap va nap plugins,
    echo      dam bao khong bao gio bi loi thieu DLL hay xung dot phien ban Qt.
    echo.
) else (
    echo.
    echo [CANH BAO] Ban build co mot so thanh phan chua hoan chinh.
)

pause
