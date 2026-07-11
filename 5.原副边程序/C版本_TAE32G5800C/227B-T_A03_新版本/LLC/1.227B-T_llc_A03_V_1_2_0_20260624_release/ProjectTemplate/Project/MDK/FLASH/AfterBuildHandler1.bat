@echo off
:: enter .bat folder
cd %~dp0
:: 批处理所在路径
set bat_path=%0
:: MDK $J 这里传入的是KEIL 编译器头文件路径，利用这个路径找到编译器相关工具链地址
set tool_chain_inc=%1
:: MDK #L 这里传入的是KEIL生成的axf文件的完整路径
set axf_full_path=%2
:: 目标IAP工具目录（相对路径）
set iap_tool_dir=..\..\..\..\..\..\iap\IAP-tools\IAP-tools\IapTool\227B-LLC
:: 获取axf文件的名字，不含后缀
set axf_name=%~n2
if %tool_chain_inc:~-1,1% == \ (
    :: 删除路径最后的\
    set tool_chain_inc=%tool_chain_inc:~,-1%
)
:: call .bin generate function
call :binGenerate %tool_chain_inc% %axf_full_path%
if %errorlevel% == 1 (
    echo Failed 1: fromelf generate .bin failed!
    goto :EOF
)
:: call 文件复制
call :doFileCopy %axf_full_path%
:: 对hex文件进行Patch并生成patch后的.bin和.hex
call :doFilePatch %axf_name%
if %errorlevel% == 1 (
    echo Failed 2: Patch failed!
    goto :EOF
)
:: 复制bin文件到IAP工具目录
call :copyToIapTools %axf_name%
:: 更新JSON文件
call :updateJsonFile %axf_name%
exit /b %errorlevel%

:: Function Definiations ------------------------------------------------
:: .bin generate function
:binGenerate
:: 通过头文件路径，获取工具链的根目录
set tool_chain_root=%~dp1
:: 获取axf的路径
set axf_path=%~dp2
:: 获取axf的名字
set axf_name=%~n2
:: 执行fromelf 生成bin文件
%tool_chain_root%bin\fromelf --bin %2 --output %axf_path%\%axf_name%.bin
exit /b %errorlevel%

:: 将axf/hex/bin文件复制到Execute文件夹下
:doFileCopy
:: 获取axf的名字
set axf_name=%~n1
copy /Y .\Objects\%axf_name%.axf .\Execute\%axf_name%.axf
copy /Y .\Objects\%axf_name%.hex .\Execute\%axf_name%.hex
copy /Y .\Objects\%axf_name%.bin .\Execute\%axf_name%.bin
:: 根据用户的配置，可能hex不生成，不管这个，直接返回成功
exit /b 0

:: 对文件进行Patch
:doFilePatch
set target_name=%1
Patcher.exe .\Execute\%target_name%.hex
exit /b %errorlevel%

:: 复制bin文件到IAP工具目录
:copyToIapTools
set target_name=%1
copy /Y .\Execute\%target_name%.bin "%iap_tool_dir%\%target_name%.bin"
echo Copied %target_name%.bin to IAP tools directory
exit /b 0

:: 更新JSON文件
:updateJsonFile
set target_name=%1
set "jsonFile=%iap_tool_dir%\227B-LLC.json"
set "tempFile=%iap_tool_dir%\227B-LLC_temp.json"

:: 使用PowerShell读取和替换JSON内容
powershell -Command "$json = Get-Content '%jsonFile%' | ConvertFrom-Json; $json.appFile = '227B-LLC/%target_name%.bin'; $json | ConvertTo-Json | Set-Content '%tempFile%'"

:: 替换原文件
move /Y "%tempFile%" "%jsonFile%"
echo Updated JSON file with new bin filename: %target_name%.bin
exit /b 0