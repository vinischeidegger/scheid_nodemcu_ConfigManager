#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=./pio_remote_common.sh
source "${SCRIPT_DIR}/pio_remote_common.sh"

MONITOR_BAUD="${MONITOR_BAUD:-115200}"

print_help() {
    cat <<'EOF'
Monitor NodeMCU serial output remotely using PlatformIO inside Docker.

Usage:
  scripts/pio_remote_monitor.sh

Optional environment variables:
  PIO_COMPOSE_FILE   Docker compose file (default: /srv/platformio/compose.yaml)
  PIO_SERVICE        Compose service name (default: platformio)
  NODEMCU_PORT       Serial monitor port (default: /dev/nodemcu)
  MONITOR_BAUD       Serial baud rate (default: 115200)
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    print_help
    exit 0
fi

print_runtime_config
echo "  monitor port : ${NODEMCU_PORT}"
echo "  monitor baud : ${MONITOR_BAUD}"

echo
echo "Starting serial monitor (press Ctrl+C to stop)..."
compose_run device monitor --port "${NODEMCU_PORT}" --baud "${MONITOR_BAUD}"
