#!/bin/bash

BUILD_TYPE=$1

# COMPILER_PATH=/usr/bin/g++

COMPILER_PATH=/usr/bin/arm-linux-gnueabihf-g++

if [ -z "$BUILD_TYPE" ]; then
    BUILD_TYPE="Debug"
fi

rm -rf tmp

mkdir tmp
cd tmp

##########################################################
## install oatpp module

function install_module() {

    BUILD_TYPE=$1
    MODULE_NAME=$2
    NPROC=$(nproc)

    if [ -z "$NPROC" ]; then
        NPROC=1
    fi

    printf "\n\nINSTALLING MODULE '$MODULE_NAME' ($BUILD_TYPE) using $NPROC threads ...\n\n"

    git clone --depth=1 https://github.com/oatpp/$MODULE_NAME

    cd $MODULE_NAME
    mkdir build
    cd build

    # OatSqlite=oatpp-sqlite
    # if [ $MODULE_NAME = $OatSqlite ]; then
    #     ############################################################################
    #     ## Flag '-DOATPP_SQLITE_AMALGAMATION=ON' used by oatpp-sqlite module only ##
    #     ############################################################################
    #     cmake -DCMAKE_CXX_COMPILER=${COMPILER_PATH} -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DOATPP_BUILD_TESTS=OFF -DOATPP_SQLITE_AMALGAMATION=ON ..
    #     sudo make install -j $NPROC
    # else
    #     cmake -DCMAKE_CXX_COMPILER=${COMPILER_PATH} -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DOATPP_BUILD_TESTS=OFF ..
    #     sudo make install -j $NPROC
    # fi

    cd ../../

}

##########################################################

install_module $BUILD_TYPE oatpp
install_module $BUILD_TYPE oatpp-swagger
install_module $BUILD_TYPE oatpp-sqlite

cd ../
# rm -rf tmp
