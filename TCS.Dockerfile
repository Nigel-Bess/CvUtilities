FROM gcr.io/fulfil-web/nvidia-cv/master:latest AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y make wget unzip git protobuf-compiler libprotobuf-dev libcurl4-openssl-dev libspdlog-dev libeigen3-dev g++ gcc libssl-dev

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
COPY Fulfil.Dispense/ ./Fulfil.Dispense/
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
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
    && make

CMD ["./Fulfil.TCS/build/tcs"]
