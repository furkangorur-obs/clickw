#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$(readlink -f -- "$0")")"
export QT_QPA_PLATFORM=wayland
export LANG=tr_TR.UTF-8
export LC_ALL=tr_TR.UTF-8
exec ./build/clickw "$@"
