$ErrorActionPreference = 'Stop'

# 目录约定：本脚本位于 源代码 下，输出到同级的 应用程序
$srcRoot = $PSScriptRoot
$projRoot = Split-Path -Parent $srcRoot
$appDir = Join-Path $projRoot '应用程序'
$buildDir = Join-Path $env:TEMP 'hld_canfd_dualbank_build'
$stageDir = Join-Path $buildDir 'deploy'

# Qt 工具链（可用环境变量 HLD_QT_ROOT / HLD_MINGW_ROOT 覆盖）
$QT_ROOT = if ($env:HLD_QT_ROOT) { $env:HLD_QT_ROOT } else { Join-Path $projRoot '.qt\6.8.3\mingw_64' }
$MINGW_ROOT = if ($env:HLD_MINGW_ROOT) { $env:HLD_MINGW_ROOT } else { Join-Path $projRoot '.qt\Tools\mingw1310_64' }

$QMAKE = Join-Path $QT_ROOT 'bin\qmake.exe'
$WINDEPLOYQT = Join-Path $QT_ROOT 'bin\windeployqt.exe'
if (-not (Test-Path -LiteralPath $QMAKE)) { throw "未找到 qmake：$QMAKE" }
if (-not (Test-Path -LiteralPath $WINDEPLOYQT)) { throw "未找到 windeployqt：$WINDEPLOYQT" }

# 原生工具（qmake/make/windeployqt）对中文路径兼容性差，这里用 ASCII 目录联接规避
$srcJunction = Join-Path $env:TEMP 'hld_canfd_dualbank_src'
if (-not (Test-Path -LiteralPath $srcJunction)) {
    New-Item -ItemType Junction -Path $srcJunction -Target $srcRoot | Out-Null
}

$env:PATH = "$MINGW_ROOT\bin;$QT_ROOT\bin;$env:PATH"

# 1) 编译
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
Set-Location $buildDir
& $QMAKE (Join-Path $srcJunction 'HLD_CANFDToolPro.pro') 'CONFIG+=release'
if ($LASTEXITCODE -ne 0) { throw "qmake 失败" }
& mingw32-make -j4
if ($LASTEXITCODE -ne 0) { throw "编译失败" }

$builtExe = Join-Path $buildDir 'release\HLD_CANFDToolPro.exe'
if (-not (Test-Path -LiteralPath $builtExe)) { throw "未找到编译产物：$builtExe" }

# 2) 在 ASCII 暂存目录里完成部署，最后再整体复制到中文目录
if (Test-Path -LiteralPath $stageDir) { Remove-Item -LiteralPath $stageDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stageDir | Out-Null

$stageExe = Join-Path $stageDir 'HLD_CANFDToolPro.exe'
Copy-Item -LiteralPath $builtExe -Destination $stageExe

& $WINDEPLOYQT --release --compiler-runtime --no-translations $stageExe
if ($LASTEXITCODE -ne 0) { throw "windeployqt 失败" }

$controlSrc = Join-Path $srcRoot 'ControlCANFD.dll'
if (Test-Path -LiteralPath $controlSrc) {
    Copy-Item -LiteralPath $controlSrc -Destination (Join-Path $stageDir 'ControlCANFD.dll') -Force
} else {
    Write-Warning "未找到 ControlCANFD.dll（$controlSrc），请手动复制到应用程序"
}

# 3) 落地到应用程序（保留用户 config/setdb.txt）
$configFile = Join-Path $appDir 'config\setdb.txt'
$configBackupFile = Join-Path $env:TEMP 'hld_canfd_setdb.txt.backup'

# 若上次构建异常退出留下了备份且当前没有配置，先恢复
if (-not (Test-Path -LiteralPath $configFile) -and (Test-Path -LiteralPath $configBackupFile)) {
    New-Item -ItemType Directory -Force -Path (Join-Path $appDir 'config') | Out-Null
    Copy-Item -LiteralPath $configBackupFile -Destination $configFile -Force
}

# 保存本次用户配置
if (Test-Path -LiteralPath $configFile) {
    Copy-Item -LiteralPath $configFile -Destination $configBackupFile -Force
}

if (Test-Path -LiteralPath $appDir) { Remove-Item -LiteralPath $appDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $appDir | Out-Null
Copy-Item -Path (Join-Path $stageDir '*') -Destination $appDir -Recurse -Force

# 恢复用户配置
New-Item -ItemType Directory -Force -Path (Join-Path $appDir 'config') | Out-Null
if (Test-Path -LiteralPath $configBackupFile) {
    Copy-Item -LiteralPath $configBackupFile -Destination $configFile -Force
}

Write-Host ""
Write-Host "构建并部署完成：$appDir" -ForegroundColor Green
