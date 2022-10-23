
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# 创建工程，名称为：my_project
project(my_project)

# 基于 main.cpp生成可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 设定可执行二进制文件的目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#设置cpm
set(CPM_DOWNLOAD_VERSION 0.32.0) 
# 设置在 build/cmake路径 set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
  file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()
include(${CPM_DOWNLOAD_LOCATION})

# 使用完整语法直接基于URL添加另一个依赖库 依赖库名json ，版本号为3.7.3
CPMAddPackage("gh:nlohmann/json@3.7.3")

CPMAddPackage("gh:gabime/spdlog@1.8.2")

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.10.0
  VERSION 1.10.0
  OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

# 这样写也行哦  target_link_libraries(${PROJECT_NAME} spdlog gtest gtest_main gmock nlohmann_json)
target_link_libraries(${PROJECT_NAME} spdlog gtest gtest_main gmock nlohmann_json::nlohmann_json)
target_compile_features(${PROJECT_NAME}  PRIVATE cxx_std_17)

