if "%1"=="" (set n=16) else (set n=%1)
for /l %%i in (1,1,%n%) do (echo === test round %%i:
	call runTests.bat
)