FROM arm64v8/ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    gcc-10 g++-10 libstdc++-10-dev \
    build-essential \
    cmake \
    git \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    libsdl2-dev \
    pkg-config \
    libfreetype6-dev \
    libcurl4-openssl-dev \
    libwebkit2gtk-4.0-dev \
    libgtk-3-dev \
    libjack-jackd2-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 \
 && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100

WORKDIR /app/WiFiTest/Builds/raspi
