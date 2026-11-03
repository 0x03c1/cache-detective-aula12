#!/usr/bin/env bash
# Entra no ambiente Linux da atividade (macOS / Linux / Git Bash).
# Uso: bash docker/entrar.sh        (a partir da pasta da atividade)
set -e
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
if ! docker image inspect cache-detective:aula12 >/dev/null 2>&1; then
  echo "Construindo a imagem cache-detective:aula12 (so na primeira vez)..."
  docker build -t cache-detective:aula12 "$RAIZ/docker"
fi
docker run --rm -it -v "$RAIZ/codigo:/atividade" cache-detective:aula12
