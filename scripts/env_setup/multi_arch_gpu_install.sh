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
OS_NAME="$(lsb_release -a 2>/dev/null | grep Description | sed 's/Description:\s*//g')"
PROCESSOR="$(uname -p)"
PYTHON3_LOCATION="/usr"
PYTHON3_BIN="${PYTHON3_LOCATION}/bin/python3"
PYTHON_VERSION="$("${PYTHON3_BIN}" -V  | grep -Eo "[0-9.]*" )"
PYTHON3_LIB_LOC="lib/python$(echo "$PYTHON_VERSION" | cut -d . -f 1,2 )"


#PROTOBUF3_VERSION=21.12
PROTOBUF3_VERSION=3.19.6
BAZEL_VERSION=1.16.0
CMAKE_VERSION=3.23.5
GRPC_VERSION=1.48.2

EIGEN_VERSION=3.3.9
OPENCV_VERSION=4.6.0
LIBREALSENSE_VERSION=2.54.2
#LIBREALSENSE_VERSION=2.49.0
MONGOC_VERSION=1.21.1
MONGOCXX_VERSION=3.6.6
SPDLOG_VERSION=1.11.0


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
  UPDATE_CUDA_ARGS "7.2"
  DO_RS_USB_PATCH="OFF"
elif [[ "$PROCESSOR" == "x86_64" ]]; then
  echo -e "${LIM}Valid host architecture $PROCESSOR found!${NC}"
  PKG_MAKE_PREFIX="/usr/local"
  UPDATE_CUDA_ARGS "7.0"
  DO_RS_USB_PATCH="ON"
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

if (( $(echo "$(lsb_release -a 2>/dev/null | grep Release | sed 's/Release:\s*//g') < ${MINIMUM_RELEASE}" |bc -l) )); then
  echo -e "${RED}Dependency script expects Ubuntu 18+ as host OS. You may need to change your distro or use an older commit."
  echo -e "${RED}Exiting workspace setup!${NC}"
  exit 1
fi

echo -e "${LIM}Valid host operating system Ubuntu 18 found!${NC}"

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

cd "$PATH_TO_SCRIPT"

print_env_params
continue_or_skip || exit 0

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
    sudo apt-get update && sudo apt-get install -y apt-utils make build-essential autoconf automake libtool pkg-config \
      bc tmux git gitk vim wget rsync curl libcurl4-gnutls-dev  libsystemd-dev libnotify-dev zip unzip

    sudo apt update && sudo apt install -y python3 python3-dev python3-venv

    sudo apt update && sudo apt install -y libssl-dev libsasl2-dev libsnappy-dev libbsd-dev ca-certificates
    sudo apt update && sudo apt install -y libboost-dev libboost-all-dev

    sudo apt update && sudo apt install -y libopenblas-base libopenblas-dev libsuperlu-dev libsuitesparse-dev \
    liblapack-doc liblapack-dev liblapacke-dev libatlas3-base libatlas-base-dev gfortran

    #sudo apt install -y libhdf5-serial-dev hdf5-tools libhdf5-dev
    # I think this may have been causing some of the ld issues ^ along with the suite sparse?

    sudo apt install -y libusb-1.0-0-dev libudev-dev at libglfw3-dev libgtk-3-dev libgtk2.0-dev

    sudo apt-get install -y libglew-dev glew-utils libglvnd-dev qt5-default libgl1-mesa-dev libglu1-mesa-dev libgl1 freeglut3-dev

    # Uncertain if should install
    sudo apt update && sudo apt install -y \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libjpeg-dev \
        libjpeg8-dev \
        libjpeg-turbo8-dev \
        libpng-dev \
        libpostproc-dev \
        libswscale-dev \
        libtbb-dev \
        libtiff5-dev \
        libxvidcore-dev \
        libx264-dev \
        libv4l-dev v4l-utils \
        tesseract-ocr libtesseract-dev  libleptonica-dev \
        zlib1g-dev

    sudo apt purge -y libopencv libeigen3-dev #cmake cmake-data
    # NOTE: DO NOT REMOVE THE SYSTEM PROTO BUF! IT MESSES SHIT UP libprotobuf10 libprotobuf-lite10 python3-protobuf
