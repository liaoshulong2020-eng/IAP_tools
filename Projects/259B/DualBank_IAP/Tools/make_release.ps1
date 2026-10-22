param([string]$Root = (Split-Path $PSScriptRoot -Parent))
$ErrorActionPreference = 'Stop'

function Crc32-Tae([byte[]]$Data) {
    [uint32]$crc = 0
    foreach ($v in $Data) {
        $crc = $crc -bxor ([uint32]$v -shl 24)
        for ($b = 0; $b -lt 8; $b++) {
            if (($crc -band 0x80000000) -ne 0) { $crc = [uint32](($crc -shl 1) -bxor 0x04C11DB7) }
            else { $crc = [uint32]($crc -shl 1) }
        }
    }
    return $crc
}

function Make-Target([string]$Name,[string]$BootPath,[string]$AppPath,[string]$Out) {
    [byte[]]$bootBytes = [IO.File]::ReadAllBytes($BootPath)
    [byte[]]$appBytes = [IO.File]::ReadAllBytes($AppPath)
    if ($bootBytes.Length -gt 0x7000) { throw "$Name Bootloader exceeds 28KB code area" }
    if ($appBytes.Length -gt 0x16000) { throw "$Name APP exceeds 88KB area" }
    $used = 0x8000 + $appBytes.Length
    $imageSize = [int](($used + 127) -band (-bnot 127))
    [byte[]]$image = New-Object byte[] $imageSize
    [Array]::Fill[byte]($image, 0xFF)
    [Array]::Copy($bootBytes, 0, $image, 0, $bootBytes.Length)
    [Array]::Copy($appBytes, 0, $image, 0x8000, $appBytes.Length)
    [uint32]$appCrc = Crc32-Tae $appBytes
    [Array]::Copy([BitConverter]::GetBytes([uint32]$appBytes.Length), 0, $image, 0x7000, 4)
    [Array]::Copy([BitConverter]::GetBytes($appCrc), 0, $image, 0x7004, 4)
    $dir = Join-Path $Out $Name
    New-Item -ItemType Directory -Force $dir | Out-Null
    [IO.File]::WriteAllBytes((Join-Path $dir "$Name.dbiap"), $image)
    [byte[]]$factory = New-Object byte[] 0x40000
    [Array]::Fill[byte]($factory, 0xFF)
    [Array]::Copy($image, 0, $factory, 0, $image.Length)
    [Array]::Copy($image, 0, $factory, 0x20000, $image.Length)
    [IO.File]::WriteAllBytes((Join-Path $dir "$Name.factory.256k.bin"), $factory)
    Copy-Item $BootPath (Join-Path $dir "$Name.bootloader.bin") -Force
    Copy-Item $AppPath (Join-Path $dir "$Name.app.bin") -Force
    $bankCrc = Crc32-Tae $image
    @("target=$Name","boot_size=$($bootBytes.Length)","app_size=$($appBytes.Length)",('app_crc=0x{0:X8}' -f $appCrc),"bank_image_size=$imageSize",('bank_crc=0x{0:X8}' -f $bankCrc)) |
        Set-Content (Join-Path $dir 'checksums.txt') -Encoding utf8
}

$out = Join-Path $Root '成品_可烧录'
New-Item -ItemType Directory -Force $out | Out-Null
Make-Target 'LLC' (Join-Path $Root 'Firmware\BootLoader_LLC_D\Keil\Execute\BootLoader-259B-LLC-D.bin') (Join-Path $Root 'Firmware\APP_LLC_D\ProjectTemplate\Project\MDK\FLASH\Execute\259B_LLC_V_1_2_0_20260820_iap_release.bin') $out
Make-Target 'PFC' (Join-Path $Root 'Firmware\BootLoader_PFC_D\Keil\Execute\BootLoader-259B-PFC-D.bin') (Join-Path $Root 'Firmware\APP_PFC_D\ProjectTemplate\Project\MDK\FLASH\Objects\259B_pfc_A04_v1_2_0_20260424_beta.bin') $out
$hostSource = Join-Path $Root 'HostTool\应用程序'
$hostDestination = Join-Path $out '上位机'
if (Test-Path -LiteralPath $hostDestination) {
    $resolvedHost = (Resolve-Path -LiteralPath $hostDestination).Path
    $resolvedOut = (Resolve-Path -LiteralPath $out).Path
    if (-not $resolvedHost.StartsWith($resolvedOut + [IO.Path]::DirectorySeparatorChar)) { throw 'Invalid host output path' }
    Remove-Item -LiteralPath $resolvedHost -Recurse -Force
}
New-Item -ItemType Directory -Force $hostDestination | Out-Null
Copy-Item (Join-Path $hostSource '*') $hostDestination -Recurse -Force
Get-ChildItem $out -Recurse -File | Where-Object {$_.Name-ne'SHA256SUMS.txt'} | Get-FileHash -Algorithm SHA256 |
    ForEach-Object {'{0}  {1}'-f $_.Hash,$_.Path.Substring((Resolve-Path $out).Path.Length+1)} |
    Set-Content (Join-Path $out 'SHA256SUMS.txt') -Encoding utf8
