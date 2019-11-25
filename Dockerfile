FROM ubuntu:16.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential bc bison \
    pkg-config curl flex git jq locales \
    python3 python3.5-dev python3-pip \
    gdb openssl \
    libcap-dev libncurses5-dev libncursesw5-dev libssl-dev libffi-dev zlibc libglib2.0 \
    zlib1g zlib1g-dev libboost-all-dev libboost-dev libc6-dbg \
    subversion sudo unzip vim wget && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Specify locales
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Install cmake 3.12.2
RUN mkdir -p /home/sip/.cmake && \
    wget https://cmake.org/files/v3.12/cmake-3.12.2-Linux-x86_64.sh -P /home/sip/.cmake && \
    /bin/sh /home/sip/.cmake/cmake-3.12.2-Linux-x86_64.sh --prefix=/home/sip/.cmake --skip-license && \
    ln /home/sip/.cmake/bin/cmake /usr/local/bin/cmake && \
    ln -s /home/sip/.cmake/share/cmake-3.12/ /usr/local/share/cmake-3.12 && \
    rm /home/sip/.cmake/cmake-3.12.2-Linux-x86_64.sh

# Install llvm 6.0 (based on http://apt.llvm.org/)
RUN echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" >> /etc/apt/sources.list && \
    echo "deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" >> /etc/apt/sources.list && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN apt-get update && apt-get install -y \
    libllvm6.0 llvm-6.0 llvm-6.0-dev llvm-6.0-doc llvm-6.0-examples llvm-6.0-runtime \
    clang-6.0 clang-tools-6.0 clang-6.0-doc libclang-common-6.0-dev libclang-6.0-dev \
    libclang1-6.0 clang-format-6.0 python-clang-6.0 \
    libfuzzer-6.0-dev lldb-6.0 lld-6.0 && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Add a non-root user
RUN useradd --create-home --shell /bin/bash sip && \
    adduser sip sudo && \
    echo 'sip:sip' | chpasswd


WORKDIR /home/sip

#################################
### off-tree-obfuscations ###
#################################

RUN git clone https://github.com/mr-ma/offtree-o-llvm.git && \
    mkdir -p /home/sip/offtree-o-llvm/passes/build && \
    cmake -H/home/sip/offtree-o-llvm/passes -B/home/sip/offtree-o-llvm/passes/build && \
    make -C /home/sip/offtree-o-llvm/passes/build 

# Switch to user sip
RUN chown -R sip:sip /home/sip/
USER sip
WORKDIR /home/sip/offtree-o-llvm




