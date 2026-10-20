@echo off
setlocal
cd /d "%~dp0"

set "QT_BIN=C:\msys64\ucrt64\bin"
if not exist "%QT_BIN%\qmake6.exe" (
    echo Khong tim thay Qt 6 tai %QT_BIN%.
    echo Hay cai Qt 6 MinGW UCRT64 hoac sua duong dan QT_BIN trong run_qt.bat.
    pause
    exit /b 1
)

set "PATH=%QT_BIN%;%PATH%"
if not exist "build-qt" mkdir "build-qt"
"%QT_BIN%\qmake6.exe" clinic.pro -o build-qt\Makefile
if errorlevel 1 (
    pause
    exit /b 1
)
"%QT_BIN%\mingw32-make.exe" -C build-qt
if errorlevel 1 (
    pause
    exit /b 1
)

"build-qt\release\AnTamClinic.exe"
endlocal
