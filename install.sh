#!/bin/sh
set -eu

# 一行安装：
# curl -sSfL https://raw.githubusercontent.com/kslr/xiaoai-plus/main/install.sh | sh

APP_DIR="${APP_DIR:-/data/xiaoai-plus}"
ARCHIVE_URL="${ARCHIVE_URL:-https://github.com/kslr/xiaoai-plus/releases/latest/download/oh2p.tar.gz}"
MIN_SPACE_MB="${MIN_SPACE_MB:-100}"
CHECK_DIR="${APP_DIR}"
while [ ! -d "${CHECK_DIR}" ]; do
  if [ "${CHECK_DIR}" = "/" ]; then
    break
  fi
  CHECK_DIR="$(dirname "${CHECK_DIR}")"
done

avail_kb="$(df -k "${CHECK_DIR}" 2>/dev/null | awk 'NR==2 {print $4}')"
need_kb=$((MIN_SPACE_MB * 1024))
if [ -z "${avail_kb}" ]; then
  echo "错误：无法检查磁盘空间（${CHECK_DIR}）" >&2
  exit 1
fi
if [ "${avail_kb}" -lt "${need_kb}" ]; then
  echo "错误：磁盘空间不足，至少需要 ${MIN_SPACE_MB}MB 可用空间（${CHECK_DIR}）" >&2
  exit 1
fi

if ! command -v curl >/dev/null 2>&1; then
  echo "错误：缺少命令 curl" >&2
  exit 1
fi
if ! command -v tar >/dev/null 2>&1; then
  echo "错误：缺少命令 tar" >&2
  exit 1
fi

TMP_DIR="${APP_DIR}/.install-tmp.$$"
ARCHIVE_PATH="${TMP_DIR}/oh2p.tar.gz"
SRC_DIR="${TMP_DIR}/oh2p"
trap 'rm -rf "${TMP_DIR}"' EXIT INT TERM

mkdir -p "${TMP_DIR}" "${APP_DIR}/assets"

echo "正在下载安装包..."
curl -sSfL "${ARCHIVE_URL}" -o "${ARCHIVE_PATH}"

echo "正在解压安装包..."
tar -xzf "${ARCHIVE_PATH}" -C "${TMP_DIR}"

if [ ! -d "${SRC_DIR}" ]; then
  echo "错误：安装包格式不正确，缺少 oh2p 目录" >&2
  exit 1
fi

echo "正在安装文件到 ${APP_DIR} ..."
cp "${SRC_DIR}/xiaoai_plus_speaker" "${APP_DIR}/xiaoai_plus_speaker"
chmod 0755 "${APP_DIR}/xiaoai_plus_speaker"

cp "${SRC_DIR}/config.ini.example" "${APP_DIR}/config.ini.example"
chmod 0644 "${APP_DIR}/config.ini.example"

cp "${SRC_DIR}/assets/decoder.onnx" "${APP_DIR}/assets/decoder.onnx"
cp "${SRC_DIR}/assets/encoder.onnx" "${APP_DIR}/assets/encoder.onnx"
cp "${SRC_DIR}/assets/joiner.onnx" "${APP_DIR}/assets/joiner.onnx"
cp "${SRC_DIR}/assets/keywords.txt" "${APP_DIR}/assets/keywords.txt"
cp "${SRC_DIR}/assets/tokens.txt" "${APP_DIR}/assets/tokens.txt"
chmod 0644 \
  "${APP_DIR}/assets/decoder.onnx" \
  "${APP_DIR}/assets/encoder.onnx" \
  "${APP_DIR}/assets/joiner.onnx" \
  "${APP_DIR}/assets/keywords.txt" \
  "${APP_DIR}/assets/tokens.txt"

if [ ! -f "${APP_DIR}/config.ini" ]; then
  cp "${APP_DIR}/config.ini.example" "${APP_DIR}/config.ini"
  chmod 0644 "${APP_DIR}/config.ini"
  echo "已创建配置文件：${APP_DIR}/config.ini"
fi

echo "安装完成：${APP_DIR}"
