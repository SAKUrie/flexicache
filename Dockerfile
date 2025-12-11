# Docker image containing RISC-V toolchain and QEMU
FROM ubuntu:22.04

# Avoid interactive installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary tools
RUN apt-get update && apt-get install -y \
    gcc-riscv64-unknown-elf \
    qemu-system-misc \
    gdb-multiarch \
    make \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy project files
COPY . .

# Default to bash
CMD ["/bin/bash"]
