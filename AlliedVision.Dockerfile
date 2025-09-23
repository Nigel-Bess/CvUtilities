FROM gcr.io/fulfil-web/nvidia-cv/master:latest AS base

# Mongo stuff
RUN apt install curl -y && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/*
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
RUN cd Fulfil.MongoCpp && mkdir -p build && \
    cmake . || (cat /home/fulfil/code/Fulfil.ComputerVision/Fulfil.MongoCpp/CMakeFiles/CMakeError.log && exit 1) && \
    cmake --build . -j$(($(nproc)-1)) && \
    cmake --install .

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
COPY Fulfil.Dispense/ ./Fulfil.Dispense/
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
RUN mkdir Fulfil.AlliedVision
COPY Fulfil.AlliedVision/VimbaX_Setup-2024-1-Linux64.tar.gz ./Fulfil.AlliedVision/

RUN cd Fulfil.AlliedVision && tar -xvf VimbaX_Setup-2024-1-Linux64.tar.gz && \
    cp -r ./VimbaX_2024-1 /home/fulfil

COPY third-party/ ./third-party/
COPY scripts/build_date.sh ./scripts/build_date.sh
RUN sed -i 's/\r//' ./scripts/build_date.sh

# Install python3 and all deps used in Fulfil.AlliedVision/scripts
RUN pip3 install requests

# Build latest AlliedVision
COPY Fulfil.AlliedVision/ ./Fulfil.AlliedVision/
ENV PATH="/home/fulfil/code/Fulfil.ComputerVision/Fulfil.AlliedVision/build/:${PATH}"
RUN cd Fulfil.AlliedVision/ \
    && mkdir -p build/ \
    && make

RUN mkdir -p /code/mars/vimba

WORKDIR /home/fulfil/code/Fulfil.ComputerVision
CMD ["./Fulfil.AlliedVision/build/stark_test"]
