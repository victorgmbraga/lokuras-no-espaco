# setup-mingw.ps1 - instala o MinGW (winlibs i686) usado pelo build.bat
#
# Chamado automaticamente pelo build.bat quando o g++ nao e encontrado.
# Faz download do winlibs com retomada, verifica o SHA-256 e extrai em
# tools\mingw32 - tudo dentro da pasta do projeto (nada e gravado fora dela).
#
# Saida: exit 0 = pronto para uso | exit 1 = cancelado ou falha.

$ErrorActionPreference = 'Stop'

$Url      = 'https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-14.0.0-msvcrt-r2/winlibs-i686-posix-dwarf-gcc-16.2.0-mingw-w64msvcrt-14.0.0-r2.zip'
$Sha256   = 'f1fab67fbe6ed1b0a390d06db6af8b4f0ca25fceab4f51bac2a88af8b81a93ec'
$ToolsDir = $PSScriptRoot
$MingwDir = Join-Path $ToolsDir 'mingw32'
$DestBin  = Join-Path $MingwDir 'bin'
$ZipPath  = Join-Path $ToolsDir 'winlibs-i686.zip'

trap {
    Write-Host ""
    Write-Host ("Falha: " + $_.Exception.Message) -ForegroundColor Red
    exit 1
}

function Test-Gxx {
    return (Test-Path (Join-Path $DestBin 'g++.exe'))
}

function Get-FileSha256([string]$Path) {
    if (Get-Command Get-FileHash -ErrorAction SilentlyContinue) {
        return (Get-FileHash -Algorithm SHA256 -Path $Path).Hash.ToLower()
    }
    $line = certutil -hashfile $Path SHA256 | Select-Object -Index 1
    return $line.Trim().Replace(' ', '').Replace("`t", '').ToLower()
}

function Invoke-Download([string]$From, [string]$To) {
    if (Get-Command curl.exe -ErrorAction SilentlyContinue) {
        $opts = @('-L', '--fail', '--retry', '3', '-o', $To, $From)
        if (Test-Path $To) { $opts = @('-L', '--fail', '--retry', '3', '-C', '-', '-o', $To, $From) }
        & curl.exe @opts
        if ($LASTEXITCODE -ne 0) { throw "curl falhou com codigo $LASTEXITCODE" }
        return
    }
    try {
        Start-BitsTransfer -Source $From -Destination $To -ErrorAction Stop
    } catch {
        Invoke-WebRequest -Uri $From -OutFile $To -UseBasicParsing
    }
}

function Expand-Zip([string]$Zip, [string]$Dest) {
    if (Get-Command tar.exe -ErrorAction SilentlyContinue) {
        & tar.exe -xf $Zip -C $Dest
        if ($LASTEXITCODE -ne 0) { throw "tar falhou com codigo $LASTEXITCODE" }
        return
    }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::ExtractToDirectory($Zip, $Dest)
}

if (Test-Gxx) {
    Write-Host "MinGW ja instalado em $DestBin"
    exit 0
}

Write-Host ""
Write-Host "O compilador g++ nao foi encontrado no sistema."
Write-Host ""
Write-Host "Este script pode baixar o toolchain MinGW-w64 automaticamente:"
Write-Host "  - winlibs i686 (32 bits), GCC 16.2.0, msvcrt, posix, dwarf"
Write-Host "  - download: ~270 MB (zip) / ~950 MB depois de extraido"
Write-Host "  - origem:   GitHub - brechtsanders/winlibs_mingw"
Write-Host "  - destino:  $MingwDir"
Write-Host ""
$answer = Read-Host "Baixar e instalar agora? [S/N]"
if ([string]$answer -notmatch '^[SsYy]') {
    Write-Host "Cancelado pelo usuario."
    exit 1
}

Write-Host ""
Write-Host "Baixando (tentativas anteriores sao retomadas)..."
Invoke-Download $Url $ZipPath

Write-Host "Verificando SHA-256..."
$hash = Get-FileSha256 $ZipPath
if ($hash -ne $Sha256) {
    Write-Host ("SHA-256 nao confere. Esperado: $Sha256 | Obtido: $hash") -ForegroundColor Red
    Remove-Item $ZipPath -Force -ErrorAction SilentlyContinue
    exit 1
}

if (Test-Path $MingwDir) {
    Write-Host "Removendo instalacao anterior incompleta..."
    Remove-Item $MingwDir -Recurse -Force
}

Write-Host "Extraindo (pode demorar alguns minutos)..."
Expand-Zip $ZipPath $ToolsDir
Remove-Item $ZipPath -Force -ErrorAction SilentlyContinue

if (-not (Test-Gxx)) {
    Write-Host "g++.exe nao encontrado apos a extracao." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "MinGW instalado em $DestBin"
exit 0
