#!/usr/bin/env bash
set -Eeuo pipefail

URL="https://download.joedog.org/siege/siege-latest.tar.gz"
PREFIX="${HOME}/.local"
ZSHRC="${HOME}/.zshrc"
# Núcleos para compilar
if command -v nproc >/dev/null 2>&1; then
  JOBS="$(nproc)"
else
  JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
fi

cleanup() {
  [[ -n "${TMPDIR:-}" && -d "$TMPDIR" ]] && rm -rf "$TMPDIR" || true
}
trap cleanup EXIT

say() { printf "\n\033[1;32m%s\033[0m\n" "$*"; }
warn() { printf "\n\033[1;33m%s\033[0m\n" "$*"; }
die() { printf "\n\033[1;31mERROR:\033[0m %s\n" "$*" >&2; exit 1; }

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "Falta la herramienta requerida: $1"
}


need_cmd tar
need_cmd make
need_cmd sh

if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
  die "Necesitas curl o wget para descargar el código fuente."
fi

TMPDIR="$(mktemp -d -t siege-build-XXXXXX)"
say "Directorio temporal: $TMPDIR"

ARCHIVE="$TMPDIR/siege.tar.gz"
say "Descargando siege desde: $URL"
if command -v curl >/dev/null 2>&1; then
  curl -fL --retry 3 --connect-timeout 10 -o "$ARCHIVE" "$URL" || die "No se pudo descargar con curl."
else
  wget -O "$ARCHIVE" "$URL" || die "No se pudo descargar con wget."
fi

if ! head -c 2 "$ARCHIVE" | od -An -tx1 | grep -qi "1f 8b"; then
  warn "El archivo descargado no parece ser un .tar.gz válido (¿HTML?)."
  die "Vuelve a intentar más tarde o verifica el URL."
fi

say "Extrayendo tarball…"
tar xzf "$ARCHIVE" -C "$TMPDIR"

SRC_DIR="$(tar tzf "$ARCHIVE" | head -1 | cut -d/ -f1)"
[[ -d "$TMPDIR/$SRC_DIR" ]] || die "No se encontró el directorio fuente tras extraer."
cd "$TMPDIR/$SRC_DIR"

say "Directorio fuente: $(pwd)"

if [[ ! -x "./configure" ]]; then
  warn "No se encontró ./configure. Intentando generar scripts…"
  if [[ -x "./bootstrap" ]]; then
    ./bootstrap
  elif command -v autoreconf >/dev/null 2>&1; then
    autoreconf -fi
  else
    die "No hay ./configure ni herramientas (bootstrap/autoreconf) para generarlo."
  fi
fi

say "Ejecutando ./configure --prefix=\"$PREFIX\""
./configure --prefix="$PREFIX"

say "Compilando con $JOBS hilos…"
make -j"$JOBS"

say "Instalando en $PREFIX (sin sudo)…"
make install

BIN_DIR="${PREFIX}/bin"
if [[ ":$PATH:" != *":${BIN_DIR}:"* ]]; then
  if ! grep -qs 'export PATH="$HOME/.local/bin:$PATH"' "$ZSHRC"; then
    say "Añadiendo ${BIN_DIR} al PATH en ${ZSHRC}…"
    printf '\n# Añadido por install_siege.sh\nexport PATH="$HOME/.local/bin:$PATH"\n' >> "$ZSHRC"
  else
    warn "Entrada de PATH ya presente en ${ZSHRC}."
  fi

  if [[ -n "${ZSH_VERSION:-}" ]]; then

    source "$ZSHRC" || true
  fi
else
  warn "El binario ya está en tu PATH actual."
fi

say "Verificando instalación:"
if command -v siege >/dev/null 2>&1; then
  siege --version || true
  say "¡Siege instalado correctamente! Ejecuta:  siege -c5 -t10s https://example.com"
else
  warn "No se encontró 'siege' en PATH. Abre una nueva terminal o ejecuta:"
  printf '  source "%s"\n' "$ZSHRC"
  printf 'y luego prueba con:\n  siege --version\n'
fi
