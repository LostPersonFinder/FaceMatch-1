FOR /f %%i IN ('dir /b /s c:..\ *.bz2') DO "bunzip2-102-x86-win32.exe" %%i 
pause
