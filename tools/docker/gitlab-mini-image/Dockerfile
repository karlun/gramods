FROM ubuntu:26.04 AS build-stage

RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" \
    apt-get -y install \
      build-essential \
      cmake \
      doxygen \
      git \
      graphviz \
      libgtest-dev \
      ninja-build \
      ;
