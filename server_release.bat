@echo off

:: Check for Visual Studio
if exist "%VS100COMNTOOLS%" (
 set VSPATH="%VS100COMNTOOLS%..\..\VC\"
 goto set_c
)
if exist "%VS90COMNTOOLS%" (
 set VSPATH="%VS90COMNTOOLS%..\..\VC\"
 goto set_c
)
if exist "%VS80COMNTOOLS%" (
 set VSPATH="%VS80COMNTOOLS%..\..\VC\"
 goto set_c
)

:set_c
call %VSPATH%vcvarsall.bat x86
goto compile

@echo You need Microsoft Visual Studio 8, 9 or 10 installed
pause
exit

:compile
@echo === compiling teeworlds (server release) ===
@..\bam\bam server_release
@echo.
@pause