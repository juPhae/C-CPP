# 设置CMake所需的最低版本 3.5。如果使用的CMake版本低于该版本，则会发出致命错误：
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(arm-gtest)

# 设置编译器
set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")

# 设定可执行二进制文件的目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# # 设置编译选项
# # 允许c++20标准、多线程。
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x  -lpthread -lm ")


# # 指定googleteste的搜索路径
set(GTEST_SYSROOT ./thirdparty/googletst)


# #gtest 头文件路径
set(GTEST_H ${GTEST_SYSROOT}/include)
# # gtest 库文件路径
set(GTEST_SO ${GTEST_SYSROOT}/lib)

 message(STATUS "GTEST_H =  ${GTEST_H}")
 message(STATUS "GTEST_SO =  ${GTEST_SO}")

 # #gtest 头文件路径加入到项目中
 include_directories(
        ${GTEST_H}
 )

# #gtest 库文件路径加入到链接的路径中
link_directories(${GTEST_SO})

##指定 rpath目录 为当前目录，可执行文件和so文件在同一个目录
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)


add_executable(arm-gtest ./src/main.cpp)
target_link_libraries(arm-gtest gtest)         # 链接最终目录程序
