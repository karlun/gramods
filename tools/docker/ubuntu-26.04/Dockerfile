FROM ubuntu:26.04 AS build-stage

RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" \
    apt-get -y install \
      build-essential \
      cmake \
      doxygen \
      git \
      graphviz \
      ninja-build \
      ;
RUN DEBIAN_FRONTEND="noninteractive" \
    apt-get -y install \
      inkscape \
      libasio-dev \
      libeigen3-dev \
      libfreeimage-dev \
      libglew-dev \
      libgtest-dev \
      libopenscenegraph-dev \
      libopenvr-dev \
      libsdl2-dev \
      libtclap-dev \
      libtinyxml2-dev \
      libuvc-dev \
      nlohmann-json3-dev \
      ;

RUN git clone https://github.com/vrpn/vrpn.git \
      -b v07.36 /vrpn.git
RUN cmake \
      -S /vrpn.git \
      -B /vrpn.git/build \
      -G Ninja \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX=/vrpn-install \
      -D VRPN_USE_LIBUSB_1_0:BOOL=OFF \
      ;
RUN ninja -C /vrpn.git/build install

ARG GIT_BRANCH=main
ARG GIT_COMMIT=unspecified
LABEL git_branch=$GIT_BRANCH
LABEL git_commit=$GIT_COMMIT

RUN --mount=type=secret,id=GIT_AUTH \
    git clone https://$(cat /run/secrets/GIT_AUTH)@gitlab.liu.se/karlu20/gramods.git \
      -b $GIT_BRANCH /gramods.git
RUN cmake \
      -S /gramods.git \
      -B /gramods.git/build \
      -G Ninja \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX:PATH=/gramods \
      -D FreeImage_DIR:PATH=/gramods.git/cmake_modules/freeimage-shared/ \
      -D VRPN_DIR:PATH=/gramods.git/cmake_modules/vrpn-static/ \
      -D CMAKE_PREFIX_PATH=/vrpn-install \
      ;

RUN mkdir -p /gramods.git/build/modules/gmGraphics/resources
RUN ninja -C /gramods.git/build
RUN ninja -C /gramods.git/build install
RUN tar cf gramods-ubuntu-26.04.tar gramods
