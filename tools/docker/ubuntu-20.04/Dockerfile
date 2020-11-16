FROM ubuntu:20.04

RUN \
  apt-get update; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install cmake git build-essential ninja-build doxygen graphviz; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libgtest-dev libasio-dev libtclap-dev libtinyxml2-dev libsdl2-dev libeigen3-dev libglew-dev libuvc-dev libfreeimage-dev libopenscenegraph-dev inkscape

RUN \
  git clone https://github.com/vrpn/vrpn.git; \
  mkdir vrpn/build; \
  cd vrpn/build; \
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/vrpn-install \
    .. ; \
  ninja install

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
    -D VRPN_INCLUDE_DIR:PATH=/vrpn-install/include \
    -D VRPN_LIBRARY:FILEPATH=/vrpn-install/lib/libvrpn.a \
    -D VRPN_SERVER_LIBRARY:FILEPATH=/vrpn-install/lib/libvrpnserver.a \
    -D QUAT_INCLUDE_DIR:PATH=/vrpn-install/include \
    -D QUAT_LIBRARY:FILEPATH=/vrpn-install/lib/libquat.a \
    .. ; \
  mkdir -p /gramods/build/modules/gmGraphics/resources
RUN \
  ninja -C gramods/build; \
  ninja -C gramods/build install
