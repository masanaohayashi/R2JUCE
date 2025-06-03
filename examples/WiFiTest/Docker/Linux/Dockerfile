
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libfreetype6-dev \
    libcurl4-openssl-dev \
    libwebkit2gtk-4.0-dev \
    libgtk-3-dev \
    libjack-jackd2-dev \
    libasound2-dev \
    libx11-dev \
    libxext-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxcomposite-dev \
    libxcursor-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# glibc互換性のためのフラグを設定
ENV CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"

WORKDIR /workspace
