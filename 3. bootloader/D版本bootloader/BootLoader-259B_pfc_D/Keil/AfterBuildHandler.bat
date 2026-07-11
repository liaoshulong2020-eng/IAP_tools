@echo off
setlocal

cd /d "%~dp0"

set "tool_chain_inc=%~1"
shift /1

set "axf_full_path=%~1"
:collect_axf_path
shift /1
if "%~1"=="" goto axf_path_done
set "axf_full_path=%axf_full_path% %~1"
goto collect_axf_path

:axf_path_done
for %%A in ("%axf_full_path%") do (
    set "axf_name=%%~nA"
    set "axf_path=%%~dpA"
)

if "%tool_chain_inc%"=="" (
    echo Failed: missing compiler include path.
    exit /b 1
)

if "%axf_full_path%"=="" (
    echo Failed: missing axf path.
    exit /b 1
)

if "%tool_chain_inc:~-1%"=="\" (
    set "tool_chain_inc=%tool_chain_inc:~0,-1%"
)

for %%I in ("%tool_chain_inc%\..") do set "tool_chain_root=%%~fI"
set "fromelf=%tool_chain_root%\bin\fromelf.exe"

if not exist "%fromelf%" (
    echo Failed: fromelf not found: "%fromelf%"
    exit /b 1
)

"%fromelf%" --bin "%axf_full_path%" --output "%axf_path%%axf_name%.bin"
if errorlevel 1 (
    echo Failed: fromelf generate bin failed.
    exit /b 1
)

if not exist ".\Execute" mkdir ".\Execute"

copy /Y ".\Objects\%axf_name%.axf" ".\Execute\%axf_name%.axf" >nul
if exist ".\Objects\%axf_name%.hex" copy /Y ".\Objects\%axf_name%.hex" ".\Execute\%axf_name%.hex" >nul
if exist ".\Objects\%axf_name%.bin" copy /Y ".\Objects\%axf_name%.bin" ".\Execute\%axf_name%.bin" >nul

if exist ".\Patcher.exe" (
    ".\Patcher.exe" ".\Execute\%axf_name%.hex"
    if errorlevel 1 (
        echo Warning: Patcher failed, original hex/bin files were still copied.
    )
)

exit /b 0
