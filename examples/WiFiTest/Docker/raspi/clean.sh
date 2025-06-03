#!/bin/bash

set -e

IMAGE_NAME="rpi-builder"

echo "=== WiFiTest プロジェクトのクリーンアップ ==="

# 使用方法を表示
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    echo "使用方法: $0 [オプション]"
    echo "オプション:"
    echo "  --docker, -d          Dockerイメージも削除"
    echo "  --all, -a             ビルドファイルとDockerイメージをすべて削除"
    echo "  --help, -h            このヘルプを表示"
    echo ""
    echo "デフォルト: ビルドファイルのみ削除"
    exit 0
fi

# オプションの解析
CLEAN_DOCKER=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --docker|-d|--all|-a)
            CLEAN_DOCKER=true
            shift
            ;;
        *)
            echo "不明なオプション: $1"
            echo "ヘルプ: $0 --help"
            exit 1
            ;;
    esac
done

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

PROJECT_PATH="/Users/ring2/Documents/src/R2JUCE"
BUILD_PATH="$PROJECT_PATH/examples/WiFiTest/Builds/raspi"

# ビルドファイルのクリーンアップ
if [[ -d "$BUILD_PATH" ]]; then
    echo "ビルドファイルをクリーンアップ中..."
    
    # Dockerコンテナ内でmake cleanを実行
    if docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
        docker run --rm \
          --platform linux/arm64 \
          -v "$PROJECT_PATH":/mnt/project \
          -w /mnt/project/examples/WiFiTest/Builds/raspi \
          "$IMAGE_NAME" \
          make clean
    else
        echo "Dockerイメージが存在しないため、手動でビルドディレクトリを削除します..."
        rm -rf "$BUILD_PATH/build"
        rm -rf "$BUILD_PATH/pre_build"
        echo "ビルドディレクトリを削除しました"
    fi
else
    echo "ビルドパスが見つかりません: $BUILD_PATH"
fi

# Dockerイメージの削除
if [[ "$CLEAN_DOCKER" == "true" ]]; then
    echo "Dockerイメージを削除中..."
    if docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
        docker rmi "$IMAGE_NAME"
        echo "Dockerイメージ '$IMAGE_NAME' を削除しました"
    else
        echo "Dockerイメージ '$IMAGE_NAME' は存在しません"
    fi
    
    # 未使用のDockerリソースもクリーンアップ
    echo "未使用のDockerリソースをクリーンアップ中..."
    docker system prune -f
fi

echo "=== クリーンアップ完了 ==="
