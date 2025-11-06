# Note! This file is a hack! Ideally we'd use Dispense.original.amd.Dockerfile, however the Realsense driver
# suddenly broke horribly, so we instead base this hack Dockerfile on the last known good image that has
# a valid Realsense driver pre-installed, the original file represents when we last built from scratch, and ideally
# we end up back on it but for now this hack Dockerfile built on top of the "magic" realsense image works...
FROM gcr.io/fulfil-web/realsense-orbbec-cv-amd:latest AS base

# Install Ninja build system
RUN apt-get update && apt-get install -y ninja-build

WORKDIR /home/fulfil/code/Fulfil.ComputerVision

# Remove stale app layers baked into magic base image, but be smart about it and leave
# Cmake cache and build files around to warm up CMake builds
COPY third-party third-partyNew
RUN rsync -r third-partyNew/ third-party/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf third-partyNew

COPY Fulfil.MongoCpp ./Fulfil.MongoCppNew
RUN rsync -r Fulfil.MongoCppNew/ Fulfil.MongoCpp/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.MongoCppNew

# Build MongoCpp
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY third-party ./Fulfil.MongoCpp/third-party/
RUN cd Fulfil.MongoCpp && mkdir -p build
RUN rm "Fulfil.MongoCpp/CMakeCache.txt"
# TODO: restore ninja
#RUN cd Fulfil.MongoCpp && cmake -GNinja . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1) && cmake --build . -j$(($(nproc)-1)) && cmake --install .
RUN cd Fulfil.MongoCpp && cmake -GNinja . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1) && cmake --build . -j$(($(nproc)-1)) && cmake --install .

COPY Fulfil.CPPUtils ./Fulfil.CPPUtilsNew
RUN rsync -r Fulfil.CPPUtilsNew/ Fulfil.CPPUtils/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.CPPUtilsNew

COPY Fulfil.DepthCam ./Fulfil.DepthCamNew
RUN rsync -r Fulfil.DepthCamNew/ Fulfil.DepthCam/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.DepthCamNew

COPY Fulfil.Dispense ./Fulfil.DispenseNew
RUN rsync -r Fulfil.DispenseNew/ Fulfil.Dispense/ --delete --exclude build --exclude "CMakeC*" --exclude "CMakeF*" && rm -rf ./Fulfil.DispenseNew

# Build CPPUtils + OrbbecUtils
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
COPY Fulfil.OrbbecUtils/ ./Fulfil.OrbbecUtils/
RUN cp Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h.template Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN cd scripts && sed -i 's/\r//' ./build_date.sh && bash build_date.sh
# Maybe bring this back in some form, but it thrashes the docker layer caching constantly leading to mega slow development, instead we
# should depend on docker image names and tags to check what builds are running going forward, else flip this back on and the runtime
# will become aware of it's git branch again
#COPY .git .git

#RUN ls orbbec/OrbbecSDK_v2/build/linux_x86_64 && exit 1 # bin and lib
RUN cp -r /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp ./Fulfil.CPPUtils/Fulfil.MongoCpp && \
    mkdir -p Fulfil.CPPUtils/lib
RUN mkdir -p Fulfil.OrbbecUtils/lib && \
    mkdir -p Fulfil.OrbbecUtils/lib/orbbecsdk && \
    mkdir -p Fulfil.OrbbecUtils/lib/orbbecsdk/lib && \
    mkdir -p Fulfil.OrbbecUtils/lib/orbbecsdk/bin && \
    cp -r orbbec/OrbbecSDK_v2/* Fulfil.OrbbecUtils/lib/orbbecsdk && \
    cp -r orbbec/OrbbecSDK_v2/build/linux_x86_64/lib/* Fulfil.OrbbecUtils/lib/orbbecsdk/lib && \
    cp -r orbbec/OrbbecSDK_v2/build/linux_x86_64/bin/* Fulfil.OrbbecUtils/lib/orbbecsdk/bin/ && \
    cp -r /home/fulfil/code/Fulfil.ComputerVision/Fulfil.OrbbecUtils/lib/orbbecsdk/build/src/CMakeFiles/Export/lib/* Fulfil.OrbbecUtils/lib/orbbecsdk/lib

# Remove broken old tests for now
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/test
# For now don't bother building since Depthcam can't just add_project and avoid rebuilding without a lot of refactoring,
# so leave it to the DepthCam lib build to also build CPPUtils :-(
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && mkdir -p build/ && cmake -GNinja . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/CMakeFiles/CMakeError.log && exit 1)
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && make
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && cmake --build . -j$(($(nproc)-1))

# Build DepthCam lib
RUN cp -r third-party Fulfil.DepthCam/third-party && mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense && cp -r /usr/src/librealsense/build/* /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense
RUN cp /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/cmake_install.cmake /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/libs/librealsense/CMakeLists.txt
# TODO: restore ninja
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && mkdir -p build && cmake -GNinja -DIS_CONTAINERIZED=1 .
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && mkdir -p build && cmake -DIS_CONTAINERIZED=1 .
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam && cmake --build . -j$(($(nproc)-1)) || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.DepthCam/CMakeFiles/CMakeOutput.log && exit 1)

# Build Dispense using Ninja
RUN rm -rf "Fulfil.Dispense/build/CMakeCache.txt"
# TODO: restore ninja
#RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && mkdir -p build && cd build && cmake -GNinja -DBUILD_TESTS=ON -H/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense -B/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense && mkdir -p build && cd build && cmake -DBUILD_TESTS=ON -H/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense -B/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build && cmake --build . -j$(($(nproc)-1))

# Harmless test run setup stuff
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs/tray_test_logs
RUN mkdir -p /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/logs/csvs

CMD ["./Fulfil.Dispense/build/app/main"]
