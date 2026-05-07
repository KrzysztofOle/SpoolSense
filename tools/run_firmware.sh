#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

printf '[SpoolSense] Run firmware: flash + monitor\n'
exec "$script_dir/run_idf.sh" flash monitor
