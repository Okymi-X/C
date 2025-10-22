# Script PowerShell pour compiler les deux versions

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  COMPILATION MESSAGERIE - MULTI-OS" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Compilation version Windows
Write-Host "[1/2] Compilation version Windows..." -ForegroundColor Green
gcc -o messagerie_windows.exe main_multiplatform.c fonctions.c -lm -lws2_32 -lmswsock

if ($LASTEXITCODE -eq 0) {
    Write-Host "[OK] Version Windows compilée: messagerie_windows.exe" -ForegroundColor Green
} else {
    Write-Host "[ERREUR] Échec compilation Windows" -ForegroundColor Red
    exit 1
}

Write-Host ""

# Vérifier si WSL est disponible
Write-Host "[2/2] Tentative de compilation Linux (via WSL)..." -ForegroundColor Green

if (Get-Command wsl -ErrorAction SilentlyContinue) {
    Write-Host "WSL détecté, compilation Linux..." -ForegroundColor Cyan
    wsl gcc -o messagerie_linux main_multiplatform.c fonctions.c -lm -lpthread -D__linux__
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[OK] Version Linux compilée: messagerie_linux" -ForegroundColor Green
    } else {
        Write-Host "[AVERTISSEMENT] Impossible de compiler pour Linux via WSL" -ForegroundColor Yellow
    }
} else {
    Write-Host "[INFO] WSL non disponible, version Linux non compilée" -ForegroundColor Yellow
    Write-Host "[INFO] Pour compiler sous Linux, utilisez: compile_linux.sh" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  COMPILATION TERMINÉE" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Exécutables disponibles:" -ForegroundColor White
Write-Host "  - Windows: .\messagerie_windows.exe" -ForegroundColor Cyan
if (Test-Path "messagerie_linux") {
    Write-Host "  - Linux:   ./messagerie_linux (via WSL)" -ForegroundColor Cyan
}
Write-Host ""
