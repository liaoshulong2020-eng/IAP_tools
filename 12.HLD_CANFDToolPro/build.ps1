# HLD_CANFDToolPro 一键构建脚本（qmake + MinGW）
# 用法：在 PowerShell 中执行  .\build.ps1
# 默认使用工程根目录下的 .qt 工具链，可按需通过环境变量覆盖。

$ErrorActionPreference = 'Stop'

# ---- 按需修改这里的路径 ----
$QT_ROOT = if ($env:HLD_QT_ROOT) { $env:HLD_QT_ROOT } else { Join-Path $PSScriptRoot '.qt\6.8.3\mingw_64' }
$MINGW_ROOT = if ($env:HLD_MINGW_ROOT) { $env:HLD_MINGW_ROOT } else { Join-Path $PSScriptRoot '.qt\Tools\mingw1310_64' }
# -----------------------------

$QMAKE = Join-Path $QT_ROOT 'bin\qmake.exe'
if (-not (Test-Path $QMAKE)) { throw "未找到 qmake：$QMAKE" }

$env:PATH = "$MINGW_ROOT\bin;$QT_ROOT\bin;$env:PATH"

$buildDir = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
Set-Location $buildDir

& $QMAKE '..\HLD_CANFDToolPro.pro' 'CONFIG+=release'
if ($LASTEXITCODE -ne 0) { throw "qmake 失败" }

& mingw32-make -j4
if ($LASTEXITCODE -ne 0) { throw "编译失败" }

Write-Host ""
Write-Host "编译完成：$buildDir\release\HLD_CANFDToolPro.exe" -ForegroundColor Green
Write-Host "运行前请把 64 位 ControlCANFD.dll 放到 exe 同目录。" -ForegroundColor Yellow
