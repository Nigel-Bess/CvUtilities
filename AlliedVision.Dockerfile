FROM gcr.io/fulfil-web/nvidia-cv/master:latest AS base

WORKDIR /home/fulfil/code/Fulfil.ComputerVision/
COPY Fulfil.DepthCam/ ./Fulfil.DepthCam/
COPY Fulfil.Dispense/ ./Fulfil.Dispense/
COPY Fulfil.MongoCpp/ ./Fulfil.MongoCpp/
COPY Fulfil.CPPUtils/ ./Fulfil.CPPUtils/
RUN mkdir Fulfil.AlliedVision
COPY Fulfil.AlliedVision/VimbaX_Setup-2024-1-Linux64.tar.gz ./Fulfil.AlliedVision/

RUN cd Fulfil.AlliedVision && tar -xvf VimbaX_Setup-2024-1-Linux64.tar.gz
RUN cp -r ./Fulfil.AlliedVision/VimbaX_2024-1 /home/fulfil

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
