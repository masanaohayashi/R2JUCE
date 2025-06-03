#!/bin/bash
set -e

IMAGE_NAME="linux-juce-builder-22"

# Dockerデーモンが起動しているかチェック
if ! docker info >/dev/null 2>&1; then
    echo "Docker起動中..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        open -a Docker
        while ! docker info >/dev/null 2>&1; do
            sleep 2
        done
    else
        echo "Dockerデーモンを起動してください: sudo systemctl start docker"
        exit 1
    fi
fi

# Dockerイメージが存在するかチェック、なければビルド
if ! docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
    echo "イメージをビルド中..."
    docker build -t "$IMAGE_NAME" .
fi

# ビルド実行
docker run --rm -it \
  -v ~/Documents/src/R2fx:/app/R2fx \
  -w /app/R2fx/IFW3/Builds/LinuxMakefile \
  -e CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0" \
  "$IMAGE_NAME" make CONFIG=Release -j12
