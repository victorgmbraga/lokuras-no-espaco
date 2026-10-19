# setup-emsdk.ps1 - instala o Emscripten (emsdk) usado pelo build-web.bat
#
# Chamado automaticamente pelo build-web.bat quando o em++ nao e encontrado.
# Baixa o emsdk (Git + ~1,3 GB de download do compilador WebAssembly) e o
# instala em tools\emsdk - tudo dentro da pasta do projeto. Nada e gravado
# fora dela e nao e preciso root/administrador.
#
# Saida: exit 0 = pronto para uso | exit 1 = cancelado ou falha.

$ErrorActionPreference = 'Stop'

$EmsdkVersion = '6.0.12'
$ToolsDir     = $PSScriptRoot
$EmsdkDir     = Join-Path $ToolsDir 'emsdk'
$EmxxExe      = Join-Path $EmsdkDir 'upstream\emscripten\em++.exe'
$EmxxBat      = Join-Path $EmsdkDir 'upstream\emscripten\em++.bat'
$RepoUrl      = 'https://github.com/emscripten-core/emsdk.git'

trap {
    Write-Host ""
    Write-Host ("Falha: " + $_.Exception.Message) -ForegroundColor Red
    exit 1
}

if ((Test-Path $EmxxExe) -or (Test-Path $EmxxBat)) {
    Write-Host "Emscripten ja instalado em $EmsdkDir"
    exit 0
}

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "O Git nao foi encontrado no PATH." -ForegroundColor Red
    Write-Host "Instale o Git (https://git-scm.com/downloads) e rode de novo, ou"
    Write-Host "baixe o emsdk manualmente em https://emscripten.org/docs/getting_started/downloads.html"
    Write-Host "e rode `".\emsdk install $EmsdkVersion`" + `".\emsdk activate $EmsdkVersion`" na pasta dele."
    exit 1
}

Write-Host ""
Write-Host "O Emscripten (em++) nao foi encontrado no sistema."
Write-Host ""
Write-Host "Este script pode baixar o Emscripten automaticamente:"
Write-Host "  - emsdk $EmsdkVersion (compilador C/C++ -> WebAssembly)"
Write-Host "  - download: ~1,3 GB (~2,5 GB depois de instalado)"
Write-Host "  - origem:   GitHub - emscripten-core/emsdk"
Write-Host "  - destino:  $EmsdkDir"
Write-Host ""
$answer = Read-Host "Baixar e instalar agora? [S/N]"
if ([string]$answer -notmatch '^[SsYy]') {
    Write-Host "Cancelado pelo usuario."
    exit 1
}

if (Test-Path $EmsdkDir) {
    Write-Host "Removendo instalacao anterior incompleta..."
    Remove-Item $EmsdkDir -Recurse -Force
}

Write-Host ""
Write-Host "Baixando o emsdk (Git)..."
& git clone --depth 1 $RepoUrl $EmsdkDir
if ($LASTEXITCODE -ne 0) { throw "git clone falhou com codigo $LASTEXITCODE" }

Write-Host ""
Write-Host "Instalando o SDK $EmsdkVersion (download do compilador, pode demorar)..."
& cmd.exe /c "cd /d `"$EmsdkDir`" && emsdk.bat install $EmsdkVersion"
if ($LASTEXITCODE -ne 0) { throw "emsdk install falhou com codigo $LASTEXITCODE" }

Write-Host ""
Write-Host "Ativando o SDK $EmsdkVersion..."
& cmd.exe /c "cd /d `"$EmsdkDir`" && emsdk.bat activate $EmsdkVersion"
if ($LASTEXITCODE -ne 0) { throw "emsdk activate falhou com codigo $LASTEXITCODE" }

if (-not ((Test-Path $EmxxExe) -or (Test-Path $EmxxBat))) {
    Write-Host "em++ nao encontrado apos a instalacao." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Emscripten instalado em $EmsdkDir"
exit 0
