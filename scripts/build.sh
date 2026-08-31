#!/usr/bin/env bash

set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
preset="${1:-Debug}"

case "${preset}" in
  Debug|Release) ;;
  *)
    echo "Usage: $0 [Debug|Release]" >&2
    exit 2
    ;;
esac

find_bundle_tool() {
  local bundle_name="$1"
  local relative_path="$2"
  local candidate
  local bundle_root="${CUBE_BUNDLE_PATH:-${HOME}/Library/Application Support/stm32cube/bundles}"

  for candidate in "${bundle_root}/${bundle_name}"/*/"${relative_path}"; do
    if [[ -x "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done

  return 1
}

cmake_bin="$(command -v cmake || true)"
ninja_bin="$(command -v ninja || true)"
gcc_bin="$(command -v arm-none-eabi-gcc || true)"

if [[ -z "${cmake_bin}" ]]; then
  cmake_bin="$(find_bundle_tool cmake 'CMake.app/Contents/bin/cmake' || true)"
fi
if [[ -z "${ninja_bin}" ]]; then
  ninja_bin="$(find_bundle_tool ninja 'bin/ninja' || true)"
fi
if [[ -z "${gcc_bin}" ]]; then
  gcc_bin="$(find_bundle_tool gnu-tools-for-stm32 'bin/arm-none-eabi-gcc' || true)"
fi

if [[ -z "${cmake_bin}" || -z "${ninja_bin}" || -z "${gcc_bin}" ]]; then
  echo "STM32 build tools were not found." >&2
  echo "Install the STM32Cube tool bundles, or add cmake, ninja, and arm-none-eabi-gcc to PATH." >&2
  exit 1
fi

export PATH="$(dirname "${cmake_bin}"):$(dirname "${ninja_bin}"):$(dirname "${gcc_bin}"):${PATH}"

cmake --preset "${preset}" -S "${project_dir}"
cmake --build --preset "${preset}"