fi
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
echo -e "\n${CYN}About to start $LIBRARY install. To skip this step choose not to proceed with the process...${NC}"
if continue_or_skip ; then
  echo -e "${PRP}Checking for correct $LIBRARY version in default system location...${NC}"
    pm_installed libopencv && sudo apt purge -y libopencv
  if [[ ! -f "$PKG_MAKE_PREFIX/bin/opencv_version" ]] || [[ "$($PKG_MAKE_PREFIX/bin/opencv_version)" < "$OPENCV_VERSION" ]] ; then
      echo -e "${BLU}No $LIBRARY version found in $DEFAULT_INSTALL_DIR.${NC}"
      sudo apt purge -y libopencv libeigen3-dev
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
      time cmake  -DCMAKE_BUILD_TYPE=RELEASE \
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
                  -DWITH_PROTOBUf=ON \
                  -DBUILD_FAT_JAVA_LIB=OFF \
                  -DBUILD_opencv_python2=OFF \
                  -DBUILD_opencv_python3=ON \
                  -DBUILD_opencv_python_tests=OFF \
                  -DWITH_EIGEN=ON \
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
                  -DCUDA_ARCH_BIN=${GPU_CUDA_ARCH_BIN}\
                  -DCUDA_ARCH_PTX="" \
                  -DCUDA_NVCC_FLAGS="$(get_cuda_flags)" \
                  -DOpenGL_GL_PREFERENCE=GLVND \
                  -DOPENCV_FORCE_PYTHON_LIBS=ON \
                  -DPYTHON3_EXECUTABLE=${PYTHON3_BIN} \
                  -DPYTHON_DEFAULT_EXECUTABLE=${PYTHON3_BIN} \
                  ..

      EXIT_LOG "Configure CMake for OpenCV" $?
      echo "Finished configuring OpenCV. Validate build parameters and proceed to make."
      continue_or_skip || exit 0
      run_make_routine "OpenCV ${OPENCV_VERSION}"
      echo "Finished building OpenCV source. Install pending..."
      continue_or_skip && run_install_routine "OpenCV ${OPENCV_VERSION}" || echo -e "${GRN}Chose to skip install to $DEFAULT_INSTALL_DIR. Proceeding to next step.\n${NC}"

  else
    echo -e "${GRN}Found CPP $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...\n${NC}"
  fi

  echo -e "Python cv2 source should be installed at default CXX location. \nDo you want to try updating installation to python lib location ${PYTHON3_LOCATION}/${PYTHON3_LIB_LOC}"
  if continue_or_skip ; then
      echo -e "${CYN}Searching for cv2 python package at default temporary lib location ${PKG_MAKE_PREFIX}${NC}"
      CV2_PY_PACKAGE="$(find "${PKG_MAKE_PREFIX}" -name "cv2")"
      [ "$(echo "${CV2_PY_PACKAGE}" | wc -l )" -ne 1 ] && echo -e "${RED}Found multiple installations of cv2 library!\n${CV2_PY_PACKAGE} clean up and reinstall!" && exit 1
      if [[ "$CV2_PY_PACKAGE" == "" ]] ; then
        echo -e "${RED} Unable to find the temp cv2 install! Validate that the c++ OpenCV4 lib properly installed in ${PKG_MAKE_PREFIX}.${NC}"
      else
          echo -e "${PRP}cv2 python package found at temp location ${CV2_PY_PACKAGE}${NC}"
          if [[ "${PYTHON3_LOCATION}" == "/usr" ]] ; then
            cv2_install_loc="/usr/local/${PYTHON3_LIB_LOC}/dist-packages"
            [ ! -d "$cv2_install_loc" ] && mkdir -p "$cv2_install_loc"
            [[ "${CV2_PY_PACKAGE}" != "$cv2_install_loc/cv2" ]] && echo echo -e "${PRP}Moving to new location $cv2_install_loc${NC}" \
              && sudo mv "${CV2_PY_PACKAGE}" "$cv2_install_loc"
          else
            echo -e "${BLU}Python3 is not set to system binary... you must hand install package to correct location.${NC}"
          fi
          echo -e "${BLU}After update, able to import ${LIM}cv2${BLU} version ${LIM}$(PYTHON_PACKAGE_VER cv2)${BLU} for ${PYTHON3_BIN}...${NC}"
          "$PYTHON3_BIN" -c "import cv2 ; print(f'Number of cv2 cuda enabled devices: {cv2.cuda.getCudaEnabledDeviceCount()}')"
          echo -e "${GRN}Updated python cv2 installation location.\n${NC}"
      fi
  else
      echo -e "${GRN}Chose to skip updating python cv2 installation location. Proceeding to next step.\n${NC}"
  fi

  echo -e "${YLW}OpenCV 4 installation complete!\n\n${NC}"
