@echo off
echo PFC Bootloader配置文件生成工具
echo ================================================

cd /d "%~dp0"

echo.
echo 正在生成PFC配置文件...

:: 检查Python是否可用
python --version >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到Python，请确保Python已安装并添加到PATH
    pause
    exit /b 1
)

:: 运行Python脚本
python generate_pfc_config.py

if errorlevel 1 (
    echo.
    echo 错误: 脚本执行失败
    pause
    exit /b 1
)

echo.
echo 检查生成的文件...
if exist "BOOTLOADER_5800_PFC_BIN_DATA.cs" (
    echo ✓ BOOTLOADER_5800_PFC_BIN_DATA.cs 已生成
) else (
    echo ✗ BOOTLOADER_5800_PFC_BIN_DATA.cs 生成失败
)

if exist "BOOTLOADER_5800_PFC_HEX_DATA.cs" (
    echo ✓ BOOTLOADER_5800_PFC_HEX_DATA.cs 已生成
) else (
    echo ✗ BOOTLOADER_5800_PFC_HEX_DATA.cs 生成失败
)

echo.
echo 完成!
pause
