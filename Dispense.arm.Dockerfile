# Note! This file is a hack! Ideally we'd use Dispense.original.amd.Dockerfile, however the Realsense driver
# suddenly broke horribly, so we instead base this hack Dockerfile on the last known good image that has
# a valid Realsense driver pre-installed, the original file represents when we last built from scratch, and ideally
# we end up back on it but for now this hack Dockerfile built on top of the "magic" realsense image works...
FROM gcr.io/fulfil-web/realsense-orbbec-cv-arm:latest AS base

# Install Ninja build system
RUN apt-get update && apt-get install -y ninja-build ccache && rm -rf /var/lib/apt/lists/*

ENV CCACHE_DIR=/ccache \ CCACHE_MAXSIZE=10G

WORKDIR /home/fulfil/code/Fulfil.ComputerVision

# Remove stale app layers baked into magic base image, but be smart about it and leave
# Cmake cache and build files around to warm up CMake builds
COPY third-party third-partyNew
COPY Fulfil.MongoCpp ./Fulfil.MongoCppNew
RUN rsync -r third-partyNew/ third-party/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf third-partyNew && \
    rsync -r Fulfil.MongoCppNew/ Fulfil.MongoCpp/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.MongoCppNew

# Build MongoCpp
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY third-party ./Fulfil.MongoCpp/third-party/
RUN cd Fulfil.MongoCpp && \
    mkdir -p build && \
    rm -f "CMakeCache.txt" && \
    rm -f build/CMakeCache.txt && \
    cd build && \
    cmake \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        .. \
    || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1) && \
    cmake --build . -j$(($(nproc)-1))

# CPPUtils sync
COPY Fulfil.CPPUtils ./Fulfil.CPPUtilsNew
RUN rsync -r Fulfil.CPPUtilsNew/ Fulfil.CPPUtils/ \
      --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && \
    rm -rf ./Fulfil.CPPUtilsNew

RUN cp Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h.template Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN cd scripts && sed -i 's/\r//' ./build_date.sh && bash build_date.sh
# Maybe bring this back in some form, but it thrashes the docker layer caching constantly leading to mega slow development, instead we
# should depend on docker image names and tags to check what builds are running going forward, else flip this back on and the runtime
# will become aware of it's git branch again
#COPY .git .git

#Build CPPUtils
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
RUN cd Fulfil.CPPUtils && \
    mkdir -p build && \
    rm -f "CMakeCache.txt" && \
    rm -f build/CMakeCache.txt && \
    cd build && \
    cmake \
        -DFULFIL_LOG_DIR=/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs \
        -DFULFIL_INI_DIR=/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        .. \
    || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/CMakeFiles/CMakeError.log && exit 1) && \
    cmake --build . -j$(($(nproc)-1))

COPY Fulfil.OrbbecUtils ./Fulfil.OrbbecUtilsNew
RUN rsync -r Fulfil.OrbbecUtilsNew/ Fulfil.OrbbecUtils/ \
      --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && \
    rm -rf ./Fulfil.OrbbecUtilsNew

#RUN ls orbbec/OrbbecSDK_v2/build/linux_arm64 && exit 1 # bin and lib
RUN mkdir -p Fulfil.OrbbecUtils/lib/orbbecsdk/lib && \
    mkdir -p Fulfil.OrbbecUtils/lib/orbbecsdk/bin && \
    cp -r orbbec/OrbbecSDK_v2/* \
          Fulfil.OrbbecUtils/lib/orbbecsdk && \
    cp -r orbbec/OrbbecSDK_v2/build/linux_arm64/lib/* \
          Fulfil.OrbbecUtils/lib/orbbecsdk/lib && \
    cp -r orbbec/OrbbecSDK_v2/build/linux_arm64/bin/* \
          Fulfil.OrbbecUtils/lib/orbbecsdk/bin && \
    cp -r /home/fulfil/code/Fulfil.ComputerVision/Fulfil.OrbbecUtils/lib/orbbecsdk/build/src/CMakeFiles/Export/lib/* \
          Fulfil.OrbbecUtils/lib/orbbecsdk/lib


# Build OrbbecUtils
COPY Fulfil.OrbbecUtils ./Fulfil.OrbbecUtils
RUN cd Fulfil.OrbbecUtils && \
    mkdir -p build && \
    rm -f "CMakeCache.txt" && \
    rm -f build/CMakeCache.txt && \
    cd build && \
    cmake \
        -DFULFIL_LOG_DIR=/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs \
        -DFULFIL_INI_DIR=/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        .. \
    || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.OrbbecUtils/CMakeFiles/CMakeError.log && exit 1) && \
    cmake --build . -j$(($(nproc)-1))

# Remove broken old tests for now
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/test

# DepthCam sync
COPY Fulfil.DepthCam ./Fulfil.DepthCamNew
RUN rsync -r Fulfil.DepthCamNew/ Fulfil.DepthCam/ \
      --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && \
    rm -rf ./Fulfil.DepthCamNew

# Build DepthCam lib
RUN --mount=type=cache,target=/ccache \
    cp -r third-party Fulfil.DepthCam/third-party && \
    mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense && \
    cp -r /usr/src/librealsense/build/* /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense && \
    cp /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/cmake_install.cmake /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/CMakeLists.txt && \
    cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && \
    cmake -S . -B build \
        -DIS_CONTAINERIZED=1 \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
    cmake --build build -j$(($(nproc)-1))

# Dispense sync
COPY Fulfil.Dispense ./Fulfil.DispenseNew
RUN rsync -r Fulfil.DispenseNew/ Fulfil.Dispense/ \
      --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && \
    rm -rf ./Fulfil.DispenseNew

# Build Dispense app
RUN --mount=type=cache,target=/ccache \
    cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && \
    cmake -S . -B build \
        -DBUILD_TESTS=ON \
        -DFULFIL_LOG_DIR=/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs \
        -DFULFIL_INI_DIR=/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
    cmake --build build -j$(($(nproc)-1))

# Harmless test run setup stuff
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs/tray_test_logs && \
    mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs/csvs

CMD ["./Fulfil.Dispense/build/app/main"]
