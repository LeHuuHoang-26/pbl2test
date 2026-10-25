@echo off
cd /d "%~dp0"
echo Dang bien dich Portable Launcher...
windres launcher.rc -o launcher_res.o
gcc -O2 -s -mwindows launcher.c launcher_res.o -o ../../AnTamMng.exe
if %ERRORLEVEL% equ 0 (
    del launcher_res.o
    echo Hoan tat! Da cap nhat file AnTamMng.exe o thu muc goc.
) else (
    echo Co loi xay ra khi bien dich!
)