fi

LIBRARY="librealsense2"
cd "$WORKINGDIR" || exit
PKG_Installed=$(ls $DEFAULT_INSTALL_DIR | grep -E "$LIBRARY")
echo -e "${PRP}Checking for $LIBRARY in $DEFAULT_INSTALL_DIR...${NC}"
if [ "" = "$PKG_Installed" ] ; then
  # Realsense camera library
  echo -e "${BLU}No $LIBRARY version found.${NC}"

  if [ ! -d "librealsense" ] ; then
      echo -e "${CYN}Cloning $LIBRARY repo...${NC}"
      git clone https://github.com/IntelRealSense/librealsense.git
  fi
  cd librealsense || exit
  git checkout "v${LIBREALSENSE_VERSION}" || exit

  echo -e "${LIM}Setting up udev rules...${NC}"
  ./scripts/setup_udev_rules.sh
  EXIT_LOG "setup_udev_rules" $?

  if [[ "$DO_RS_USB_PATCH" == "ON" ]] && echo "The script is about to patch the kernel for USB camera control" \
    && continue_or_skip ; then
    echo -e "${LIM}Patching Ubuntu for Realsense libs.${NC}"
    ./scripts/patch-realsense-ubuntu-lts.sh
    EXIT_LOG "patch-realsense-ubuntu-lts" $?
  else
    echo -e "\n${PRP}Skip Realsense Patches was set! No updates to kernel will be made!${NC}\n"
  fi

  echo -e "${LIM}Installing $LIBRARY...${NC}"
  [ ! -d "build" ] && mkdir build
  cd build || exit
 
  # -DCMAKE_LIBRARY_PATH="/usr/lib;/usr/lib/aarch64-linux-gnu;/usr/local/lib" \
  time cmake -DCMAKE_BUILD_TYPE=RELEASE \
      -DCMAKE_INSTALL_PREFIX=$PKG_MAKE_PREFIX \
      -DBUILD_EXAMPLES:BOOL=ON \
      -DFORCE_RSUSB_BACKEND:BOOL=OFF \
      -DBUILD_WITH_OPENMP:BOOL=ON \
      -DOpenGL_GL_PREFERENCE=GLVND \
      -DBUILD_WITH_TM2:BOOL=OFF \
      -DBUILD_WITH_CUDA:BOOL=${USE_GPU} \
      -DCMAKE_CUDA_ARCHITECTURES=${CUDA_GENCODE} \
      -DCUDA_NVCC_FLAGS="$(get_cuda_flags)" \
      -DBUILD_PYTHON_BINDINGS:BOOL=ON \
      -DPYBIND11_INSTALL:BOOL=ON \
      -DPYTHON_EXECUTABLE=${PYTHON3_BIN} \
      ..
  EXIT_LOG "Configure CMake for Realsense" $?
  continue_or_skip || exit 0
  sudo make uninstall && make clean
  EXIT_LOG "Uninstall and make clean for Realsense" $?
  run_make_routine "Intel Realsense SDK"
  run_install_routine "Intel Realsense SDK"
