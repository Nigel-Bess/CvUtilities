#!/bin/bash

# Exit on command fails 
#set -e

# Colors of the rainbow...
RED='\033[0;31m'
GRN='\033[0;32m'
LIM='\033[1;32m'
CYN='\033[0;36m'
YLW='\033[1;33m'
BLU='\033[1;34m'
PRP='\033[1;35m'
NC='\033[0m' # No Color
PATH=/usr/local/cuda-11/bin:${PATH} # make sure cuda is in path
function continue_or_skip {
  while true ; do
    read -p "Do you want to continue? (y/n) " yn
    case $yn in
        [yY] ) echo -e "Continue process..." ; return 0 ;;
        [nN] ) echo -e "Do not proceed with process..." ; return 1 ;;
        * ) echo "Please answer yes or no. ";;
    esac
  done
}

function print_env_params {
  echo -e "${NC}Current environment/build params on host:${NC}"
  echo -e "${NC}-- Operating System:  ${BLU}${OS_NAME}${NC}"
  echo -e "${NC}-- Processor Type:    ${BLU}${PROCESSOR}${NC}"
  echo -e "${NC}-- GPU Architecture:  ${BLU}${GPU_CUDA_ARCH_BIN}${NC}"
  echo -e "${NC}-- Cuda Gencode:      ${BLU}${CUDA_GENCODE}${NC}"
  echo -e "${NC}-- Build with Cuda:   ${BLU}${USE_GPU}${NC}"
  echo -e "${NC}-- Install Prefix:    ${BLU}${PKG_MAKE_PREFIX}${NC}"
  echo -e "${NC}-- Install Directory: ${BLU}${DEFAULT_INSTALL_DIR}${NC}"
  echo -e "${NC}-- Python Binary:     ${BLU}${PYTHON3_BIN}${NC}"
  echo -e "${NC}-- Python Version:    ${BLU}${PYTHON_VERSION}${NC}"
  echo -e "${NC}-- Python Libraries:  ${BLU}${PYTHON3_LIB_LOC}${NC}"
  echo -e "${NC}-- Num Build Threads: ${BLU}${NUM_CPU}${NC}"
  echo
  echo -e "${NC}-- Eigen3 Version:    ${YLW}${EIGEN_VERSION}${NC}"
  echo -e "${NC}-- OpenCV Version:    ${YLW}${OPENCV_VERSION}${NC}"
  echo -e "${NC}-- Realsense Version: ${YLW}${LIBREALSENSE_VERSION}${NC}"
  echo -e "${NC}-- MongoC Version:    ${YLW}${MONGOC_VERSION}${NC}"
  echo -e "${NC}-- MongoCXX Version:  ${YLW}${MONGOCXX_VERSION}${NC}"
  echo -e "${NC}-- SpdLog Version:    ${YLW}${SPDLOG_VERSION}${NC}"


}

# set up and compile the cpp libraries necessary to run dispense
# This should install everything required for Dispense/DC code on a laptop or PC or Jetson.
# Currently only tested against machine with GPU acceleration.
# Though should still run on machines with out GPUs

NUM_CPU=12
MINIMUM_RELEASE=18.04
OS_NAME="$(lsb_release -ds 2>/dev/null)"
PROCESSOR="$(uname -p)"
PYTHON3_LOCATION="/usr"
PYTHON3_BIN="${PYTHON3_LOCATION}/bin/python3"
PYTHON_VERSION="$("${PYTHON3_BIN}" -V  | grep -Eo "[0-9.]*" )"
PYTHON3_LIB_LOC="lib/python$(echo "$PYTHON_VERSION" | cut -d . -f 1,2 )"


CMAKE_VERSION=3.25.2

EIGEN_VERSION=3.3.9
OPENCV_VERSION=4.6.0
LIBREALSENSE_VERSION=2.54.2
MONGOC_VERSION=1.21.1
MONGOCXX_VERSION=3.6.6
SPDLOG_VERSION=1.11.0
# Architecture to gencode ref https://arnon.dk/matching-sm-architectures-arch-and-gencode-for-various-nvidia-cards/
JETSON_CUDA_ARCH="7.2" # 7.2 for Xavier 8.7 for Orin
POSSIBLE_HOST_CUDA_ARCH="7.0" # This was specific to ambers x86 machine.
# # If host computer has a gpu, update above. if it does not, the script detects that  and skips GPU specific steps


