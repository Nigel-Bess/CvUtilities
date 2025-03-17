ARG BASE_IMAGE=nvidia/cuda:11.4.3-base-ubuntu20.04
ARG TAG=latest
ARG CMAKE_VERSION=3.22
ARG CMAKE_BUILD=0
ARG OPENCV_VERSION=4.6.0
ARG GRPC_VERSION=${GRPC_VERSION:-1.54.0}
ARG VIMBA_NAME=VimbaX_2024-1

# base
FROM ${BASE_IMAGE} AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y make wget unzip git protobuf-compiler libprotobuf-dev libcurl4-openssl-dev libspdlog-dev libeigen3-dev g++ gcc libssl-dev

# CMake
ARG CMAKE_VERSION
ARG CMAKE_BUILD
WORKDIR /tmp/
RUN wget https://cmake.org/files/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz
RUN tar -xzf cmake-$CMAKE_VERSION.$CMAKE_BUILD.tar.gz
WORKDIR /tmp/cmake-$CMAKE_VERSION.$CMAKE_BUILD
RUN ./bootstrap
RUN make -j$(nproc) && make install

# OpenCV
ARG OPENCV_VERSION
RUN apt-get install -y cmake
WORKDIR /opencv
RUN wget -O opencv.zip https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip \
    && wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/${OPENCV_VERSION}.zip \
    && unzip opencv.zip \
    && unzip opencv_contrib.zip \
    && mv opencv-${OPENCV_VERSION} opencv \
    && mv opencv_contrib-${OPENCV_VERSION} opencv_contrib

RUN mkdir /opencv/opencv/build
WORKDIR /opencv/opencv/build
RUN cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D INSTALL_C_EXAMPLES=OFF \
    -D OPENCV_ENABLE_NONFREE=ON \
    -D OPENCV_GENERATE_PKGCONFIG=ON \
    -D OPENCV_EXTRA_MODULES_PATH=/opencv/opencv_contrib/modules \
    -D PYTHON_EXECUTABLE=/usr/local/bin/python \
    -D BUILD_EXAMPLES=ON .. \
    && make -j$(nproc) && make install && ldconfig

# Protobuf / gRPC
ARG GRPC_VERSION
RUN apt-get install -y cmake
WORKDIR /home/fulfil/
RUN git clone --recurse-submodules -b v${GRPC_VERSION} --depth 1 --shallow-submodules https://github.com/grpc/grpc
RUN mkdir -p grpc/cmake/build
WORKDIR /home/fulfil/grpc/cmake/build
RUN cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local ../..
RUN make -j $(nproc) && make install

# VMB
ARG VIMBA_NAME
WORKDIR /home/fulfil/
COPY ./VimbaX_Setup-2024-1-Linux64.tar.gz /home/fulfil/
RUN tar xzf /home/fulfil/VimbaX_Setup-2024-1-Linux64.tar.gz
WORKDIR /home/fulfil/$VIMBA_NAME/
RUN ./cti/Install_GenTL_Path.sh

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
COPY Fulfil.Dispense/ ./Fulfil.Dispense/
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
RUN mkdir Fulfil.TCS
COPY Fulfil.TCS/VimbaX_Setup-2024-1-Linux64.tar.gz ./Fulfil.TCS/

RUN cd Fulfil.TCS && tar -xvf VimbaX_Setup-2024-1-Linux64.tar.gz
RUN cp -r ./Fulfil.TCS/VimbaX_2024-1 /home/fulfil

ENV GENICAM_GENTL64_PATH="/home/fulfil/VimbaX_2024-1/cti"
ENV VIMBA_HOME="/home/fulfil/VimbaX_2024-1"

RUN cd /home/fulfil/VimbaX_2024-1/cti/ && bash Install_GenTL_Path.sh
COPY third-party/ ./third-party/
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN sed -i 's/\r//' ./scripts/build_date.sh

ENV PATH="/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/build/:${PATH}"
ENV GENICAM_GENTL64_PATH="/home/fulfil/VimbaX_2024-1/cti"
RUN mkdir -p /code/mars/vimba

# Install python3 and all deps used in Fulfil.TCS/scripts
RUN apt-get update -y
RUN apt-get install python3 python3-pip -y
RUN pip3 install requests

WORKDIR /home/fulfil/code/Fulfil.ComputerVision

# Build latest TCS
COPY Fulfil.TCS/ ./Fulfil.TCS/
RUN cd Fulfil.TCS/ \
    && mkdir -p build/ \
    && make

CMD ["./Fulfil.TCS/build/main"]
