FROM gcr.io/fulfil-web/nvidia-cv/master:latest AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y udev libgl1-mesa-dev make curl wget unzip git protobuf-compiler libprotobuf-dev libcurl4-openssl-dev libspdlog-dev libeigen3-dev g++ gcc libssl-dev libjpeg8 libpng-dev unzip libcurl4-openssl-dev libspdlog-dev libeigen3-dev

# Install Orbbec SDK
ENV ORBBEC_SDK_VERSION=v2.4.11
WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
RUN mkdir orbbec

RUN cd orbbec && \
    git clone https://github.com/Orbbec/OrbbecSDK_v2.git && \
    cd OrbbecSDK_v2 && mkdir build && cd build && \
    cmake .. && cmake --build . --config Release -j$(($(nproc)-1))

RUN /lib/systemd/systemd-udevd --daemon && udevadm trigger && cd orbbec/OrbbecSDK_v2/scripts && chmod -R +x ./env_setup && ./env_setup/install_udev_rules.sh && ./env_setup/setup.sh
ENV LD_LIBRARY_PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/lib:$LD_LIBRARY_PATH
ENV LD_LIBRARY_PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/lib:$LD_LIBRARY_PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/bin:$PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/bin:$PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/linux_x86_64/bin:$PATH
ENV PATH=/home/fulfil/code/Fulfil.ComputerVision/orbbec/OrbbecSDK_v2/build/linux_x86_64/lib:$PATH
# Reload linux libs
RUN ldconfig

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
# COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
# COPY Fulfil.Dispense/ ./Fulfil.Dispense/

COPY third-party ./third-party/

# Install MongoDB C++ Driver
RUN curl -fsSL https://pgp.mongodb.com/server-7.0.asc | \
    gpg -o /usr/share/keyrings/mongodb-server-7.0.gpg --dearmor
RUN echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-7.0.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/7.0 multiverse" \
| tee /etc/apt/sources.list.d/mongodb-org-7.0.list

WORKDIR /home/fulfil/code/Fulfil.ComputerVision
ENV MDB_VERSION="3.10.0"
RUN curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r${MDB_VERSION}/mongo-cxx-driver-r${MDB_VERSION}.tar.gz
RUN tar -xzf mongo-cxx-driver-r${MDB_VERSION}.tar.gz
RUN cd mongo-cxx-driver-r${MDB_VERSION} && cmake . -DCMAKE_BUILD_TYPE=Release -DMONGOCXX_OVERRIDE_DEFAULT_INSTALL_PREFIX=OFF && cmake --build . && cmake --build . -j$(($(nproc)-1)) --target install
# I don't understand why this hack is needed for the runtime to see the mongo driver lib, but it's important
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# Build MongoCpp
WORKDIR /home/fulfil/code/Fulfil.ComputerVision
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY third-party ./Fulfil.MongoCpp/third-party/
RUN cd Fulfil.MongoCpp && mkdir -p build
RUN cd Fulfil.MongoCpp && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1)
RUN cd Fulfil.MongoCpp && cmake --build . -j$(($(nproc)-1))
RUN cd Fulfil.MongoCpp && cmake --install .

# Build CPPUtils + OrbbecUtils
WORKDIR /home/fulfil/code/Fulfil.ComputerVision
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
COPY Fulfil.OrbbecUtils/ ./Fulfil.OrbbecUtils/


RUN cp Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h.template Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN sed -i 's/\r//' ./scripts/build_date.sh
#COPY .git .git
RUN cd scripts && bash build_date.sh

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



# Remove broken old tests
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/test
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/test
#COPY .git .git

RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.OrbbecUtils && mkdir -p build/ \
    && cd build && cmake .. || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.OrbbecUtils/CMakeFiles/CMakeError.log && exit 1) && cmake --build . -j$(($(nproc)-1))
RUN cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils && mkdir -p build/ \
    && cd build && cmake .. || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/CMakeFiles/CMakeError.log && exit 1) && cmake --build . -j$(($(nproc)-1))

RUN mkdir Fulfil.TCS

COPY third-party/ ./third-party/
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN sed -i 's/\r//' ./scripts/build_date.sh

# Install all deps used in Fulfil.TCS/scripts
RUN pip3 install requests

WORKDIR /home/fulfil/code/Fulfil.ComputerVision

# Build latest TCS
COPY Fulfil.TCS/ ./Fulfil.TCS/
RUN mkdir -p /code/mars/vimba
ENV PATH="/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/build/:${PATH}"

RUN cd Fulfil.TCS/ \
    && mkdir -p build/ \
    && cd build && cmake ..
RUN cd Fulfil.TCS/build && cmake . && cmake --build . -j$(($(nproc)-1))

CMD ["./Fulfil.TCS/build/tcs"]