#!/bin/bash
set -e

IMAGE_NAME="linux-juce-builder-22"

echo "=== 完全クリーンビルド開始 ==="

# Dockerイメージを削除
if docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
    echo "既存Dockerイメージを削除中..."
    docker rmi "$IMAGE_NAME"
fi

# ビルドファイルを削除
echo "ビルドファイルを削除中..."
docker run --rm \
  -v ~/Documents/src/R2fx:/app/R2fx \
  -w /app/R2fx/IFW3/Builds/LinuxMakefile \
  ubuntu:22.04 bash -c "rm -rf build/ && make clean" 2>/dev/null || true

# Dockerイメージを再ビルド
echo "Dockerイメージを再ビルド中..."
docker build -t "$IMAGE_NAME" .

echo "=== クリーン完了 ==="
echo "通常ビルドは ./build.sh を実行してください"


