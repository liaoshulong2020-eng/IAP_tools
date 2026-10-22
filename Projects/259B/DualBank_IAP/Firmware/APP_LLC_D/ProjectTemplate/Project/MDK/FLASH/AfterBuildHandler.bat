@echo off
:: enter .bat folder
cd %~dp0
:: batch file path
set bat_path=%0
:: MDK $J KEIL compiler header file path
set tool_chain_inc=%1
:: MDK #L KEIL generated axf file full path
set axf_full_path=%2
:: get axf file name without extension
set axf_name=%~n2
:: project name
set project_name=259B-LLC

echo Debug: Parameters received:
echo   tool_chain_inc=%tool_chain_inc%
echo   axf_full_path=%axf_full_path%
echo   axf_name=%axf_name%
echo   project_name=%project_name%

if "%tool_chain_inc:~-1,1%" == "\" (
    :: remove last backslash
    set tool_chain_inc=%tool_chain_inc:~0,-1%
)

:: call .bin generate function
echo Step 1: Generating bin file...
call :binGenerate "%tool_chain_inc%" "%axf_full_path%"
if %errorlevel% == 1 (
    echo Failed 1: fromelf generate .bin failed!
    goto :EOF
)

:: call file copy
echo Step 2: Copying files...
call :doFileCopy "%axf_full_path%"

:: patch hex file and generate patched .bin and .hex
echo Step 3: Patching files...
call :doFilePatch "%axf_name%"
if %errorlevel% == 1 (
    echo Failed 2: Patch failed!
    goto :EOF
)

:: ensure bootloader files exist
echo Step 3.5: Ensuring BootLoader files exist...
call :ensureBootLoaderFiles
if %errorlevel% == 1 (
    echo Failed 3.5: BootLoader files preparation failed!
    goto :EOF
)

:: copy bin and hex files to IAP directories
echo Step 4: Copying to IAP directories...
call :copyToIapDirectories "%axf_name%"
if %errorlevel% == 1 (
    echo Failed 4: Copy to IAP directories failed!
    goto :EOF
)

:: merge bootloader, app, and hex files
echo Step 5: Merging files...
call :mergeIapFiles "%axf_name%"
if %errorlevel% == 1 (
    echo Failed 5: Merge IAP files failed!
    goto :EOF
)

:: copy generated jlink files back to Execute directory
echo Step 6: Copying jlink files back to Execute...
call :copyJlinkFilesToExecute "%axf_name%"
if %errorlevel% == 1 (
    echo Failed 6: Copy jlink files to Execute failed!
    goto :EOF
)

echo All steps completed successfully!
exit /b %errorlevel%

:: Function Definitions ------------------------------------------------

:: .bin generate function
:binGenerate
:: get toolchain root directory from header file path
set tool_chain_root=%~dp1
:: get axf path
set axf_path=%~dp2
:: get axf name
set axf_name=%~n2
:: execute fromelf to generate bin file
"%tool_chain_root%bin\fromelf" --bin "%~2" --output "%axf_path%%axf_name%.bin"
exit /b %errorlevel%

:: copy axf/hex/bin files to Execute folder
:doFileCopy
:: get axf name
set axf_name=%~n1
copy /Y ".\Objects\%axf_name%.axf" ".\Execute\%axf_name%.axf"
copy /Y ".\Objects\%axf_name%.hex" ".\Execute\%axf_name%.hex"
copy /Y ".\Objects\%axf_name%.bin" ".\Execute\%axf_name%.bin"
:: according to user config, hex may not be generated, ignore this and return success
exit /b 0

:: patch file
:doFilePatch
set target_name=%~1
"Patcher.exe" ".\Execute\%target_name%.hex"
exit /b %errorlevel%