else
  echo -e "${GRN}Found $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...${NC}"
fi
echo -e "${YLW}Realsense2 installation complete!${NC}"


#export CC="/usr/bin/gcc-$(get_latest_sys_gcc | cut -d . -f 1 )"
#export CXX="/usr/bin/g++-$(get_latest_sys_gcc | cut -d . -f 1 )"

LIBRARY="mongocxx"
cd "$WORKINGDIR" || exit
PKG_Installed=$(ls $DEFAULT_INSTALL_DIR | grep -E "$LIBRARY")
PKG_bsonxx=$(ls $DEFAULT_INSTALL_DIR | grep -E "bsoncxx")
PKG_mongoc=$(ls $DEFAULT_INSTALL_DIR | grep -E "libmongoc-1.0")
echo -e "${PRP}Checking for $LIBRARY in default system location...${NC}"
if [ "" = "$PKG_Installed" ] || [ "" = "$PKG_bsonxx" ] || [ "" = "$PKG_mongoc" ]; then

  # Need to first download the mongo-c library with bson
  echo -e "${BLU}No $LIBRARY version found in $DEFAULT_INSTALL_DIR.${NC}"
  echo -e "${PRP}Checking for updates to package list...${NC}"
  sudo apt update
  # Additional required libraries
  echo -e "${LIM}Installing dependencies for cpp Mongo driver library...${NC}"


  echo -e "${CYN}Downloading Mongo C driver (required for cpp driver)...${NC}"
  wget -c https://github.com/mongodb/mongo-c-driver/releases/download/${MONGOC_VERSION}/mongo-c-driver-${MONGOC_VERSION}.tar.gz -O - | tar -xz
  cd mongo-c-driver-${MONGOC_VERSION} || exit
  echo -e "${LIM}Installing Mongo C Driver...${NC}"
  [ ! -d "cmake-build" ] && mkdir cmake-build
  cd cmake-build || exit
  cmake .. \
    -D ENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF \
    -D CMAKE_INSTALL_PREFIX="$PKG_MAKE_PREFIX"
  EXIT_LOG "Configure CMake for MongoC" $?
  continue_or_skip || exit

  run_make_routine "MongoC"
  run_install_routine "MongoC"
  cd "$WORKINGDIR" || exit
  echo -e "${YLW}Mongo C driver installation complete!${NC}"
  echo -e "${CYN}Downloading Mongo cpp driver...${NC}"
  curl -OL "https://github.com/mongodb/mongo-cxx-driver/archive/r${MONGOCXX_VERSION}.tar.gz"
  tar -xzf "r${MONGOCXX_VERSION}.tar.gz"
  echo -e "${LIM}Installing Mongo cpp Driver...${NC}"
  cd "mongo-cxx-driver-r${MONGOCXX_VERSION}" || exit
  [ ! -d "build" ] && mkdir build
  cd build || exit
  cmake ..                                \
    -DCMAKE_BUILD_TYPE=Release          \
    -DBUILD_SHARED_AND_STATIC_LIBS=ON   \
    -DBUILD_VERSION="${MONGOCXX_VERSION}"              \
    -DBSONCXX_POLY_USE_MNMLSTC=1        \
    -DCMAKE_PREFIX_PATH="$PKG_MAKE_PREFIX"     \
    -DCMAKE_INSTALL_PREFIX=$PKG_MAKE_PREFIX
  EXIT_LOG "Configure CMake for MongoCXX" $?
  continue_or_skip || exit

  sudo make EP_mnmlstc_core
  EXIT_LOG "Make EP_mnmlstc_core" $?
  run_make_routine "MongoCXX"
  run_install_routine "MongoCXX"
else
  echo -e "${GRN}Found $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...${NC}"
fi
echo -e "${YLW}Mongo cpp driver installation complete!\n\n${NC}"

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
