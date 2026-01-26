#!/bin/bash
#
# code style, refer to: https://google.github.io/styleguide/shellguide.html

readonly BASH_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

readonly ROOT_PROJECT_DIR=${BASH_DIR}

readonly CMAKE_SRC_DIR="${ROOT_PROJECT_DIR}/src"
readonly CMAKE_BUILD_DIR="${ROOT_PROJECT_DIR}/build"

# 
BUILD_SUB_DIR="win_x64"
CMAKE_BUILD_OUT_DIR="${CMAKE_BUILD_DIR}/${BUILD_SUB_DIR}"
LOG_MSG="generate ${BUILD_SUB_DIR} project" 
echo "${LOG_MSG} ..."
cmake -S "${CMAKE_SRC_DIR}" \
  -B "${CMAKE_BUILD_OUT_DIR}" \
  -G "Visual Studio 17 2022" -A x64 -T v143 \
  -DCMAKE_CONFIGURATION_TYPES="Debug;Release" \
  -DUSE_STATIC_LIBS:BOOL=ON 

if [ $? -ne 0 ]; then
  echo "${LOG_MSG} failed"
  exit 1
fi
echo "${LOG_MSG} success"

echo "end"
