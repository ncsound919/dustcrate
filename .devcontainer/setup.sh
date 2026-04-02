#!/usr/bin/env bash
# .devcontainer/setup.sh
# One-time environment setup run by the dev container on creation.
set -euo pipefail

echo "==> Installing system dependencies for JUCE desktop build..."
sudo apt-get update -y
sudo apt-get install -y --no-install-recommends \
    ninja-build \
    cmake \
    clang \
    clang-format \
    clang-tidy \
    doxygen \
    graphviz \
    libasound2-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxcomposite-dev \
    libfreetype-dev \
    libfontconfig1-dev \
    libwebkit2gtk-4.0-dev

echo "==> Initialising JUCE submodule..."
git submodule update --init --recursive

echo "==> Configuring CMake (Debug)..."
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

echo "==> Dev container ready. Run 'cmake --build build' to build."
