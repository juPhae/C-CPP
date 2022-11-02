# 设置CMake所需的最低版本 3.5。如果使用的CMake版本低于该版本，则会抛出错误：
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# 声明了项目的名称和支持的编程语言(CXX代表C++)：
project(oatppServer LANGUAGES CXX)

# 设置可执行文件名
# set(TARGET "oatppServer")

# # 设置编译器路径
set(CMAKE_CXX_COMPILER "${ARM_CXX_APTH}arm-linux-gnueabihf-g++")

file(GLOB_RECURSE SRC_DIR_LIST_1 "src/*.cpp" "src/*.c") # 遍历获取src/路径下所有的*.cpp和*.c文件列表

# file(GLOB_RECURSE SRC_DIR_LIST_2 "thirdparty/*.cpp" "thirdparty/*.c") # 遍历获取source/路径下所有的*.cpp和*.c文件列表

# 自动查找头文件路径函数(增加去重部分)     # FIND_INCLUDE_DIR()调用时要指定绝对路径
macro(FIND_INCLUDE_DIR result curdir) # 定义函数,2个参数:存放结果result；指定路径curdir；
    file(GLOB_RECURSE children "${curdir}/*.hpp" "${curdir}/*.h") # 遍历获取{curdir}中*.hpp和*.h文件列表

    set(dirlist "") # 定义dirlist中间变量，并初始化

    foreach(child ${children}) # for循环
        string(REGEX REPLACE "(.*)/.*" "\\1" LIB_NAME ${child}) # 字符串替换,用/前的字符替换/*h

        if(IS_DIRECTORY ${LIB_NAME}) # 判断是否为路径
            list(FIND dirlist ${LIB_NAME} list_index)

            if(${list_index} LESS 0)
                LIST(APPEND dirlist ${LIB_NAME}) # 将合法的路径加入dirlist变量中
            else()
            endif() # 结束判断
        endif()
    endforeach() # 结束for循环

    set(${result} ${dirlist}) # dirlist结果放入result变量中
endmacro()

# 查找src目录下的所有*.hpp,*.h头文件,并路径列表保存到 INCLUDE_DIR_LIST_1 变量中
FIND_INCLUDE_DIR(INCLUDE_DIR_LIST_1 ${PROJECT_SOURCE_DIR}/src) # 调用函数，指定参数

# # 查找thirdparty目录下的所有*.hpp,*.h头文件,并路径列表保存到 INCLUDE_DIR_LIST_2 变量中
# FIND_INCLUDE_DIR(INCLUDE_DIR_LIST_2 ${PROJECT_SOURCE_DIR}/thirdparty) # 调用函数，指定参数

# 将INCLUDE_DIR_LIST中路径列表加入工程
include_directories(
    ${INCLUDE_DIR_LIST_1}

    # ${INCLUDE_DIR_LIST_2}
)

# # 设定可执行二进制文件的目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# ##  引入编译器检查
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

# ## 优先使用高版本的标准
if(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "Using flag -std=c++0x.")
elseif(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(CMAKE_CXX_STANDARD 17)
    message(STATUS "Using flag -std=c++17.")
elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(CMAKE_CXX_STANDARD 14)
    message(STATUS "Using flag -std=c++14.")
elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_STANDARD 11)
    message(STATUS "Using flag -std=c++11.")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# # 时间戳
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")
message("Build timestamp is ${BUILD_TIMESTAMP}")

# # 版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 001)
message("Version is ${VERSION_MAJOR} ${VERSION_MINOR} ${VERSION_PATCH}")

# 加入版本号
configure_file(cmake/version.h.in version.h)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

# 设置install之后的库文件夹加载的路径
set(CMAKE_INSTALL_RPATH "/app/oatppServer/lib") # set install runtime path

# 构建install文件夹
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# # 查找 oatpp 依赖
find_package(oatpp REQUIRED)

if(oatpp_FOUND)
    message(STATUS "Found oatpp version: ${oatpp_VERSION_STRING}")
else()
    message(FATAL_ERROR "Could not find oatpp")
endif()

# # 添加多个源文件  生成可以执行的文件
add_executable(${PROJECT_NAME} ${SRC_DIR_LIST_1})

# # 将目标文件与库文件进行链接
target_link_libraries(${PROJECT_NAME} oatpp::oatpp)

# # define path to swagger-ui res folder
# add_definitions(
# -DOATPP_SWAGGER_RES_PATH="${OATPP_BASE_DIR}/bin/oatpp-swagger/res"
# )
install(TARGETS ${PROJECT_NAME} DESTINATION .)