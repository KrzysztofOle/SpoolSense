#!/usr/bin/env bash

set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
idf_path="${IDF_PATH:-/Users/krzysztof/esp/esp-idf}"
tools_path="${IDF_TOOLS_PATH:-/Users/krzysztof/.espressif}"
python_bin="${PYTHON:-/usr/local/bin/python3}"

cd "$project_root"

echo "[SpoolSense] project: $project_root"
echo "[SpoolSense] command: idf.py $*"
echo "[SpoolSense] exporting ESP-IDF environment..."

export_cmd="$(
  PATH=/usr/bin:/bin:/usr/sbin:/sbin \
    IDF_PATH="$idf_path" \
    IDF_TOOLS_PATH="$tools_path" \
    "$python_bin" "$idf_path/tools/idf_tools.py" export --format shell --prefer-system
)"

eval "$export_cmd"
echo "[SpoolSense] environment ready"
exec idf.py "$@"
