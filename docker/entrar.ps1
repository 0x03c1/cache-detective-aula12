# Entra no ambiente Linux da atividade (Windows PowerShell + Docker Desktop).
# Uso: powershell -ExecutionPolicy Bypass -File docker\entrar.ps1
$raiz = Split-Path -Parent $PSScriptRoot
docker image inspect cache-detective:aula12 *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Construindo a imagem cache-detective:aula12 (so na primeira vez)..."
    docker build -t cache-detective:aula12 "$raiz\docker"
}
docker run --rm -it -v "${raiz}\codigo:/atividade" cache-detective:aula12
