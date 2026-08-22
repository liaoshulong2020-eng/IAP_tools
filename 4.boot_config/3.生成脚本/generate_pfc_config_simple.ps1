# PFC Bootloader配置文件生成工具 - 简化版本

Write-Host "PFC Bootloader配置文件生成工具" -ForegroundColor Green
Write-Host "=================================================="

# 设置工作目录
Set-Location $PSScriptRoot

# 生成PFC BIN配置文件
Write-Host "`n1. 生成PFC BIN配置文件..." -ForegroundColor Yellow

$binPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.bin"
if (-not (Test-Path $binPath)) {
    $binPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.bin"
}

if (Test-Path $binPath) {
    try {
        $data = [System.IO.File]::ReadAllBytes($binPath)
        $fileName = Split-Path $binPath -Leaf
        
        # 生成C#代码
        $lines = @()
        $lines += "// $fileName ($($data.Length) bytes)"
        $lines += "private static readonly byte[] BOOTLOADER_5800_PFC_BIN_DATA = new byte[]"
        $lines += "{"
        
        # 每行16个字节
        for ($i = 0; $i -lt $data.Length; $i += 16) {
            $end = [Math]::Min($i + 15, $data.Length - 1)
            $chunk = $data[$i..$end]
            $hexValues = $chunk | ForEach-Object { "0x{0:X2}" -f $_ }
            $line = "    " + ($hexValues -join ", ")
            
            if ($i + 16 -lt $data.Length) {
                $line += ","
            }
            
            $lines += $line
        }
        
        $lines += "};"
        
        $outputPath = "BOOTLOADER_5800_PFC_BIN_DATA.cs"
        $content = $lines -join "`r`n"
        [System.IO.File]::WriteAllText($outputPath, $content, [System.Text.Encoding]::UTF8)
        
        Write-Host "成功生成: $outputPath" -ForegroundColor Green
        Write-Host "文件大小: $($data.Length) 字节" -ForegroundColor Green
    }
    catch {
        Write-Host "错误: $($_.Exception.Message)" -ForegroundColor Red
    }
} else {
    Write-Host "错误: 找不到PFC bootloader二进制文件" -ForegroundColor Red
}

# 生成PFC HEX配置文件
Write-Host "`n2. 生成PFC HEX配置文件..." -ForegroundColor Yellow

$hexPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.hex"
if (-not (Test-Path $hexPath)) {
    $hexPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.hex"
}

if (Test-Path $hexPath) {
    try {
        $hexContent = [System.IO.File]::ReadAllText($hexPath)
        $hexBytes = [System.Text.Encoding]::UTF8.GetBytes($hexContent)
        $fileName = Split-Path $hexPath -Leaf
        
        # 生成C#代码
        $lines = @()
        $lines += "// $fileName ($($hexBytes.Length) bytes)"
        $lines += "private static readonly byte[] BOOTLOADER_5800_PFC_HEX_DATA = new byte[]"
        $lines += "{"
        
        # 每行16个字节
        for ($i = 0; $i -lt $hexBytes.Length; $i += 16) {
            $end = [Math]::Min($i + 15, $hexBytes.Length - 1)
            $chunk = $hexBytes[$i..$end]
            $hexValues = $chunk | ForEach-Object { "0x{0:X2}" -f $_ }
            $line = "    " + ($hexValues -join ", ")
            
            if ($i + 16 -lt $hexBytes.Length) {
                $line += ","
            }
            
            $lines += $line
        }
        
        $lines += "};"
        
        $outputPath = "BOOTLOADER_5800_PFC_HEX_DATA.cs"
        $content = $lines -join "`r`n"
        [System.IO.File]::WriteAllText($outputPath, $content, [System.Text.Encoding]::UTF8)
        
        Write-Host "成功生成: $outputPath" -ForegroundColor Green
        Write-Host "文件大小: $($hexBytes.Length) 字节" -ForegroundColor Green
    }
    catch {
        Write-Host "错误: $($_.Exception.Message)" -ForegroundColor Red
    }
} else {
    Write-Host "错误: 找不到PFC bootloader HEX文件" -ForegroundColor Red
}

Write-Host "`n=================================================="
Write-Host "完成!" -ForegroundColor Cyan

# 检查生成的文件
if ((Test-Path "BOOTLOADER_5800_PFC_BIN_DATA.cs") -and (Test-Path "BOOTLOADER_5800_PFC_HEX_DATA.cs")) {
    Write-Host "`n✅ PFC配置支持已成功添加到boot_config目录" -ForegroundColor Green
    Write-Host "   - BOOTLOADER_5800_PFC_BIN_DATA.cs" -ForegroundColor Green
    Write-Host "   - BOOTLOADER_5800_PFC_HEX_DATA.cs" -ForegroundColor Green
} else {
    Write-Host "`n❌ 部分文件生成失败" -ForegroundColor Red
}

Write-Host "`n按任意键继续..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
