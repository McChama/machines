FROM ubuntu:24.04

USER root

RUN apt-get update --fix-missing && \
    apt-get install -y \
        sudo \
        build-essential \
        file \
        apt-utils \
        git \
        git-lfs \
        cmake \
        cmake-curses-gui \
        mingw-w64 \
        libsdl2-dev \
        libsdl2-image-dev \
        libopenal-dev \
        libalure-dev \
        libenet-dev \
        libfreetype-dev \
        libglew-dev \
        libglm-dev \
        libaudio-dev

# Passwordless sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers


RUN useradd -ms /bin/bash -G sudo dev
USER dev
WORKDIR /home/dev
