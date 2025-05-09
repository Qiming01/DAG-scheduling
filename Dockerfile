# 使用官方Ubuntu基础镜像
FROM ubuntu:24.04

# 设置非交互式环境以避免安装过程中的提示
ENV DEBIAN_FRONTEND=noninteractive

# 安装必要的依赖项
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    build-essential \
    graphviz \
    libyaml-cpp-dev \
    python3-matplotlib \
    libtbb-dev \
    libeigen3-dev \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# 克隆项目并初始化子模块
WORKDIR /app
COPY . .
RUN git submodule update --init --recursive

# 构建项目（添加-p参数避免目录存在时报错）
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

# 设置默认工作目录
WORKDIR /app/build

# 设置容器启动时的默认命令
CMD ["bash"]