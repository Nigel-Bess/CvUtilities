#!/bin/bash
# Colors of the rainbow...
RED='\033[0;31m'
GRN='\033[0;32m'
LIM='\033[1;32m'
CYN='\033[0;36m'
YLW='\033[1;33m'
BLU='\033[1;34m'
PRP='\033[1;35m'
NC='\033[0m' # No Color

#WARNING PLEASE ADJUST INSTALL LOCATION TO NEEDS. NUCS & PC = /usr/local, Jetsons -> /usr
PKG_MAKE_PREFIX="/usr"
DEFAULT_INSTALL_DIR="$PKG_MAKE_PREFIX/include"
cd ..
[ ! -d "cpp_dependencies" ] &&  mkdir cpp_dependencies
cd cpp_dependencies && echo "Made temporary directory for dependency $(pwd)"
echo -e "${BLU}Moved into temporary directory $(pwd)${NC}"
WORKINGDIR="$(pwd)"

NUM_CPU=$(nproc)
echo -e "${BLU}To speed up builds will attempt to use $(($NUM_CPU - 1)) cores out of $NUM_CPU. For best results cancel resource intense processes.${NC}"

run_make_routine() {
  time make -j$(($NUM_CPU - 1))
  if [ $? -eq 0 ] ; then
    echo -e "${BLU}$1 make successful${NC}"
  else
    # Try to make again; Sometimes there are issues with the build
    # because of lack of resources or concurrency issues
    echo -e "${RED}Make did not build ${NC}" >&2
    echo -e "${LIM}Retrying on single thread... ${NC}"
    # Single thread this time
    make
    if [ $? -eq 0 ] ; then
      echo -e "${BLU}$1 make successful${NC}"
    else
     # Try to make again
      echo -e "${RED}Make did not successfully build${NC}" >&2
      echo -e "${RED}Please fix issues with environment for $1 and retry build${NC}"
      exit 1
    fi
  fi
}

run_install_routine(){
  time sudo make install
  if [ $? -eq 0 ] ; then
     echo -e "${BLU}Mongo C driver installed in: $CMAKE_INSTALL_PREFIX${NC}"
  else
     echo -e "${RED}There was an issue with the final installation!${NC}"
     exit 1
  fi
}

LIBRARY="mongocxx"
cd $WORKINGDIR
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
  sudo apt install -y libssl-dev libsasl2-dev libsnappy-dev libbsd-dev
  sudo apt update && sudo apt install -y libboost-dev libboost-all-dev

  echo -e "${CYN}Downloading Mongo C driver (required for cpp driver)...${NC}"
  wget -c https://github.com/mongodb/mongo-c-driver/releases/download/1.21.1/mongo-c-driver-1.21.1.tar.gz -O - | tar -xz
  #tar xzf mongo-c-driver-1.21.1.tar.gz
  cd mongo-c-driver-1.21.1
  echo -e "${LIM}Installing Mongo C Driver...${NC}"
  [ ! -d "cmake-build" ] && mkdir cmake-build
  cd cmake-build
  cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF \
  -DCMAKE_INSTALL_PREFIX="$PKG_MAKE_PREFIX" ..
  EXIT_LOG "Configure CMake for MongoC" $?
  run_make_routine "MongoC"
  run_install_routine "MongoC"
  cd $WORKINGDIR
  echo -e "${YLW}Mongo C driver installation complete!${NC}"
  echo -e "${CYN}Downloading Mongo cpp driver...${NC}"
  curl -OL https://github.com/mongodb/mongo-cxx-driver/archive/r3.6.6.tar.gz
  tar -xzf r3.6.6.tar.gz
  echo -e "${LIM}Installing Mongo cpp Driver...${NC}"
  cd mongo-cxx-driver-r3.6.6
  [ ! -d "build" ] && mkdir build
  cd build
  cmake ..                                \
    -DCMAKE_BUILD_TYPE=Release          \
    -DCMAKE_CXX_STANDARD=11             \
    -DBUILD_SHARED_AND_STATIC_LIBS=ON   \
    -DBUILD_VERSION=1.21.1              \
    -DBSONCXX_POLY_USE_MNMLSTC=1        \
    -DCMAKE_PREFIX_PATH="$PKG_MAKE_PREFIX"     \
    -DCMAKE_INSTALL_PREFIX=$PKG_MAKE_PREFIX
  EXIT_LOG "Configure CMake for MongoCXX" $?
  sudo make EP_mnmlstc_core
  EXIT_LOG "Make EP_mnmlstc_core" $?
  run_make_routine "MongoCXX"
  run_install_routine "MongoCXX"
else
  echo -e "${GRN}Found $LIBRARY installation at $DEFAULT_INSTALL_DIR. Skipping install...${NC}"
fi