:: ensure bootloader files exist (created by C# program)
:ensureBootLoaderFiles
set bootloader_dir=.\IAP\IapFileCreator\%project_name%
if not exist "%bootloader_dir%\BootLoader-5800.bin" (
    echo Warning: BootLoader-5800.bin not found, should be created by C# program
    exit /b 1
)
if not exist "%bootloader_dir%\BootLoader-5800.hex" (
    echo Warning: BootLoader-5800.hex not found, should be created by C# program  
    exit /b 1
)
echo BootLoader files are ready
exit /b 0

:: copy bin and hex files to IAP directories
:copyToIapDirectories
set target_name=%~1
set source_bin=.\Execute\%target_name%.bin
set source_hex=.\Execute\%target_name%.hex

echo Debug: copyToIapDirectories called with target_name=%target_name%

:: check if source files exist
if not exist "%source_bin%" (
    echo Error: Source bin file %source_bin% does not exist!
    exit /b 1
)

if not exist "%source_hex%" (
    echo Error: Source hex file %source_hex% does not exist!
    exit /b 1
)

:: target directory 1: IAP\IapFileCreator\%project_name%
set dest_dir1=.\IAP\IapFileCreator\%project_name%
if not exist "%dest_dir1%" (
    echo Creating directory: %dest_dir1%
    mkdir "%dest_dir1%"
)
echo Copying %source_bin% and %source_hex% to %dest_dir1%
copy /Y "%source_bin%" "%dest_dir1%\%target_name%.bin"
copy /Y "%source_hex%" "%dest_dir1%\%target_name%.hex"
if %errorlevel% neq 0 (
    echo Error: Failed to copy to %dest_dir1%
    exit /b 1
)

:: update JSON configuration files (both BIN and HEX)
call :updateJsonConfig "%dest_dir1%\%project_name%-BIN.json" "%target_name%"
call :updateHexJsonConfig "%dest_dir1%\%project_name%-HEX.json" "%target_name%"

:: target directory 2: IAP\IapTool\%project_name%
set dest_dir2=.\IAP\IapTool\%project_name%
if not exist "%dest_dir2%" (
    echo Creating directory: %dest_dir2%
    mkdir "%dest_dir2%"
)
echo Copying %source_bin% and %source_hex% to %dest_dir2%
copy /Y "%source_bin%" "%dest_dir2%\%target_name%.bin"
copy /Y "%source_hex%" "%dest_dir2%\%target_name%.hex"
if %errorlevel% neq 0 (
    echo Error: Failed to copy to %dest_dir2%
    exit /b 1
)

:: update JSON configuration files for IapTool (both BIN and HEX)
call :updateIapToolJsonConfig "%dest_dir2%\%project_name%-BIN.json" "%target_name%"
call :updateIapToolHexJsonConfig "%dest_dir2%\%project_name%-HEX.json" "%target_name%"

echo Successfully copied bin and hex files to IAP directories.
exit /b 0

:: update JSON configuration file
:updateJsonConfig
set json_file=%~1
set bin_name=%~2

:: create outFile name by replacing "iap" with "jlink"
set out_name=%bin_name:iap=jlink%

echo Updating JSON configuration: %json_file%
echo   appFile: %project_name%/%bin_name%.bin
echo   outFile: %project_name%/%out_name%.bin

:: check if JSON file exists
if not exist "%json_file%" (
    echo Warning: JSON file %json_file% does not exist, creating new one...
    call :createNewJsonConfig "%json_file%" "%bin_name%" "%out_name%"
    exit /b 0
)

:: create temporary file for JSON update
set temp_file=%json_file%.tmp

:: read and update JSON file
(
    for /f "delims=" %%i in ('type "%json_file%"') do (
        set line=%%i
        setlocal enabledelayedexpansion
        if "!line:appFile=!" neq "!line!" (
            echo   "appFile": "%project_name%/%bin_name%.bin",
        ) else if "!line:outFile=!" neq "!line!" (
            echo   "outFile": "%project_name%/%out_name%.bin",
        ) else (
            echo !line!
        )
        endlocal
    )
) > "%temp_file%"

:: replace original file with updated file
move "%temp_file%" "%json_file%"
echo JSON configuration updated successfully.
exit /b 0

:: create new JSON configuration file
:createNewJsonConfig
set json_file=%~1
set bin_name=%~2
set out_name=%~3

(
echo {
echo   "bootFile": "%project_name%/BootLoader-5800.bin",
echo   "appFile": "%project_name%/%bin_name%.bin",
echo   "outFile": "%project_name%/%out_name%.bin",
echo   "bootMaxSize": 32768,
echo   "argBaseAddr": 28672,
echo   "appBaseAddr": 32768,
echo   "appMaxSize": 229376
echo }
) > "%json_file%"

echo Created new JSON configuration file: %json_file%
exit /b 0

:: update HEX JSON configuration file
:updateHexJsonConfig
set json_file=%~1
set hex_name=%~2

:: create outFile name by replacing "iap" with "jlink"
set out_name=%hex_name:iap=jlink%

echo Updating HEX JSON configuration: %json_file%
echo   appFile: %project_name%/%hex_name%.hex
echo   outFile: %project_name%/%out_name%.hex

:: check if JSON file exists
if not exist "%json_file%" (
    echo Warning: HEX JSON file %json_file% does not exist, creating new one...
    call :createNewHexJsonConfig "%json_file%" "%hex_name%" "%out_name%"
    exit /b 0
)

:: create temporary file for JSON update
set temp_file=%json_file%.tmp

:: read and update JSON file
(
    for /f "delims=" %%i in ('type "%json_file%"') do (
        set line=%%i
        setlocal enabledelayedexpansion
        if "!line:appFile=!" neq "!line!" (
            echo   "appFile": "%project_name%/%hex_name%.hex",
        ) else if "!line:outFile=!" neq "!line!" (
            echo   "outFile": "%project_name%/%out_name%.hex",
        ) else (
            echo !line!
        )
        endlocal
    )
) > "%temp_file%"

:: replace original file with updated file
move "%temp_file%" "%json_file%"
echo HEX JSON configuration updated successfully.
exit /b 0

:: create new HEX JSON configuration file
:createNewHexJsonConfig
set json_file=%~1
set hex_name=%~2
set out_name=%~3

(
echo {
echo   "bootFile": "%project_name%/BootLoader-5800.hex",
echo   "appFile": "%project_name%/%hex_name%.hex",
echo   "outFile": "%project_name%/%out_name%.hex",
echo   "bootMaxSize": 32768,
echo   "argBaseAddr": 28672,
echo   "appBaseAddr": 32768,
echo   "appMaxSize": 229376,
echo   "flashBaseAddr": 134217728
echo }
) > "%json_file%"

echo Created new HEX JSON configuration file: %json_file%
exit /b 0

:: update IapTool JSON configuration file
:updateIapToolJsonConfig
set json_file=%~1
set bin_name=%~2

echo Updating IapTool JSON configuration: %json_file%
echo   appFile: %project_name%/%bin_name%.bin

:: check if JSON file exists
if not exist "%json_file%" (
    echo Warning: JSON file %json_file% does not exist, creating new one...
    call :createNewIapToolJsonConfig "%json_file%" "%bin_name%"
    exit /b 0
)

:: create temporary file for JSON update
set temp_file=%json_file%.tmp

:: read and update JSON file
(
    for /f "delims=" %%i in ('type "%json_file%"') do (
        set line=%%i
        setlocal enabledelayedexpansion
        if "!line:appFile=!" neq "!line!" (
            echo   "appFile": "%project_name%/%bin_name%.bin",
        ) else (
            echo !line!
        )
        endlocal
    )
) > "%temp_file%"

:: replace original file with updated file
move "%temp_file%" "%json_file%"
echo IapTool JSON configuration updated successfully.
exit /b 0

:: create new IapTool JSON configuration file
:createNewIapToolJsonConfig
set json_file=%~1
set bin_name=%~2

(
echo {
echo   "bootFile": "%project_name%/BootLoader-5800.bin",
echo   "appFile": "%project_name%/%bin_name%.bin",
echo   "bootMaxSize": 32768,
echo   "argBaseAddr": 28672,
echo   "appBaseAddr": 32768,
echo   "appMaxSize": 229376
echo }
) > "%json_file%"

echo Created new IapTool JSON configuration file: %json_file%
exit /b 0

:: update IapTool HEX JSON configuration file
:updateIapToolHexJsonConfig
set json_file=%~1
set hex_name=%~2

echo Updating IapTool HEX JSON configuration: %json_file%
echo   appFile: %project_name%/%hex_name%.hex

:: check if JSON file exists
if not exist "%json_file%" (
    echo Warning: HEX JSON file %json_file% does not exist, creating new one...
    call :createNewIapToolHexJsonConfig "%json_file%" "%hex_name%"
    exit /b 0
)

:: create temporary file for JSON update
set temp_file=%json_file%.tmp

:: read and update JSON file
(
    for /f "delims=" %%i in ('type "%json_file%"') do (
        set line=%%i
        setlocal enabledelayedexpansion
        if "!line:appFile=!" neq "!line!" (
            echo   "appFile": "%project_name%/%hex_name%.hex",
        ) else (
            echo !line!
        )
        endlocal
    )
) > "%temp_file%"

:: replace original file with updated file
move "%temp_file%" "%json_file%"
echo IapTool HEX JSON configuration updated successfully.
exit /b 0

:: create new IapTool HEX JSON configuration file
:createNewIapToolHexJsonConfig
set json_file=%~1
set hex_name=%~2

(
echo {
echo   "bootFile": "%project_name%/BootLoader-5800.hex",
echo   "appFile": "%project_name%/%hex_name%.hex",
echo   "bootMaxSize": 32768,
echo   "argBaseAddr": 28672,
echo   "appBaseAddr": 32768,
echo   "appMaxSize": 229376,
echo   "flashBaseAddr": 134217728
echo }
) > "%json_file%"

echo Created new IapTool HEX JSON configuration file: %json_file%
exit /b 0

:: merge bootloader, app, and hex files
:mergeIapFiles
set app_name=%~1
echo Debug: mergeIapFiles called with app_name=%app_name%
set iap_dir=.\IAP\IapFileCreator\%project_name%
set bootloader_bin_file=%iap_dir%\BootLoader-5800.bin
set bootloader_hex_file=%iap_dir%\BootLoader-5800.hex
set app_bin_file=%iap_dir%\%app_name%.bin
set app_hex_file=%iap_dir%\%app_name%.hex
set json_bin_file=%iap_dir%\%project_name%-BIN.json
set json_hex_file=%iap_dir%\%project_name%-HEX.json
set ps_script=%~dp0MergeIapFiles.ps1

echo Merging bootloader, app, and hex files...
echo   bootloader_bin_file=%bootloader_bin_file%
echo   bootloader_hex_file=%bootloader_hex_file%
echo   app_bin_file=%app_bin_file%
echo   app_hex_file=%app_hex_file%
echo   json_bin_file=%json_bin_file%
echo   json_hex_file=%json_hex_file%

:: check if PowerShell script exists
if not exist "%ps_script%" (
    echo Error: PowerShell script not found: %ps_script%
    echo Please ensure MergeIapFiles.ps1 is in the same directory as this batch file.
    exit /b 1
)

:: ========== Process BIN files ==========
echo.
echo ========== Processing BIN files ==========

:: check if BIN files exist
if not exist "%bootloader_bin_file%" (
    echo Error: Bootloader BIN file not found: %bootloader_bin_file%
    exit /b 1
)

if not exist "%app_bin_file%" (
    echo Error: App BIN file not found: %app_bin_file%
    exit /b 1
)

if not exist "%json_bin_file%" (
    echo Error: BIN JSON config file not found: %json_bin_file%
    exit /b 1
)

:: call PowerShell script to merge BIN files
echo Calling PowerShell script to merge BIN files...
powershell -ExecutionPolicy Bypass -File "%ps_script%" -JsonConfigPath "%json_bin_file%" -BootloaderPath "%bootloader_bin_file%" -AppPath "%app_bin_file%" -HexPath "%app_hex_file%"

if %errorlevel% neq 0 (
    echo Error: PowerShell BIN merge operation failed
    exit /b 1
)

echo Successfully merged BIN files.

:: ========== Process HEX files ==========
echo.
echo ========== Processing HEX files ==========

:: check if HEX files exist
if not exist "%bootloader_hex_file%" (
    echo Warning: Bootloader HEX file not found: %bootloader_hex_file%
    echo Skipping HEX file merge...
    goto :skipHexMerge
)

if not exist "%app_hex_file%" (
    echo Warning: App HEX file not found: %app_hex_file%
    echo Skipping HEX file merge...
    goto :skipHexMerge
)

if not exist "%json_hex_file%" (
    echo Warning: HEX JSON config file not found: %json_hex_file%
    echo Skipping HEX file merge...
    goto :skipHexMerge
)

:: call PowerShell script to merge HEX files
echo Calling PowerShell script to merge HEX files...
powershell -ExecutionPolicy Bypass -File "%ps_script%" -JsonConfigPath "%json_hex_file%" -BootloaderPath "%bootloader_hex_file%" -AppPath "%app_hex_file%" -HexPath "%app_hex_file%"

if %errorlevel% neq 0 (
    echo Error: PowerShell HEX merge operation failed
    exit /b 1
)

echo Successfully merged HEX files.
goto :endHexMerge

:skipHexMerge
echo HEX file merge was skipped due to missing files.

:endHexMerge
echo.
echo ========== Merge operations completed ==========
echo Successfully processed both BIN and HEX files (where available).
exit /b 0

:: copy generated jlink files back to Execute directory
:copyJlinkFilesToExecute
set app_name=%~1
echo Debug: copyJlinkFilesToExecute called with app_name=%app_name%

:: create jlink file names by replacing "iap" with "jlink"
set jlink_name=%app_name:iap=jlink%

set iap_creator_dir=.\IAP\IapFileCreator\%project_name%
set execute_dir=.\Execute

:: source files (generated jlink files)
set jlink_bin_source=%iap_creator_dir%\%jlink_name%.bin
set jlink_hex_source=%iap_creator_dir%\%jlink_name%.hex

:: destination files
set jlink_bin_dest=%execute_dir%\%jlink_name%.bin
set jlink_hex_dest=%execute_dir%\%jlink_name%.hex

echo Copying generated jlink files to Execute directory...
echo   jlink_name=%jlink_name%
echo   Source directory: %iap_creator_dir%
echo   Destination directory: %execute_dir%

:: ensure Execute directory exists
if not exist "%execute_dir%" (
    echo Creating Execute directory: %execute_dir%
    mkdir "%execute_dir%"
)

:: copy BIN file if it exists
if exist "%jlink_bin_source%" (
    echo Copying jlink BIN file: %jlink_bin_source% -> %jlink_bin_dest%
    copy /Y "%jlink_bin_source%" "%jlink_bin_dest%"
    if %errorlevel% neq 0 (
        echo Error: Failed to copy jlink BIN file to Execute directory
        exit /b 1
    )
    echo Successfully copied jlink BIN file.
) else (
    echo Warning: Jlink BIN file not found: %jlink_bin_source%
)

:: copy HEX file if it exists
if exist "%jlink_hex_source%" (
    echo Copying jlink HEX file: %jlink_hex_source% -> %jlink_hex_dest%
    copy /Y "%jlink_hex_source%" "%jlink_hex_dest%"
    if %errorlevel% neq 0 (
        echo Error: Failed to copy jlink HEX file to Execute directory
        exit /b 1
    )
    echo Successfully copied jlink HEX file.
) else (
    echo Warning: Jlink HEX file not found: %jlink_hex_source%
)

:: check if at least one file was copied
if not exist "%jlink_bin_dest%" if not exist "%jlink_hex_dest%" (
    echo Error: No jlink files were found to copy back to Execute directory
    echo Expected files:
    echo   %jlink_bin_source%
    echo   %jlink_hex_source%
    exit /b 1
)

echo Successfully copied available jlink files back to Execute directory.
exit /b 0