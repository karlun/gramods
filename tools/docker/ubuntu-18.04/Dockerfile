FROM ubuntu:18.04

RUN \
  apt-get update; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install cmake git build-essential ninja-build gcc-8 g++-8 doxygen graphviz; \
  DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libgtest-dev libtclap-dev libtinyxml2-dev libsdl2-dev libeigen3-dev libglew-dev libuvc-dev libfreeimage-dev inkscape gimp; \
  update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 80 --slave /usr/bin/g++ g++ /usr/bin/g++-8 --slave /usr/bin/gcov gcov /usr/bin/gcov-8

RUN git clone https://github.com/chriskohlhoff/asio.git -b asio-1-12-0
RUN \
  git clone https://github.com/openscenegraph/OpenSceneGraph.git -b OpenSceneGraph-3.6.5; \
  mkdir OpenSceneGraph/build; \
  cd OpenSceneGraph/build; \
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/osg-install \
    .. ; \
  ninja install
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

ARG GM_GIT_BRANCH=master
ARG GM_DEPLOY_KEY
ARG GM_DEPLOY_TOKEN

RUN \
  git clone https://$GM_DEPLOY_KEY:$GM_DEPLOY_TOKEN@gitlab.liu.se/karlu20/gramods.git -b $GM_GIT_BRANCH; \
  mkdir gramods/build; \
  cd gramods; \
  git apply tools/patches/2020-11-16_ubuntu-18.04-fixes.patch; \
  cd build; \
  cmake \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/gramods-install \
    -D ASIO_INCLUDE_DIRS=/asio/asio/include \
    -D OSG_INCLUDE_DIR:PATH=/osg-install/include \
    -D OSG_LIBRARY:FILEPATH=/osg-install/lib/libosg.so \
    -D OSGGA_LIBRARY:FILEPATH=/osg-install/lib/libosgGA.so \
    -D OSGGA_INCLUDE_DIR:PATH=/osg-install/include \
    -D OSGVIEWER_LIBRARY:FILEPATH=/osg-install/lib/libosgViewer.so \
    -D OSGVIEWER_INCLUDE_DIR:PATH=/osg-install/include \
    -D OSGDB_LIBRARY:FILEPATH=/osg-install/lib/libosgDB.so \
    -D OSGDB_INCLUDE_DIR:PATH=/osg-install/include \
    -D OPENTHREADS_LIBRARY:FILEPATH=/osg-install/lib/libOpenThreads.so \
    -D OPENTHREADS_INCLUDE_DIR:PATH=/osg-install/include \
    -D VRPN_INCLUDE_DIR:PATH=/vrpn-install/include \
    -D VRPN_LIBRARY:FILEPATH=/vrpn-install/lib/libvrpn.a \
    -D VRPN_SERVER_LIBRARY:FILEPATH=/vrpn-install/lib/libvrpnserver.a \
    -D QUAT_INCLUDE_DIR:PATH=/vrpn-install/include \
    -D QUAT_LIBRARY:FILEPATH=/vrpn-install/lib/libquat.a \
    -D gramods_BUILD_DOCUMENTATION:BOOL=ON \
    -D gramods_BUILD_RESOURCES:BOOL=ON \
    -D gramods_INSTALL_DOCUMENTATION:BOOL=ON \
    -D gramods_INSTALL_RESOURCES:BOOL=ON \
    .. ; \
  mkdir -p /gramods/build/modules/gmGraphics/resources/
RUN \
  ninja -C /gramods/build; \
  ninja -C /gramods/build install
