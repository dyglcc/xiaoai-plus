#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${PROJECT_ROOT}/dist/xiaoai-plus-armhf"
BIN_DIR="${DIST_DIR}/bin"
ASSETS_DIR="${DIST_DIR}/assets"

SPEAKER_BIN="${PROJECT_ROOT}/build-armv7/xiaoai_plus_speaker"

if [ ! -f "${SPEAKER_BIN}" ]; then
  echo "Error: ${SPEAKER_BIN} not found. Run 'make compile' first." >&2
  exit 1
fi

rm -rf "${DIST_DIR}"
mkdir -p "${BIN_DIR}" "${ASSETS_DIR}"

# Binary (stripped)
cp "${SPEAKER_BIN}" "${BIN_DIR}/"
docker run --rm \
  -v "${PROJECT_ROOT}:/workspace" \
  "xiaoai-plus-toolchain:dev" \
  arm-linux-gnueabihf-strip "/workspace/dist/xiaoai-plus-armhf/bin/xiaoai_plus_speaker"

# Model files
cp "${PROJECT_ROOT}/assets/encoder.onnx" "${ASSETS_DIR}/"
cp "${PROJECT_ROOT}/assets/decoder.onnx" "${ASSETS_DIR}/"
cp "${PROJECT_ROOT}/assets/joiner.onnx"  "${ASSETS_DIR}/"
cp "${PROJECT_ROOT}/assets/tokens.txt"   "${ASSETS_DIR}/"
cp "${PROJECT_ROOT}/assets/keywords.txt" "${ASSETS_DIR}/"

# Config
cp "${PROJECT_ROOT}/config.ini.example" "${DIST_DIR}/"
if [ -f "${PROJECT_ROOT}/config.ini" ]; then
  cp "${PROJECT_ROOT}/config.ini" "${DIST_DIR}/"
else
  cp "${PROJECT_ROOT}/config.ini.example" "${DIST_DIR}/config.ini"
  echo "Warning: ${PROJECT_ROOT}/config.ini not found; using config.ini.example as placeholder." >&2
fi

# Launcher
cat > "${DIST_DIR}/run.sh" <<'EOF'
#!/bin/ash
set -eu
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "${DIR}"
exec "${DIR}/bin/xiaoai_plus_speaker" -c "${DIR}/config.ini" "$@"
EOF
chmod +x "${DIST_DIR}/run.sh"

# Manifest
{
  echo "built_at=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "binary_size=$(stat -c%s "${BIN_DIR}/xiaoai_plus_speaker")"
  sha256sum "${BIN_DIR}/xiaoai_plus_speaker" "${ASSETS_DIR}"/*
} > "${DIST_DIR}/manifest.txt"

echo "==> Distribution packaged to ${DIST_DIR}"
ls -lhR "${DIST_DIR}"
