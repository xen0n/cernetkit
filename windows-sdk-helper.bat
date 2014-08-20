@echo off

rem This is a slightly adapted version of VCVarsQueryRegistry.bat from VS2010,
rem since there simply isn't any other way to obtain this information. :-/
rem (maybe I should change to :-\ to better signify the Windows thing)

setlocal
call :GetWindowsSdkDir
echo %WindowsSdkDir%
endlocal

goto end


:GetWindowsSdkDir
set WindowsSdkDir=
call :GetWindowsSdkDirHelper HKLM > nul 2>&1
if errorlevel 1 call :GetWindowsSdkDirHelper HKCU > nul 2>&1
if errorlevel 1 set WindowsSdkDir=%VCINSTALLDIR%\PlatformSDK\
exit /B 0

:GetWindowsSdkDirHelper
for /F "tokens=1,2*" %%i in ('reg query "%1\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A" /v "InstallationFolder"') DO (
    if "%%i"=="InstallationFolder" (
        set "WindowsSdkDir=%%k"
    )
)

if "%WindowsSdkDir%"=="" exit /B 1
exit /B 0


:end
