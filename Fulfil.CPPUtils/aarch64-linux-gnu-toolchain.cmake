# which compilers to use
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING TRUE)
set(target_arch aarch64-linux-gnu)
set(CMAKE_C_COMPILER "/usr/bin/${target_arch}-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/${target_arch}-g++")

set(CMAKE_LIBRARY_ARCHITECTURE ${target_arch})

# Where to look for the target environment. (More paths can be added here)
set(CMAKE_FIND_ROOT_PATH "/usr/${target_arch}")
set(CMAKE_INCLUDE_PATH "/usr/include/${target_arch}")
set(CMAKE_LIBRARY_PATH "/usr/lib/${target_arch}")
set(CMAKE_PROGRAM_PATH "/usr/bin/${target_arch}")

# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_STAGING_PREFIX /)