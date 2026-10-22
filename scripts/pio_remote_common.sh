#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_NAME="$(basename "${REPO_ROOT}")"

PIO_COMPOSE_FILE="${PIO_COMPOSE_FILE:-/srv/platformio/compose.yaml}"
PIO_SERVICE="${PIO_SERVICE:-platformio}"
PIO_CONTAINER_ROOT="${PIO_CONTAINER_ROOT:-/workspace}"
PIO_EXAMPLE_PATH="${PIO_EXAMPLE_PATH:-examples/BasicSetup}"
PIO_ENV="${PIO_ENV:-nodemcuv2}"
NODEMCU_PORT="${NODEMCU_PORT:-/dev/nodemcu}"

CONTAINER_PROJECT_DIR="${PIO_CONTAINER_ROOT}/${REPO_NAME}"
CONTAINER_EXAMPLE_DIR="${CONTAINER_PROJECT_DIR}/${PIO_EXAMPLE_PATH}"

compose_run() {
    docker compose -f "${PIO_COMPOSE_FILE}" run --rm --no-deps "${PIO_SERVICE}" "$@"
}

print_runtime_config() {
    echo "Remote PlatformIO configuration:"
    echo "  compose file : ${PIO_COMPOSE_FILE}"
    echo "  service      : ${PIO_SERVICE}"
    echo "  repo root    : ${REPO_ROOT}"
    echo "  container dir: ${CONTAINER_PROJECT_DIR}"
    echo "  example dir  : ${CONTAINER_EXAMPLE_DIR}"
    echo "  env          : ${PIO_ENV}"
}
