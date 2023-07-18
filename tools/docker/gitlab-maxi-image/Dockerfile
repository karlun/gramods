FROM ubuntu:22.04 AS build-stage

RUN \
  apt-get update; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install cmake git build-essential ninja-build doxygen graphviz; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libgtest-dev libtclap-dev libtinyxml2-dev libsdl2-dev libsdl2-ttf-dev libeigen3-dev libglew-dev libasio-dev libopenscenegraph-dev libuvc-dev libfreeimage-dev inkscape pipenv

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
