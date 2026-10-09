#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
g++ -std=c++17 -Wall -Wextra -Werror -Isrc tests/snapshot_altitude_test.cpp -o "$out"
"$out"

# Weather caching and safe front/back image handover (native implementation)
g++ -std=c++17 -O2 -pthread -I src tests/weather_image_test.cpp src/weather.cpp src/weather_image.cpp -o /tmp/desk_radar_weather_test
/tmp/desk_radar_weather_test
