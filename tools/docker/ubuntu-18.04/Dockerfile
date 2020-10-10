FROM ubuntu:18.04
ENV GIT_BRANCH master
ENV DEPLOY_KEY gitlab+deploy-token-74
ENV DEPLOY_TOKEN ZsEheLySLsz1hFRMY9bZ

RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install cmake git build-essential doxygen graphviz
RUN DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libgtest-dev
RUN DEBIAN_FRONTEND="noninteractive" \
  apt-get -y install libtclap-dev libtinyxml2-dev libsdl2-dev libeigen3-dev libglew-dev libuvc-dev libfreeimage-dev inkscape gimp

RUN git clone https://$DEPLOY_KEY:$DEPLOY_TOKEN@gitlab.liu.se/karlu20/gramods.git -b $GIT_BRANCH
RUN mkdir gramods/build
RUN cd gramods/build;\
  cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/gramods-install \
    -D CPACK_DEBIAN_PACKAGE_DEPENDS="libtinyxml2-6a, libsdl2-2.0-0, libglew2.1, libuvc0, libfreeimage3" ..
RUN mkdir -p /gramods/build/modules/gmGraphics/resources/
RUN make -C gramods/build
RUN make -C gramods/build docs
RUN make -C gramods/build install
