FROM ubuntu:22.04 AS build-stage

RUN \
  apt-get update; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install cmake git build-essential ninja-build doxygen graphviz; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libgtest-dev libasio-dev libtclap-dev libtinyxml2-dev libsdl2-dev libeigen3-dev libglew-dev libuvc-dev libfreeimage-dev libopenscenegraph-dev inkscape

RUN \
  git clone https://github.com/vrpn/vrpn.git -b v07.35; \
  mkdir vrpn/build; \
  cd vrpn/build; \
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/vrpn-install \
    .. ; \
  ninja install

ARG GM_GIT_BRANCH=main
ARG GM_DEPLOY_KEY
ARG GM_DEPLOY_TOKEN

RUN \
  git clone https://$GM_DEPLOY_KEY:$GM_DEPLOY_TOKEN@gitlab.liu.se/karlu20/gramods.git -b $GM_GIT_BRANCH gramods.git; \
  mkdir gramods.git/build;\
  cd gramods.git/build;\
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX:PATH=/gramods \
    -D TinyXML2_DIR:PATH=/gramods.git/cmake_modules/tinyxml2-shared/ \
    -D FreeImage_DIR:PATH=/gramods.git/cmake_modules/freeimage-shared/ \
    -D VRPN_DIR:PATH=/gramods.git/cmake_modules/vrpn-static/ \
    -D CMAKE_PREFIX_PATH=/vrpn-install \
    .. ; \
  mkdir -p /gramods.git/build/modules/gmGraphics/resources
RUN \
  ninja -C gramods.git/build; \
  ninja -C gramods.git/build install
RUN \
  tar cf gramods-ubuntu-22.04.tar gramods
