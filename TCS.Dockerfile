FROM gcr.io/fulfil-web/nvidia-cv/master:latest AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y curl make wget unzip git protobuf-compiler libprotobuf-dev libcurl4-openssl-dev libspdlog-dev libeigen3-dev g++ gcc libssl-dev

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
# COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
# COPY Fulfil.Dispense/ ./Fulfil.Dispense/

COPY third-party ./third-party/

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

# Build MongoCpp
WORKDIR /home/fulfil/code/Fulfil.ComputerVision
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY third-party ./Fulfil.MongoCpp/third-party/
RUN cd Fulfil.MongoCpp && mkdir -p build
RUN cd Fulfil.MongoCpp && cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1)
RUN cd Fulfil.MongoCpp && cmake --build . -j$(($(nproc)-1))
RUN cd Fulfil.MongoCpp && cmake --install .

# Build CPPUtils
WORKDIR /home/fulfil/code/Fulfil.ComputerVision
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
RUN cp Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h.template Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN sed -i 's/\r//' ./scripts/build_date.sh
#COPY .git .git
RUN cd scripts && bash build_date.sh

RUN cp -r ./Fulfil.MongoCpp ./Fulfil.CPPUtils/
# Remove broken old tests
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.CPPUtils/test
RUN rm -rf /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/test
#COPY .git .git

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
#RUN ls Fulfil.CPPUtils/build && exit 1
#RUN cd Fulfil.TCS/build && cmake --build .  -j$(($(nproc)-1))
RUN cd Fulfil.TCS/build && cmake . && cmake --build . -j$(($(nproc)-1)) # && make

CMD ["./Fulfil.TCS/build/tcs"]
