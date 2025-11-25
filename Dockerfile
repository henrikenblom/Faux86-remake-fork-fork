# Dockerfile for building Faux86-remake for Raspberry Pi 3
# This provides a reproducible build environment with exact versions

FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    gcc-arm-none-eabi \
    g++-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    make \
    git \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Clone Circle SDK at exact commit
RUN git clone https://github.com/rsta2/circle.git && \
    cd circle && \
    git checkout d486a51dad128de8c0eafedbdc53cda3aa557c40

# Configure Circle SDK
RUN cd circle && \
    cp Config.mk.sample Config.mk && \
    echo 'RASPPI=3' >> Config.mk && \
    echo 'AARCH=32' >> Config.mk && \
    echo 'PREFIX=arm-none-eabi-' >> Config.mk && \
    echo 'DEFINE += -DKERNEL_MAX_SIZE=0x2000000' >> Config.mk && \
    echo 'DEFINE += -DNO_SDHOST' >> Config.mk && \
    echo 'DEFINE += -DNO_BUSY_WAIT' >> Config.mk && \
    echo 'DEFINE += -DNO_REALTIME' >> Config.mk && \
    echo 'DEFINE += -DNO_USB_SOF_INTR' >> Config.mk

# Build Circle SDK and addons
RUN cd circle && \
    ./makeall --nosample && \
    cd addon/SDCard && make && cd ../.. && \
    cd addon/vc4/sound && make && cd ../../.. && \
    cd addon/vc4/vchiq && make && cd ../../.. && \
    cd addon/linux && make && cd ../.. && \
    cd addon/fatfs && make && cd ../.. && \
    cd addon/Properties && make && cd ../..

# Copy Faux86 source code (mounted at runtime)
VOLUME /workspace

# Build script
COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
