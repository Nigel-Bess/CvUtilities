# The original (and intended) Dockerfile for Dispense. At some point we lost the ability to run
# the freshly built-from-source Realsense driver from this file and so we've stuffed the last
# known good build of this file into cv-dispense/realsense-amd image 
# from which the current hacky canonical Dockerfile
# derives so it can skip building otherwise bad Realsense drivers.  If the RS driver is no
# longer needed or solved for this should once again become the proper canonical image!

FROM gcr.io/fulfil-web/nvidia-cv/master:latest AS base
ENV DEBIAN_FRONTEND=noninteractive

# Realsense - NOTE! This copies from a Ub 20.04 base image which happens to match the CUDA base image here,
# proceed with caution when updating this fragile alliance of deps.
RUN apt-get update -y
RUN apt-get install -qq -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libusb-1.0-0-dev \
    pkg-config \
    libgtk-3-dev \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \    
    curl \
    python3 \
    python3-dev \
    ffmpeg \
    ca-certificates 

RUN LIBREALSENSE_VERSION=2.54.2
WORKDIR /usr/src
# Get the latest tag of remote repository: https://stackoverflow.com/a/12704727
# Needs to be a single command as ENV can't be set from Bash command: https://stackoverflow.com/questions/34911622/dockerfile-set-env-to-result-of-command
RUN export LIBRS_GIT_TAG=`git -c 'versionsort.suffix=-' \
                         ls-remote --exit-code --refs --sort='version:refname' --tags https://github.com/IntelRealSense/librealsense '*.*.*' \
                         | tail --lines=1 \
                         | cut --delimiter='/' --fields=3`; \
    export LIBRS_VERSION=${LIBRS_VERSION:-${LIBRS_GIT_TAG#"v"}}; \
    curl https://codeload.github.com/IntelRealSense/librealsense/tar.gz/refs/tags/v${LIBRS_VERSION} -o librealsense.tar.gz; \
    tar -zxf librealsense.tar.gz; \
    rm librealsense.tar.gz; \
    ln -s /usr/src/librealsense-${LIBRS_VERSION} /usr/src/librealsense

# Don't ask, inject OpenGL dep to cmake file that we suddenly started needing, gross job Intel!
RUN sed -i '8 i find_package\(OpenGL REQUIRED\)' /usr/src/librealsense/CMakeLists.txt
RUN cd /usr/src/librealsense \
 && mkdir build && cd build \
 && cmake \
    -DCMAKE_C_FLAGS_RELEASE="${CMAKE_C_FLAGS_RELEASE} -s" \
    -DCMAKE_CXX_FLAGS_RELEASE="${CMAKE_CXX_FLAGS_RELEASE} -s" \
    -DCMAKE_INSTALL_PREFIX=/opt/librealsense \    
    -DBUILD_GRAPHICAL_EXAMPLES=OFF \
    -DBUILD_PYTHON_BINDINGS:bool=false \
    -DCMAKE_BUILD_TYPE=Release ../ && make -j$(($(nproc)-1)) all && make install

# Install MongoDB C++ Driver
RUN curl -fsSL https://pgp.mongodb.com/server-7.0.asc | \
    gpg -o /usr/share/keyrings/mongodb-server-7.0.gpg --dearmor
RUN echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-7.0.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/7.0 multiverse" \
| tee /etc/apt/sources.list.d/mongodb-org-7.0.list
RUN apt-get update
RUN apt-get install pkg-config libssl-dev libsasl2-dev openssl -y
RUN apt upgrade openssl libssl-dev libsasl2-dev -y

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp
ENV MDB_VERSION="3.10.0"
RUN curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r${MDB_VERSION}/mongo-cxx-driver-r${MDB_VERSION}.tar.gz
RUN tar -xzf mongo-cxx-driver-r${MDB_VERSION}.tar.gz
RUN cd mongo-cxx-driver-r${MDB_VERSION} && cmake . -DCMAKE_BUILD_TYPE=Release -DMONGOCXX_OVERRIDE_DEFAULT_INSTALL_PREFIX=OFF && cmake --build . && cmake --build . -j$(($(nproc)-1)) --target install

# Fulfil.ComputerVision from here!
WORKDIR /home/fulfil/code/Fulfil.ComputerVision
COPY third-party ./third-party/

# Build MongoCpp
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY third-party ./Fulfil.MongoCpp/third-party/
RUN cd Fulfil.MongoCpp && mkdir -p build
# Don't actually bother building since Depthcam will forcably need to rebuild anyway without a lot of refactoring :-(
RUN cd Fulfil.MongoCpp && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1)
RUN cd Fulfil.MongoCpp && cmake --build . -j$(($(nproc)-1))
RUN cd Fulfil.MongoCpp && cmake --install .

# Build CPPUtils
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
RUN cp Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h.template Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN sed -i 's/\r//' ./scripts/build_date.sh
# Maybe bring this back in some form, but it thrashes the docker layer caching constantly leading to mega slow development, instead we
# should depend on docker image names and tags to check what builds are running going forward, else flip this back on and the runtime
# will become aware of it's git branch again
#COPY .git .git
RUN cd scripts && bash build_date.sh
RUN cp -r /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp ./Fulfil.CPPUtils/Fulfil.MongoCpp
# Remove broken old tests for now
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/test
# For now don't bother building since Depthcam can't just add_project and avoid rebuilding without a lot of refactoring,
# so leave it to the DepthCam lib build to also build CPPUtils :-(
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && mkdir -p build/ && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/CMakeFiles/CMakeError.log && exit 1)
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && make
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && cmake --build . -j$(($(nproc)-1))

# Build DepthCam lib
RUN apt-get install -qq -y --no-install-recommends libjson-c-dev
COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
COPY third-party ./Fulfil.DepthCam/third-party/
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense
RUN cp -r /usr/src/librealsense/build/* /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense
RUN cp /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/cmake_install.cmake /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/CMakeLists.txt
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && mkdir -p build && cmake -DIS_CONTAINERIZED=1 .
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && cmake --build . -j$(($(nproc)-1)) || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/CMakeFiles/CMakeOutput.log && exit 1)

# Build old Dispense to warm make build cache
RUN mkdir -p Fulfil.Dispense
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && mkdir -p build/
COPY Fulfil.Dispense/Fulfil.Dispense-snapshot.zip .
RUN unzip Fulfil.Dispense-snapshot.zip
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && cmake . && cmake --build .
COPY Fulfil.Dispense ./Fulfil.DispenseNew
RUN cp -r Fulfil.DispenseNew/* Fulfil.Dispense/
#COPY Fulfil.Dispense Fulfil.Dispense

# Build Dispense
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/CMakeFiles/CMakeError.log && exit 1)
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && cmake --build . -j$(($(nproc)-1))

# Harmless test run setup stuff
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs/tray_test_logs
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs/csvs

CMD ["./Fulfil.Dispense/app/main"]
