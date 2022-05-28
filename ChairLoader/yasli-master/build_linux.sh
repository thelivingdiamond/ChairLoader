#!/bin/sh

BUILD_ROOT=$(dirname $(readlink -f $0))

CONFIGURATION=Debug
TEMP_DIR=${BUILD_ROOT}/.tmp/linux-${CONFIGURATION}

mkdir -p ${TEMP_DIR} 
cd ${TEMP_DIR} &&
CC=clang CXX=clang++ cmake \
	-DTARGET_LINUX=1 \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DCMAKE_BUILD_TYPE=${CONFIGURATION} \
	-G "Unix Makefiles" ${BUILD_ROOT} &&
make -j6
