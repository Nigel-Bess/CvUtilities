# Note! This file is a hack! Ideally we'd use Dispense.original.amd.Dockerfile, however the Realsense driver
# suddenly broke horribly, so we instead base this hack Dockerfile on the last known good image that has
# a valid Realsense driver pre-installed, the original file represents when we last built from scratch, and ideally
# we end up back on it but for now this hack Dockerfile built on top of the "magic" realsense image works...
FROM gcr.io/fulfil-web/cv-dispense/realsense-arm:latest AS base
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get install -y rsync libgl1-mesa-dev libjson-c-dev udev protobuf-compiler libprotobuf-dev libcurl4-openssl-dev libspdlog-dev libeigen3-dev g++ gcc libssl-dev curl make wget unzip git protobuf-compiler libprotobuf-dev libcurl4-openssl-dev libspdlog-dev libeigen3-dev g++ gcc libssl-dev

# Install Orbbec SDK
ENV ORBBEC_SDK_VERSION=v2.4.11
WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
RUN mkdir orbbec && cd orbbec && curl -o orbbec.zip https://storage.googleapis.com/public-libs/OrbbecSDK-${ORBBEC_SDK_VERSION}.zip
RUN cd orbbec && unzip orbbec.zip && rm orbbec.zip
RUN mv orbbec/OrbbecSDK_* orbbec/OrbbecSDK
RUN ./orbbec/OrbbecSDK/setup.sh

RUN cd orbbec && git clone  https://github.com/Orbbec/OrbbecSDK_v2.git
RUN cd orbbec && cd OrbbecSDK_v2 && mkdir build && cd build && cmake .. && cmake --build . --config Release -j$(($(nproc)-1))
RUN cd orbbec && cd OrbbecSDK_v2/build && cmake --build . -j$(($(nproc)-1))

RUN /lib/systemd/systemd-udevd --daemon && udevadm trigger && cd orbbec/OrbbecSDK_v2/scripts && chmod -R +x ./env_setup && ./env_setup/install_udev_rules.sh && ./env_setup/setup.sh
ENV LD_LIBRARY_PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/lib:$LD_LIBRARY_PATH
#ENV LD_LIBRARY_PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/linux_x86_64/lib:$LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/lib:$LD_LIBRARY_PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/bin:$PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/linux_x86_64/bin:$PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/bin:$PATH
# Reload linux libs
RUN ldconfig

# Fulfil.ComputerVision from here!
WORKDIR /home/fulfil/code/Fulfil.ComputerVision

# Remove stale app layers baked into magic base image, but be smart about it and leave
# Cmake cache and build files around to warm up CMake builds
COPY third-party third-partyNew
RUN rsync -r third-partyNew/ third-party/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf third-partyNew

COPY Fulfil.MongoCpp ./Fulfil.MongoCppNew
RUN rsync -r Fulfil.MongoCppNew/ Fulfil.MongoCpp/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.MongoCppNew

COPY Fulfil.CPPUtils ./Fulfil.CPPUtilsNew
RUN rsync -r Fulfil.CPPUtilsNew/ Fulfil.CPPUtils/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.CPPUtilsNew

COPY Fulfil.DepthCam ./Fulfil.DepthCamNew
RUN rsync -r Fulfil.DepthCamNew/ Fulfil.DepthCam/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.DepthCamNew

COPY Fulfil.Dispense ./Fulfil.DispenseNew
RUN rsync -r Fulfil.DispenseNew/ Fulfil.Dispense/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.DispenseNew

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
RUN cp -r orbbec/OrbbecSDK_v2 Fulfil.CPPUtils/OrbbecSDK
RUN mkdir -p Fulfil.CPPUtils/lib
RUN cp -r orbbec/OrbbecSDK Fulfil.CPPUtils/lib/orbbecsdk
# Remove broken old tests for now
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/test
# For now don't bother building since Depthcam can't just add_project and avoid rebuilding without a lot of refactoring,
# so leave it to the DepthCam lib build to also build CPPUtils :-(
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && mkdir -p build/ && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/CMakeFiles/CMakeError.log && exit 1)
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && make
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && cmake --build . -j$(($(nproc)-1))

# Build DepthCam lib
RUN cp -r third-party Fulfil.DepthCam/third-party
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense
RUN cp -r /usr/src/librealsense/build/* /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense
RUN cp /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/cmake_install.cmake /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/CMakeLists.txt
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && mkdir -p build && cmake -DIS_CONTAINERIZED=1 .
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && cmake --build . -j$(($(nproc)-1)) || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/CMakeFiles/CMakeOutput.log && exit 1)

# Build Dispense
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/CMakeFiles/CMakeError.log && exit 1)
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && cmake --build . -j$(($(nproc)-1))

# Harmless test run setup stuff

CMD ["./Fulfil.Dispense/app/main"]
