#!/bin/bash

set -e

IMAGE_NAME="rpi-builder"

# 使用方法を表示
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    echo "使用方法: $0 [オプション]"
    echo "オプション:"
    echo "  --force-rebuild, -f    Dockerイメージを強制的に再ビルド"
    echo "  --no-cache, -n        キャッシュを使わずにビルド"
    echo "  --help, -h            このヘルプを表示"
    exit 0
fi

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

# オプションの解析
FORCE_REBUILD=false
NO_CACHE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --force-rebuild|-f)
            FORCE_REBUILD=true
            shift
            ;;
        --no-cache|-n)
            NO_CACHE="--no-cache"
            FORCE_REBUILD=true  # --no-cache が指定されたら自動的に再ビルド
            shift
            ;;
        *)
            echo "不明なオプション: $1"
            exit 1
            ;;
    esac
done

# Dockerイメージのビルド判定
BUILD_IMAGE=false

if [[ "$FORCE_REBUILD" == "true" ]]; then
    echo "強制的にDockerイメージを再ビルドします..."
    BUILD_IMAGE=true
elif ! docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
    echo "Dockerイメージが存在しないため、ビルドします..."
    BUILD_IMAGE=true
else
    echo "既存のDockerイメージを使用します（強制再ビルドは --force-rebuild）"
fi

# Dockerイメージのビルド実行
if [[ "$BUILD_IMAGE" == "true" ]]; then
    if [[ $(uname -m) == "arm64" ]]; then
        docker build $NO_CACHE --platform linux/arm64 -t "$IMAGE_NAME" .
    else
        docker build $NO_CACHE -t "$IMAGE_NAME" .
    fi
fi

PROJECT_PATH="/Users/ring2/Documents/src/R2JUCE"
JUCE_PATH="/Users/ring2/JUCE"

echo "ビルド開始..."

# JUCEを /root/JUCE にマウントして $(HOME)/JUCE/modules が解決されるようにする
docker run --rm -it \
  --platform linux/arm64 \
  -v "$PROJECT_PATH":/mnt/project \
  -v "$JUCE_PATH":/root/JUCE \
  -w /mnt/project/examples/WiFiTest/Builds/raspi \
  "$IMAGE_NAME" \
  make CONFIG=Debug -j12
