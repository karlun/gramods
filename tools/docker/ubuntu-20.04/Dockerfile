FROM ubuntu:18.04

RUN \
  apt-get update; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install cmake git build-essential ninja-build doxygen graphviz; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libgtest-dev libtclap-dev libtinyxml2-dev libsdl2-dev libeigen3-dev libglew-dev libuvc-dev libfreeimage-dev inkscape gimp

ARG GM_GIT_BRANCH=master
ARG GM_DEPLOY_KEY
ARG GM_DEPLOY_TOKEN

RUN \
  git clone https://$GM_DEPLOY_KEY:$GM_DEPLOY_TOKEN@gitlab.liu.se/karlu20/gramods.git -b $GM_GIT_BRANCH; \
  mkdir gramods/build;\
  cd gramods/build;\
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/gramods-install \
    .. ; \
  mkdir -p /gramods/build/modules/gmGraphics/resources
RUN \
  make -C gramods/build; \
  make -C gramods/build install
