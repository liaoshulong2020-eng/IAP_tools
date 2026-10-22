$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$inputDir = Join-Path $root '开发板_Bank通讯验证\BuildInputs'
$out = Join-Path $root '开发板_Bank通讯验证\成品'
$bootPath = Join-Path $root 'Firmware\BootLoader_LLC_D\Keil\Execute\BootLoader-259B-LLC-D.bin'

function Crc32-Tae([byte[]]$Data) {
    [uint32]$crc = 0
    foreach ($v in $Data) {
        $crc = $crc -bxor ([uint32]$v -shl 24)
        for ($b=0; $b-lt 8; $b++) {
            if ($crc -band 0x80000000) { $crc=[uint32](($crc-shl 1)-bxor 0x04C11DB7) }
            else { $crc=[uint32]($crc-shl 1) }
        }
    }
    $crc
}

function Make-BankImage([string]$AppPath) {
    [byte[]]$boot=[IO.File]::ReadAllBytes($bootPath)
    [byte[]]$app=[IO.File]::ReadAllBytes($AppPath)
    $size=[int](((0x8000+$app.Length)+127)-band(-bnot 127))
    [byte[]]$image=New-Object byte[] $size
    [Array]::Fill[byte]($image,0xFF)
    [Array]::Copy($boot,0,$image,0,$boot.Length)
    [Array]::Copy($app,0,$image,0x8000,$app.Length)
    [Array]::Copy([BitConverter]::GetBytes([uint32]$app.Length),0,$image,0x7000,4)
    [Array]::Copy([BitConverter]::GetBytes([uint32](Crc32-Tae $app)),0,$image,0x7004,4)
    return ,$image
}

New-Item -ItemType Directory -Force $out | Out-Null
[byte[]]$bankA=Make-BankImage (Join-Path $inputDir 'BankA_12V.app.bin')
[byte[]]$bankB=Make-BankImage (Join-Path $inputDir 'BankB_48V.app.bin')
[IO.File]::WriteAllBytes((Join-Path $out '03_BankA_12V_回切.dbiap'),$bankA)
[IO.File]::WriteAllBytes((Join-Path $out '02_BankB_48V_在线升级.dbiap'),$bankB)
[byte[]]$factory=New-Object byte[] 0x40000
[Array]::Fill[byte]($factory,0xFF)
[Array]::Copy($bankA,0,$factory,0,$bankA.Length)
[Array]::Copy($bankA,0,$factory,0x20000,$bankA.Length)
[IO.File]::WriteAllBytes((Join-Path $out '01_BankA_12V_首次烧录_256K.bin'),$factory)
@(
  ('BankA_image_crc=0x{0:X8}' -f (Crc32-Tae $bankA)),
  ('BankB_image_crc=0x{0:X8}' -f (Crc32-Tae $bankB)),
  "BankA_size=$($bankA.Length)",
  "BankB_size=$($bankB.Length)"
) | Set-Content (Join-Path $out 'CRC校验.txt') -Encoding utf8

$hostSource=Join-Path $root 'HostTool\应用程序'
$hostOut=Join-Path $out '上位机'
if(Test-Path -LiteralPath $hostOut){
    $resolvedHost=(Resolve-Path -LiteralPath $hostOut).Path
    $resolvedOut=(Resolve-Path -LiteralPath $out).Path
    if(-not $resolvedHost.StartsWith($resolvedOut+[IO.Path]::DirectorySeparatorChar)){throw 'Invalid diagnostic host path'}
    Remove-Item -LiteralPath $resolvedHost -Recurse -Force
}
New-Item -ItemType Directory -Force $hostOut | Out-Null
Copy-Item (Join-Path $hostSource '*') $hostOut -Recurse -Force
Get-ChildItem $out -Recurse -File | Where-Object {$_.Name-ne'SHA256SUMS.txt'} | Get-FileHash -Algorithm SHA256 |
  ForEach-Object {'{0}  {1}'-f $_.Hash,$_.Path.Substring((Resolve-Path $out).Path.Length+1)} |
  Set-Content (Join-Path $out 'SHA256SUMS.txt') -Encoding utf8
