FROM ubuntu:24.04 AS build-stage

RUN \
    apt-get update; \
    DEBIAN_FRONTEND="noninteractive" \
    apt-get -y install \
    build-essential \
    cmake \
    doxygen \
    git \
    graphviz \
    libgtest-dev \
    ninja-build \
    ;
RUN \
    DEBIAN_FRONTEND="noninteractive" \
    apt-get -y install \
    inkscape \
    libasio-dev \
    libeigen3-dev \
    libfreeimage-dev \
    libglew-dev \
    libopenscenegraph-dev \
    libsdl2-dev \
    libsdl2-ttf-dev \
    libtclap-dev \
    libtinyxml2-dev \
    libuvc-dev \
    pipenv \
    ;

RUN \
  git clone https://github.com/vrpn/vrpn.git -b v07.36; \
  mkdir vrpn/build; \
  cd vrpn/build; \
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/vrpn-install \
    .. ; \
  ninja install
