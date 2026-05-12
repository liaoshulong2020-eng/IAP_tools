# PFC Bootloader配置文件生成工具
# 用于将PFC bootloader的bin和hex文件转换为C#数组格式

Write-Host "PFC Bootloader配置文件生成工具" -ForegroundColor Green
Write-Host "=" * 50

# 设置工作目录
Set-Location $PSScriptRoot

# 函数：将字节数组转换为C#数组格式
function ConvertTo-CSharpArray {
    param(
        [byte[]]$Data,
        [string]$ArrayName,
        [string]$Comment = ""
    )
    
    $lines = @()
    
    if ($Comment) {
        $lines += "// $Comment"
    }
    
    $lines += "private static readonly byte[] $ArrayName = new byte[]"
    $lines += "{"
    
    # 每行16个字节
    for ($i = 0; $i -lt $Data.Length; $i += 16) {
        $chunk = $Data[$i..([Math]::Min($i + 15, $Data.Length - 1))]
        $hexValues = $chunk | ForEach-Object { "0x{0:X2}" -f $_ }
        $line = "    " + ($hexValues -join ", ")
        
        if ($i + 16 -lt $Data.Length) {
            $line += ","
        }
        
        $lines += $line
    }
    
    $lines += "};"
    
    return $lines -join "`r`n"
}

# 生成PFC BIN配置文件
function Generate-PfcBinConfig {
    Write-Host "`n1. 生成PFC BIN配置文件..." -ForegroundColor Yellow
    
    $binPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.bin"
    
    if (-not (Test-Path $binPath)) {
        $binPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.bin"
    }
    
    if (-not (Test-Path $binPath)) {
        Write-Host "错误: 找不到PFC bootloader二进制文件" -ForegroundColor Red
        return $false
    }
    
    try {
        $data = [System.IO.File]::ReadAllBytes($binPath)
        $fileName = Split-Path $binPath -Leaf
        $comment = "$fileName ($($data.Length) bytes)"
        $csharpCode = ConvertTo-CSharpArray -Data $data -ArrayName "BOOTLOADER_5800_PFC_BIN_DATA" -Comment $comment
        
        $outputPath = "BOOTLOADER_5800_PFC_BIN_DATA.cs"
        [System.IO.File]::WriteAllText($outputPath, $csharpCode, [System.Text.Encoding]::UTF8)
        
        Write-Host "成功生成: $outputPath" -ForegroundColor Green
        Write-Host "文件大小: $($data.Length) 字节" -ForegroundColor Green
        return $true
    }
    catch {
        Write-Host "错误: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# 生成PFC HEX配置文件
function Generate-PfcHexConfig {
    Write-Host "`n2. 生成PFC HEX配置文件..." -ForegroundColor Yellow
    
    $hexPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800_patched.hex"
    
    if (-not (Test-Path $hexPath)) {
        $hexPath = "../3. bootloader/BootLoader-5800_pfc/Keil/Execute/BootLoader-5800.hex"
    }
    
    if (-not (Test-Path $hexPath)) {
        Write-Host "错误: 找不到PFC bootloader HEX文件" -ForegroundColor Red
        return $false
    }
    
    try {
        $hexContent = [System.IO.File]::ReadAllText($hexPath)
        $hexBytes = [System.Text.Encoding]::UTF8.GetBytes($hexContent)
        
        $fileName = Split-Path $hexPath -Leaf
        $comment = "$fileName ($($hexBytes.Length) bytes)"
        $csharpCode = ConvertTo-CSharpArray -Data $hexBytes -ArrayName "BOOTLOADER_5800_PFC_HEX_DATA" -Comment $comment
        
        $outputPath = "BOOTLOADER_5800_PFC_HEX_DATA.cs"
        [System.IO.File]::WriteAllText($outputPath, $csharpCode, [System.Text.Encoding]::UTF8)
        
        Write-Host "成功生成: $outputPath" -ForegroundColor Green
        Write-Host "文件大小: $($hexBytes.Length) 字节" -ForegroundColor Green
        return $true
    }
    catch {
        Write-Host "错误: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# 主程序
$successCount = 0

if (Generate-PfcBinConfig) {
    $successCount++
}

if (Generate-PfcHexConfig) {
    $successCount++
}

Write-Host "`n" + ("=" * 50)
Write-Host "完成! 成功生成 $successCount/2 个配置文件" -ForegroundColor Cyan

if ($successCount -eq 2) {
    Write-Host "`n✅ PFC配置支持已成功添加到boot_config目录" -ForegroundColor Green
    Write-Host "   - BOOTLOADER_5800_PFC_BIN_DATA.cs" -ForegroundColor Green
    Write-Host "   - BOOTLOADER_5800_PFC_HEX_DATA.cs" -ForegroundColor Green
    Write-Host "`n📝 下一步:" -ForegroundColor Yellow
    Write-Host "   1. 将这些文件集成到IAP工具项目中" -ForegroundColor Yellow
    Write-Host "   2. 更新TargetDetector.cs以支持PFC设备检测" -ForegroundColor Yellow
    Write-Host "   3. 更新MainForm.cs以支持PFC bootloader合并" -ForegroundColor Yellow
} else {
    Write-Host "`n❌ 部分文件生成失败，请检查错误信息" -ForegroundColor Red
}

Write-Host "`n按任意键继续..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
