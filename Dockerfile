# 构建包含 RISC-V 工具链和 QEMU 的 Docker 镜像
FROM ubuntu:22.04

# 避免交互式安装
ENV DEBIAN_FRONTEND=noninteractive

# 安装必要的工具
RUN apt-get update && apt-get install -y \
    gcc-riscv64-unknown-elf \
    qemu-system-misc \
    gdb-multiarch \
    make \
    vim \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /workspace

# 复制项目文件
COPY . .

# 默认启动 bash
CMD ["/bin/bash"]