function UPDATE_CUDA_ARGS () {
  if nvcc --version &>/dev/null ; then
    echo -e "\n${LIM}Found cuda capable device, will attempt to install with GPU functionality.${NC}\n"
    USE_GPU="ON"
    GPU_CUDA_ARCH_BIN=${1}
    CUDA_GENCODE=${GPU_CUDA_ARCH_BIN//\./}
  else
    echo -e "${BLU}Failed to find cuda capable device, GPU functionality ${RED}will not${BLU} be installed.${NC}"
    USE_GPU="OFF"
    GPU_CUDA_ARCH_BIN=
    CUDA_GENCODE=
  fi
}

function get_cuda_flags { [[ $USE_GPU == "ON" ]] && \
  echo "--expt-relaxed-constexpr --expt-extended-lambda  -arch=sm_${CUDA_GENCODE} -gencode arch=compute_${CUDA_GENCODE},code=sm_${CUDA_GENCODE}" ; }

if [[ "$PROCESSOR" == "aarch64" ]]; then
  echo -e "${LIM}Valid host architecture $PROCESSOR found!${NC}"
    if uname -r grep tegra &>/dev/null && ! nvcc --version &>/dev/null ; then
      echo -e "${RED}Script is currently running on a Jetson that is not cuda capable! You must install drivers before proceeding. ${NC}"
      echo -e "${RED}Exiting workspace setup!${NC}"
      exit 1
    fi
  PKG_MAKE_PREFIX="/usr"
  UPDATE_CUDA_ARGS "$JETSON_CUDA_ARCH"
  DO_RS_USB_PATCH="OFF"
elif [[ "$PROCESSOR" == "x86_64" ]]; then
  echo -e "${LIM}Valid host architecture $PROCESSOR found!${NC}"
  PKG_MAKE_PREFIX="/usr/local"
  UPDATE_CUDA_ARGS "7.0"
  # You may want to comment this out if running a container / wsl on windows host
  DO_RS_USB_PATCH="ON" # Unsure how the usb patch will work using WSL or docker since this is a kernel level change
else
  echo -e "${RED}Unknown host architecture detected!${NC}"
  echo -e "${RED}Exiting workspace setup!${NC}"
  exit 1
fi
DEFAULT_INSTALL_DIR="$PKG_MAKE_PREFIX/include"


if [ $NUM_CPU -gt "$(( $(nproc) -1 ))" ] ; then
    NUM_CPU="$(( $(nproc) -1 ))"
    echo -e "${RED}ERROR: Too many cores chosen for compilation! Aim for at most 1 less than total, or 4 if using hyper threading to avoid overheating errors!${NC}"
    echo -e "${CYN}Setting number of build threads to $NUM_CPU${NC}"
fi

# TODO add GPU support
echo -e "********************************************"
echo -e "*** Workspace setup for Fulfil.Dispense ***"
echo -e "********************************************"

if (( $(echo "$(lsb_release -rs 2>/dev/null) < ${MINIMUM_RELEASE}" |bc -l) )); then
  echo -e "${RED}Dependency script expects Ubuntu 18+ as host OS. You may need to change your distro or use an older commit."
  echo -e "${RED}Exiting workspace setup!${NC}"
  exit 1
fi

echo -e "${LIM}Valid host operating system Ubuntu 18+ found!${NC}"

function PROC_ARCH_ALIAS { if [[ "$(uname -p )" == "aarch64" ]] ;  then echo "arm64"
  elif [[ "$(uname -p )" == "x86_64" ]] ; then echo  "amd64" ; else uname -p ; fi }

function get_latest_sys_gcc() {
  local VERSION="[15-9][0-9]*.[0-9]*"
  dpkg-query -W --showformat='${Package} ${Status} version ${Version}\n' | grep g++ | grep installed | grep -Eo "version $VERSION" | grep -o "[0-9.]*" | sort -n -r | head -n 1
}

function pm_installed () { dpkg -l "${1:-empty_string_is_false}" &>/dev/null ;  }

function PYTHON_PACKAGE_VER { "$PYTHON3_BIN"  -c "import ${1} ; print(${1}.__version__)" ; }

function script_directory()
  {
      f=${0%/*}
      if [ -d "$f" ]; then
          dir="$f"
      else
          dir=$(dirname "$f")
      fi
      dir=$(cd "$dir" && /bin/pwd)
      echo "$dir/"
  }

PATH_TO_SCRIPT=$( script_directory )
echo -e "${PRP}Dependencies will be installed at${BLU} ${PKG_MAKE_PREFIX} ${NC}${PRP}by script. If CMake install location is changed${BLU} Fulfil.Dispense ${NC}${PRP}may not compile.${NC}"

cd "$PATH_TO_SCRIPT" || exit 1

print_env_params
continue_or_skip || exit 0

echo -e "${PRP}Current state of disk:\n\n${NC}$(df -h)\n"

[ ! -d "depthcam_dependencies" ] &&  mkdir depthcam_dependencies
cd depthcam_dependencies && echo -e "${YLW}Moving into temporary build directory${BLU} $(pwd) ${NC}${YLW}for build!${NC}"
echo -e "${CYN}NOTE: Machines with minimal memory may need to clear build folders after successful installations to conserve space during script execution.\n${NC}"

WORKINGDIR="$(pwd)"


echo -e "${PRP}Running prelim upgrade...${NC}"
#sudo apt update && sudo apt upgrade

#source /home/amber/.virtualenvs/tray-ml/bin/activate

if [ ! -x "$PYTHON3_BIN" ] && [ "$PYTHON_VERSION" == "" ]; then
    echo -e "${RED}Python binary does not exist at $PYTHON3_BIN! You may need to create a venv at this location or update path!${NC}"
    exit 1
else
    echo -e "${GRN}Found $PYTHON_VERSION installed in ${PYTHON3_LOCATION}. Moving on with installs...${NC}"
fi

# Will not work for protobuffer package

# ----------------------

echo -e "${BLU}To speed up builds will attempt to use $NUM_CPU cores out of $(nproc). For best results cancel resource intense processes.${NC}"

EXIT_LOG() {
  local cmd=${1}
  local cmd_status=${2}
  if [ $cmd_status -ne 0 ]; then
      echo -e "${RED}Command ( ${CYN}${cmd}${RED} ) failed with exit code ${2}!${NC}\n" >&2
      exit $cmd_status;
  else
      echo -e "${LIM}Command ( ${CYN}${cmd}${LIM} ) returned  with exit code ${2}! Success! ${NC}\n"
  fi
  return 0
}

run_make_routine() {
  time make -j $NUM_CPU
  if [ $? -eq 0 ] ; then
      echo -e "${BLU}${1} make successful${NC}"
  else
      # Try to make again; Sometimes there are issues with the build
      # because of lack of resources or concurrency issues
      echo -e "${RED}${1} make did not build ${NC}" >&2
      echo -e "${LIM}Retrying on single thread... ${NC}"
      # Single thread this time
      time make
      if [ $? -eq 0 ] ; then
          echo -e "${BLU}${1} make successful${NC}"
      else
          # Try to make again
          echo -e "${RED}${1} make did not successfully build${NC}" >&2
          echo -e "${RED}Please fix issues with environment for $1 and retry build${NC}"
          exit 1
      fi
  fi
}

run_install_routine(){
  local install_route="${2:-$PKG_MAKE_PREFIX}"
  local job_reduction=2
  local install_j=$( [ $NUM_CPU -gt $job_reduction ] && echo $(( $NUM_CPU - $job_reduction )) || echo 1 )
  time sudo make install -j $install_j
  if [ $? -eq 0 ] ; then
     echo -e "${BLU}${1} installed in: $install_route${NC}"
  else
    if [ $install_j -gt 1 ] ; then
        echo -e "${RED}${1} multi-job ($install_j) make install failed ${NC}" >&2
        echo -e "${LIM}Retrying on single thread... ${NC}"
        time sudo make install
        if [ $? -eq 0 ] ; then
          echo -e "${BLU}${1} installed in: $install_route${NC}"
          return 0
        fi
    fi
    echo -e "${RED}There was an issue installing ${1} to $install_route!${NC}"
    exit 1
  fi
}

echo -e "\n${CYN}Do you want to install all apt dependencies right now?${NC}"
if continue_or_skip ; then


   echo -e "\n${BLU}Installing build and general dev utilities${NC}\n"
   sudo apt-get update && sudo apt-get install -y \
    apt-transport-https apt-utils make build-essential autoconf automake libtool pkg-config \
      bc tmux git gitk vim wget rsync curl libcurl4-gnutls-dev ca-certificates gpg libsystemd-dev libnotify-dev zip unzip

   echo -e "\n${BLU}Installing python reqs...${NC}\n"

    sudo apt-get update && sudo apt-get install -y python3-pip python3-dev python3-venv
   echo -e "\n${BLU}Installing math libraries for use with eigen/opencv/realsense...${NC}\n"

    # For eigen back end. Uncertain if I should install blas on jetson, since native version present
    sudo apt update && sudo apt install -y \
      libblas-dev \
      libatlas3-base libatlas-base-dev \
      libsuperlu5 libsuperlu-dev \
      libumfpack5 libspqr2 libldl2 libcholmod3 \
      libsuitesparseconfig5 libsuitesparse-dev \
      liblapack-doc liblapack-dev liblapacke liblapacke-dev

   echo -e "\n${BLU}Installing TF and OpenCV helpers...${NC}\n"
    # Python Tensorflow installs
    sudo apt-get -y update && sudo apt-get install -y libhdf5-serial-dev hdf5-tools libhdf5-dev zlib1g-dev \
	     zip libjpeg8-dev liblapack-dev libblas-dev

    # realsense / opencv
   echo -e "\n${BLU}Installing Realsense and OpenCV helpers...${NC}\n"
    sudo apt update && sudo apt install -y libssl-dev libsasl2-dev libsnappy-dev libbsd-dev ca-certificates
    sudo apt update && sudo apt install -y libboost-dev #libboost-all-dev
    sudo apt install -y libusb-1.0-0-dev libudev-dev at libglfw3-dev libgtk-3-dev libgtk2.0-dev
    sudo apt-get install -y libglew-dev glew-utils libglvnd-dev qt5-default libgl1-mesa-dev libglu1-mesa-dev libgl1 freeglut3-dev

    # Uncertain if should install, these are on jetson already (and we don't need tesseract at this point)
    # CV data format headers
   echo -e "\n${BLU}Installing OpenCV helpers...${NC}\n"
   sudo apt update && sudo apt install -y \
       libjpeg-dev \
       libjpeg8-dev \
       libjpeg-turbo8-dev \
       libpng-dev \
       libswscale-dev \
       libtbb-dev \
       zlib1g-dev

    sudo apt update && sudo apt install -y \
        tesseract-ocr libtesseract-dev  libleptonica-dev \
        libavdevice-dev libavdevice58 \
        libpostproc-dev \
        libtiff5-dev \
        libxvidcore-dev \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libx264-dev

    # additional video driver support
   echo -e "\n${BLU}Installing additional video driver support...${NC}\n"
    sudo apt install -y libv4l-dev v4l-utils \
      v4l2loopback-dkms v4l2loopback-utils\
      uvcdynctrl ffmpeg


    # NOTE: DO NOT REMOVE THE SYSTEM PROTO BUF! IT MESSES SHIT UP libprotobuf10 libprotobuf-lite10 python3-protobuf
fi

REQUIRED_PKG="cmake"
SYS_CMAKE_VERSION="$(cmake --version 2>/dev/null | grep -o "[3-9].[0-9.]*")"
echo -e "${PRP}Checking $REQUIRED_PKG for version $CMAKE_VERSION or higher package installation...${NC}"

kitware_keyring_install () {
    echo -e "\n${BLU}Creating kitware keyring${NC}"
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs ) main" | sudo tee /etc/apt/sources.list.d/kitware.list  >/dev/null
    sudo apt-get -y update
    test -f /usr/share/doc/kitware-archive-keyring/copyright || sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
    sudo apt-get install -y kitware-archive-keyring
    sudo apt-get -y update
}

if [[ "$SYS_CMAKE_VERSION" == "$(echo -e "$CMAKE_VERSION\n$SYS_CMAKE_VERSION" | sort --version-sort | tail -n1 )" ]] ; then
   echo -e "${GRN}System cmake '$SYS_CMAKE_VERSION' meets required version $CMAKE_VERSION. Skipping install...${NC}"
else
   echo -e "${BLU}System does not meet the minimum required CMake version!"
   echo -e "${BLU}Need to update cmake from '$SYS_CMAKE_VERSION' to $CMAKE_VERSION!${NC}"
   sudo apt-get remove -y cmake cmake-data
   kitware_keyring_install
    cmake_apt_version="$(apt list --all-versions cmake 2>/dev/null \
      | grep -Eo "$(lsb_release -cs).*${CMAKE_VERSION}.*$(lsb_release -rs)[.0-9]* $(PROC_ARCH_ALIAS)" \
      | grep -Eo "${CMAKE_VERSION}[^ ]*$(lsb_release -rs)[.0-9]*" )"
    echo -e "\n${LIM}Installing cmake*=${cmake_apt_version}${NC}\n"
   sudo apt-get install "cmake-data=${cmake_apt_version}"
   sudo apt-get install "cmake=${cmake_apt_version}"
   EXIT_LOG "CMake version ${CMAKE_VERSION} installation failed" $?
fi


 cmake --version &> /dev/null || echo -e "${RED}ERROR! ${BLU}No system cmake found!"

# In case parent environment set a different CMAKE compiler.
# For GPU builds, we need to use the default compiler that pairs with cuda installation
#unset CC
#unset CXX

LIBRARY="eigen3"
cd "$WORKINGDIR" || exit
EIGEN_VERSION_FILE="$DEFAULT_INSTALL_DIR/eigen3/Eigen/src/Core/util/Macros.h"
[ -f "$EIGEN_VERSION_FILE" ] && echo "Found version file!"
function EIGEN_VER_PART { [ -f "$EIGEN_VERSION_FILE" ] && grep "#define EIGEN_${1}_VERSION" "$EIGEN_VERSION_FILE" 2>/dev/null | grep -Eo "[0-9]" 2>/dev/null ; }

function GET_SYS_EIGEN_VER {
        local eigen_version="$( EIGEN_VER_PART WORLD ).$( EIGEN_VER_PART MAJOR ).$( EIGEN_VER_PART MINOR )"
        [ "$eigen_version" != '..' ] && echo "$eigen_version" || echo 'NOT_FOUND'
}

INSTALLED_EIGEN_VER="$(GET_SYS_EIGEN_VER)"
echo -e "${PRP}Current eigen verison (${INSTALLED_EIGEN_VER}) ${NC}"

if [[ "$EIGEN_VERSION" != "$INSTALLED_EIGEN_VER" ]]; then
   echo -e "${BLU}No $LIBRARY version ${PRP}$EIGEN_VERSION${BLU} found in $DEFAULT_INSTALL_DIR...${NC}"
   [ -d "$DEFAULT_INSTALL_DIR/eigen3" ] && echo  -e "${BLU}Removing old eigen ${PRP}${INSTALLED_EIGEN_VER}${BLU} version install at $DEFAULT_INSTALL_DIR/eigen3${NC}" \
            && sudo rm -rf $DEFAULT_INSTALL_DIR/eigen3


  sudo apt purge -y libeigen3-dev
  echo -e "${BLU}No $LIBRARY found in $DEFAULT_INSTALL_DIR...${NC}"
  echo -e "${CYN}Cloning $LIBRARY repo...${NC}"
  [ ! -d "eigen" ] && git clone https://gitlab.com/libeigen/eigen.git
  cd eigen  && git checkout ${EIGEN_VERSION} || exit
  echo "${LIM}Installing $LIBRARY...${NC}"
  [ ! -d "build" ] && mkdir build
  cd build
  cmake -DCMAKE_BUILD_TYPE=RELEASE \
  -DCMAKE_INSTALL_PREFIX=$PKG_MAKE_PREFIX \
  -DOpenGL_GL_PREFERENCE=GLVND \
  -DBUILD_WITH_OPENMP:BOOL=ON \
  -DEIGEN_CUDA_COMPUTE_ARCH=${CUDA_GENCODE} ..
  EXIT_LOG "Configure CMake for Eigen3" $?
  echo "Finished configuring Eigen3. Validate build parameters and proceed to make and install."

  continue_or_skip || exit 0

  run_make_routine "Eigen3"
  run_install_routine "Eigen3"
else
  echo -e "${GRN}Found $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...${NC}"
fi
echo -e "${YLW}Eigen3 installation complete!${NC}"




LIBRARY="opencv4"
DOWNLOAD_OPENCV_EXTRAS=YES
cd "$WORKINGDIR" || exit
echo -e "\n${CYN}About to start $LIBRARY install. Desired min version is $OPENCV_VERSION, found version '$($PKG_MAKE_PREFIX/bin/opencv_version 2>/dev/null || echo NOTFOUND)'."
echo -e "To skip this step choose not to proceed with the process...${NC}"
if continue_or_skip ; then
  echo -e "${PRP}Checking for correct $LIBRARY version in default system location...${NC}"
  if [[ ! -f "$PKG_MAKE_PREFIX/bin/opencv_version" ]] || [[ "$($PKG_MAKE_PREFIX/bin/opencv_version)" < "$OPENCV_VERSION" ]] ; then
      echo -e "${BLU}No $LIBRARY version found in $DEFAULT_INSTALL_DIR.${NC}"
      pm_installed libopencv && sudo apt purge -y libopencv*

      #sudo apt purge -y libopencv
      # OpenCV
      # Source code directory
      [ ! -d "opencv_source" ] && mkdir opencv_source
      OPENCV_SOURCE_DIR="$WORKINGDIR/opencv_source"
      cd "$OPENCV_SOURCE_DIR" || exit

      echo -e "${LIM}Installing dependencies for $LIBRARY library...${NC}"

      echo -e "${CYN}Cloning $LIBRARY repo...${NC}"
      [ ! -d "opencv" ] && git clone https://github.com/opencv/opencv.git
      cd opencv
      git checkout $OPENCV_VERSION || exit
      cd ..

      # Contrib libraries
      echo -e "${CYN}Cloning $LIBRARY contrib repo...${NC}"
      [ ! -d "opencv_contrib" ] && git clone https://github.com/opencv/opencv_contrib.git
      cd opencv_contrib
      git checkout $OPENCV_VERSION || exit
      cd ..

      cd opencv || exit
      [ ! -d "build" ] && mkdir build
      cd build || exit

      echo -e "${LIM}Installing $LIBRARY with contrib modules linked to ${DEFAULT_INSTALL_DIR}/eigen3...${NC}"
      # -DENABLE_LTO=ON \  # `fatbinData' is already defined with cuda compile
      time cmake  -LH -DCMAKE_BUILD_TYPE=RELEASE \
                  -DCMAKE_INSTALL_PREFIX=$PKG_MAKE_PREFIX \
                  -DENABLE_CXX11=ON \
                  -DENABLE_CXX14=ON \
                  -DOPENCV_GENERATE_PKGCONFIG=ON \
                  -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
                  -DEIGEN_INCLUDE_PATH=${DEFAULT_INSTALL_DIR}/eigen3 \
                  -DBUILD_TESTS=OFF \
                  -DBUILD_PERF_TESTS=OFF \
                  -DBUILD_EXAMPLES=OFF \
                  -DBUILD_FAT_JAVA_LIB=OFF \
                  -DBUILD_JAVA=OFF \
                  -DBUILD_opencv_bioinspired=OFF \
                  -DBUILD_opencv_cvv=OFF \
                  -DBUILD_opencv_datasets=OFF \
                  -DBUILD_opencv_dnn_superres=OFF \
                  -DBUILD_opencv_dnn_objdetect=OFF \
                  -DBUILD_opencv_dpm=OFF \
                  -DBUILD_opencv_face=OFF \
                  -DBUILD_opencv_fuzzy=OFF \
                  -DBUILD_opencv_intensity_transform=OFF \
                  -DBUILD_opencv_julia=OFF \
                  -DBUILD_opencv_mcc=OFF \
                  -DBUILD_opencv_ovis=OFF \
                  -DBUILD_opencv_phase_unwrapping=OFF \
                  -DBUILD_opencv_quality=OFF \
                  -DBUILD_opencv_saliency=OFF \
                  -DBUILD_opencv_sfm=OFF \
                  -DBUILD_opencv_stereo=OFF \
                  -DBUILD_opencv_structured_light=OFF \
                  -DBUILD_opencv_superres=OFF \
                  -DBUILD_opencv_surface_matching=OFF \
                  -DBUILD_opencv_tracking=OFF \
                  -DBUILD_opencv_videostab=OFF \
                  -DBUILD_opencv_viz=OFF \
                  -DBUILD_opencv_xobjdetect=OFF \
                  -DBUILD_opencv_xphoto=OFF \
                  -DWITH_PROTOBUf=ON \
                  -DBUILD_FAT_JAVA_LIB=OFF \
                  -DBUILD_opencv_python2=OFF \
                  -DBUILD_opencv_python3=ON \
                  -DBUILD_opencv_python_tests=OFF \
                  -DWITH_EIGEN=ON \
                  -DWITH_LIBREALSENSE=ON \
                  -DWITH_FFMPEG=ON \
                  -DWITH_LIBV4L=ON \
                  -DWITH_GSTREAMER=ON \
                  -DWITH_GSTREAMER_0_10=OFF \
                  -DWITH_QT=ON \
                  -DWITH_OPENGL=ON \
                  -DDBUILD_WITH_OPENMP=ON \
                  -DWITH_TBB=ON \
                  -DWITH_LAPACK=ON \
                  -DWITH_TESSERACT=ON \
                  -DOPENCV_DNN_CUDA=${USE_GPU} \
                  -DWITH_CUDA=${USE_GPU} \
                  -DWITH_CUBLAS=${USE_GPU} \
                  -DWITH_CUDNN=${USE_GPU} \
                  -DWITH_CUFFT=${USE_GPU} \
                  -DCUDA_ARCH_BIN=${GPU_CUDA_ARCH_BIN} \
                  -DCUDA_ARCH_PTX="" \
                  -DCUDA_NVCC_FLAGS="$(get_cuda_flags)" \
                  -DOpenGL_GL_PREFERENCE=GLVND \
                  -DOPENCV_FORCE_PYTHON_LIBS=ON \
                  -DPYTHON3_EXECUTABLE=${PYTHON3_BIN} \
                  -DPYTHON_DEFAULT_EXECUTABLE=${PYTHON3_BIN} \
                  ..

      EXIT_LOG "Configure CMake for OpenCV" $?
      echo "Finished configuring OpenCV. Validate build parameters and proceed to make."
      if continue_or_skip ; then
        run_make_routine "OpenCV ${OPENCV_VERSION}"
        echo "Finished building OpenCV source. Install pending..."
        continue_or_skip && run_install_routine "OpenCV ${OPENCV_VERSION}" || echo -e "${GRN}Chose to skip install to $DEFAULT_INSTALL_DIR. Proceeding to next step.\n${NC}"
      fi
  else
    echo -e "${GRN}Found CPP $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...\n${NC}"
  fi

  echo -e "${CYN}Searching for cv2 python package at default temporary lib location ${PKG_MAKE_PREFIX}${NC}"
  CV2_PY_PACKAGE="$(find "${PKG_MAKE_PREFIX}" -name "cv2")"
  [[ "$CV2_PY_PACKAGE" == "" ]] && echo -e "${RED} Unable to find the temp cv2 install! Validate that the c++ OpenCV4 lib properly installed in ${PKG_MAKE_PREFIX}.${NC}" || echo -e "Python cv2 source should be installed at default CXX location. \nDo you want to try updating installation to python lib location ${PYTHON3_LOCATION}/${PYTHON3_LIB_LOC}"

  if [[ "$CV2_PY_PACKAGE" != "" ]] && continue_or_skip ; then
      [ "$(echo "${CV2_PY_PACKAGE}" | wc -l )" -ne 1 ] && echo -e "${RED}Found multiple installations of cv2 library!\n${CV2_PY_PACKAGE} clean up and reinstall!" && exit 1
      if "$PYTHON3_BIN" -c "import cv2" &>/dev/null ; then
          echo -e "${GRN}Default python installation location was successful! No move required. \n${NC}"
          cv2_install_loc="$CV2_PY_PACKAGE"
      else
          echo -e "${PRP}cv2 python package found at temp location ${CV2_PY_PACKAGE}${NC}"
          if [[ "${PYTHON3_LOCATION}" == "/usr" ]] ; then
            cv2_install_loc="/usr/local/${PYTHON3_LIB_LOC}/dist-packages"
            [ ! -d "$cv2_install_loc" ] && sudo mkdir -p "$cv2_install_loc"
            [[ "${CV2_PY_PACKAGE}" != "$cv2_install_loc/cv2" ]] && echo echo -e "${PRP}Moving to new location $cv2_install_loc${NC}" \
              && sudo mv "${CV2_PY_PACKAGE}" "$cv2_install_loc"
          else
            echo -e "${BLU}Python3 is not set to system binary... you must hand install package to correct location.${NC}"
          fi
          echo -e "${GRN}Updated python cv2 installation location.\n${NC}"
      fi
      echo -e "${BLU}Testing ${LIM}cv2${BLU} version ${LIM}$(PYTHON_PACKAGE_VER cv2)${BLU} importing for ${PYTHON3_BIN}...${NC}"
      "$PYTHON3_BIN" -c "import cv2 ; print(f'Build information for python cv2:\n{cv2.getBuildInformation()}')"
      "$PYTHON3_BIN" -c "import cv2 ; print(f'Number of cv2 cuda enabled devices: {cv2.cuda.getCudaEnabledDeviceCount()}')" && \
	      echo -e "${GRN}Successfully installed cv2 for ${LIM}${PYTHON_BIN}\n${NC}" ||  \
	      echo -e "${RED}Failed installing valid cv2 for\n  ${LIM}${PYTHON_BIN}${RED}\nat location $cv2_install_loc\n${NC}"
  else
      echo -e "${GRN}Chose to skip updating python cv2 installation location. Proceeding to next step.\n${NC}"
  fi

  echo -e "${YLW}OpenCV 4 installation complete!\n\n${NC}"
fi

LIBRARY="spdlog"
cd "$WORKINGDIR" || exit
echo -e "${PRP}Checking for $LIBRARY in $DEFAULT_INSTALL_DIR...${NC}"
PKG_Installed=$(ls $DEFAULT_INSTALL_DIR | grep -E "$LIBRARY")
if [ "" = "$PKG_Installed" ]; then
  echo -e "${BLU}No $LIBRARY version found in $DEFAULT_INSTALL_DIR...${NC}"
  echo -e "${CYN}Cloning $LIBRARY repo...${NC}"
  [ ! -d "spdlog" ] && git clone https://github.com/gabime/spdlog.git
  cd spdlog || exit
  git checkout "v${SPDLOG_VERSION}" || exit
  echo -e "${LIM}Installing $LIBRARY...${NC}"
  [ ! -d "build" ] && mkdir build
  cd build || exit
  cmake -DCMAKE_INSTALL_PREFIX="$PKG_MAKE_PREFIX" ..
  EXIT_LOG "Configure CMake for Spdlog" $?
  continue_or_skip || exit

  run_make_routine "Spdlog"
  run_install_routine "Spdlog"
  cd "$WORKINGDIR" || exit

else
  echo -e "${GRN}Found $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...${NC}"
fi
echo -e "${YLW}Logging backend installation complete!${NC}"

#unset CC
#unset CXX

echo -e "${PRP}Congrats! You have completed the installations required for the ${LIM}Fulfil.Dispense${PRP} project.${NC}"
echo -e "${RED}Test the installations by running dispense code. Once you are sure that the dependencies are properly installed, feel free to remove the ${CYN}depthcam_dependencies${RED} found at ${CYN}$(pwd)${RED}.${NC}"
echo -e "${NC}Restarting device is recommended.${NC}"
