#!/bin/bash

BLU='\033[1;34m'
RED='\033[0;31m'
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

function arch_alias { if [[ "$(uname -p )" == "aarch64" ]] ;  then echo "arm64"
  elif [[ "$(uname -p )" == "x86_64" ]] ; then echo  "amd64" ; else uname -p ; fi }

if [[ "$USER" != "root" ]] ; then
    echo -e "${RED}Must run script with root permissions! Please re-run using:${NC}\n  sudo ./install_shared_grpc_proto_deps.sh"
    exit 1
fi

WORKDIR="${HOME}/sources"
[ -d "${WORKDIR}" ] || mkdir "${WORKDIR}"
cd "${WORKDIR}" || exit
BUILD_LOG_ROOT="$WORKDIR"
ARCH="$(uname -p )"
ARCH_ALIAS="$( arch_alias )"

BAZEL_VERSION=1.16.0

GRPC_INSTALL_DIR="/opt"
GRPC_VERSION=1.54.0

#PROTOBUF_ASSET_TYPE="python"
PROTOBUF_ASSET_TYPE="cpp"
PROTOBUF_RELEASE_TAG="21.12"
PROTOBUF_VERSION_CPP="3.21.12"
PROTOBUF_VERSION_PY="4.21.12"

# Install Bazel
BAZEL_BIN="bazelisk-linux-${ARCH_ALIAS}"
BAZEL_URL=https://github.com/bazelbuild/bazelisk/releases/download/v${BAZEL_VERSION}/${BAZEL_BIN}

echo " **** Install $BAZEL_BIN **** "
if [ -f /usr/bin/bazel ] ; then
  echo "Found existing installation of bazel bin"
else
  wget  --no-check-certificate ${BAZEL_URL} && mv ${BAZEL_BIN} /usr/bin/bazel || exit
  chmod +x /usr/bin/bazel
  echo "Bazel v${BAZEL_VERSION} install complete "
fi

# Install Protobuf

echo " **** Protobuf v${PROTOBUF_RELEASE_TAG}: ${PROTOBUF_ASSET_TYPE}-${PROTOBUF_VERSION} **** "

PROTOBUF_VERSION="$( [ ${PROTOBUF_ASSET_TYPE} = 'python' ]  && echo $PROTOBUF_VERSION_PY || echo $PROTOBUF_VERSION_CPP)"
PROTOBUF_DIR="protobuf-${PROTOBUF_ASSET_TYPE}-$PROTOBUF_VERSION"
PROTOBUF_URL="https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_RELEASE_TAG}/${PROTOBUF_DIR}.tar.gz"



if [ -d "${PROTOBUF_DIR}" ] ; then
  echo "Found ${PROTOBUF_DIR} dir! Skipping download."
else
  mkdir "${PROTOBUF_DIR}"
  echo "Get ${PROTOBUF_URL}"
  wget --no-check-certificate -c "$PROTOBUF_URL" -O - | tar -xz -C "$PROTOBUF_DIR" --strip-components 1
fi

if ! pip3 -V &>/dev/null ; then
  apt-get update && apt-get install -y --no-install-recommends python3-pip
fi

cd ${PROTOBUF_DIR} || exit
  ./autogen.sh && ./configure --prefix=/usr | tee "${BUILD_LOG_ROOT}/protobuf_configure_out.txt"
  make -j "$(( $(nproc) - 2 ))" | tee "${BUILD_LOG_ROOT}/protobuf_make_out.txt"
  make check -j "$(( $(nproc) - 4 ))" | tee "${BUILD_LOG_ROOT}/protobuf_make_check_out.txt"
  make install  | tee "${BUILD_LOG_ROOT}/protobuf_install_out.txt" && ldconfig
  if [ ${PROTOBUF_ASSET_TYPE} = 'python' ] ; then
      pip3 install --no-cache-dir setuptools Cython wheel || exit
      cd python || exit
      echo "Building python protobuf..."
      python3 setup.py build --cpp_implementation  | tee "${BUILD_LOG_ROOT}/protobuf_py_build_out.txt"
      echo "Test python protobuf..."
      python3 setup.py test --cpp_implementation  | tee "${BUILD_LOG_ROOT}/protobuf_py_test_out.txt"
      echo "Make python protobuf wheel..."
      python3 setup.py bdist_wheel --cpp_implementation | tee "${BUILD_LOG_ROOT}/protobuf_py_wheel_out.txt"
      echo "Install python protobuf wheel"
      cp dist/*.whl /opt && pip3 install dist/*.whl | tee "${BUILD_LOG_ROOT}/protobuf_py_pip_out.txt" || exit
      echo "Test python proto installation"
      pip3 show protobuf || exit
  fi
echo "Test protoc installation"
protoc --version || exit

# Install gRPC

echo -e "\n${CYN}Do you want to install gRPC system wide?${NC}"
continue_or_skip || exit 0

cd "$WORKDIR" || exit
export LD_LIBRARY_PATH="/usr/lib:$LD_LIBRARY_PATH"
export PATH="$GRPC_INSTALL_DIR/bin:$PATH"

echo " **** gRPC v${GRPC_VERSION} **** "
GRPC_BUILD_CONFIG="${WORKDIR}/grpc/build_handwritten.yaml"

[ -d grpc ] && echo "Found grpc dir! Skipping download." \
  || git clone --recurse-submodules -b v${GRPC_VERSION} --depth 1 --shallow-submodules https://github.com/grpc/grpc
  mkdir -p grpc/cmake/build && cd grpc || exit
  [ -f "$GRPC_BUILD_CONFIG" ] && echo "Found valid grpc build info!" || exit
  echo "Source gRPC version $(grep '  version:' "${GRPC_BUILD_CONFIG}" | sed 's/  version://g')..."
  echo "Protobuf Expected by gRPC: $(grep "protobuf_version:" "${GRPC_BUILD_CONFIG}" | sed 's/protobuf_version://g' )"
  echo "Protobuf Expected by script: $PROTOBUF_RELEASE_TAG"
  echo "System Protoc: $(protoc --version)"
  if [ ${PROTOBUF_ASSET_TYPE} = 'python' ] ; then echo "Python Protobuf: $(pip3 show protobuf)" ; fi
  cd cmake/build || exit
  cmake ../.. \
        -D gRPC_INSTALL=ON \
        -D CMAKE_INSTALL_PREFIX="${GRPC_INSTALL_DIR}" \
        -D CMAKE_BUILD_TYPE=Release \
        -D gRPC_BUILD_TESTS=OFF \
        -D ABSL_PROPAGATE_CXX_STD=ON \
        -D gRPC_PROTOBUF_PROVIDER=package \
        -D protobuf_VERBOSE=ON  | tee "${BUILD_LOG_ROOT}/grcp_cmake_out.txt"
  make -j "$(( $(nproc) - 1 ))" | tee "${BUILD_LOG_ROOT}/grcp_make_out.txt"
  make install | tee "${BUILD_LOG_ROOT}/grcp_install_out.txt"




