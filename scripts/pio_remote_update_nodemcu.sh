#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=./pio_remote_common.sh
source "${SCRIPT_DIR}/pio_remote_common.sh"

UPLOAD_FS="true"

print_help() {
    cat <<'EOF'
Update NodeMCU remotely using PlatformIO inside Docker.

This script uploads firmware and LittleFS data by default.

Usage:
  scripts/pio_remote_update_nodemcu.sh [--firmware-only]

Flags:
  --firmware-only  Upload only firmware (skip filesystem upload).
  --with-fs        Explicitly upload filesystem after firmware (default behavior).
  -h, --help  Show this help message.

Optional environment variables:
  PIO_COMPOSE_FILE   Docker compose file (default: /srv/platformio/compose.yaml)
  PIO_SERVICE        Compose service name (default: platformio)
  PIO_CONTAINER_ROOT Container workspace root (default: /workspace)
  PIO_EXAMPLE_PATH   Example path from repo root (default: examples/BasicSetup)
  PIO_ENV            PlatformIO environment (default: nodemcuv2)
  NODEMCU_PORT       Serial upload port (default: /dev/nodemcu)
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --with-fs)
            UPLOAD_FS="true"
            shift
            ;;
        --firmware-only)
          UPLOAD_FS="false"
          shift
          ;;
        -h|--help)
            print_help
            exit 0
            ;;
        *)
            echo "Unknown argument: $1"
            print_help
            exit 1
            ;;
    esac
done

print_runtime_config
echo "  upload port  : ${NODEMCU_PORT}"

echo
echo "Uploading firmware to NodeMCU..."
compose_run run -d "${CONTAINER_EXAMPLE_DIR}" -e "${PIO_ENV}" -t upload --upload-port "${NODEMCU_PORT}"

echo "Firmware upload finished successfully."

if [[ "${UPLOAD_FS}" == "true" ]]; then
    echo
    echo "Uploading filesystem image to NodeMCU..."
    compose_run run -d "${CONTAINER_EXAMPLE_DIR}" -e "${PIO_ENV}" -t uploadfs --upload-port "${NODEMCU_PORT}"
    echo "Filesystem upload finished successfully."
else
  echo
  echo "Skipping filesystem upload (--firmware-only)."
fi

echo "NodeMCU update complete."
