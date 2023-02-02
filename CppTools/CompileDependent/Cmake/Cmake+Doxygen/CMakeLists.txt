# 设置CMake所需的最低版本 3.5。如果使用的CMake版本低于该版本，则会发出致命错误：
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# 声明了项目的名称和支持的编程语言(CXX代表C++)：
project(main LANGUAGES CXX)

# 设置编译器
set(CMAKE_CXX_COMPILER "g++")

# 查找所有*.cpp,*.c源文件,并将文件列表保存到 DIR_LIB_SRCS 变量
file(GLOB_RECURSE SRC_DIR_LIST_1 "src/*.cpp" "src/*.c") # 遍历获取src/路径下所有的*.cpp和*.c文件列表
file(GLOB_RECURSE SRC_DIR_LIST_2 "thirdparty/*.cpp" "thirdparty/*.c") # 遍历获取source/路径下所有的*.cpp和*.c文件列表

message(STATUS "SRC_DIR_LIST_1 = ${SRC_DIR_LIST_1}") # 打印SRC_DIR_LIST_1中列表信息
message(STATUS "SRC_DIR_LIST_2 = ${SRC_DIR_LIST_2}") # 打印SRC_DIR_LIST_2中列表信息

# 自动查找头文件路径函数(增加去重部分)     # FIND_INCLUDE_DIR()调用时要指定绝对路径
macro(FIND_INCLUDE_DIR result curdir) # 定义函数,2个参数:存放结果result；指定路径curdir；
    file(GLOB_RECURSE children "${curdir}/*.hpp" "${curdir}/*.h") # 遍历获取{curdir}中*.hpp和*.h文件列表
    message(STATUS "children= ${children}") # 打印*.hpp和*.h的文件列表
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

# 查找thirdparty目录下的所有*.hpp,*.h头文件,并路径列表保存到 INCLUDE_DIR_LIST_2 变量中
FIND_INCLUDE_DIR(INCLUDE_DIR_LIST_2 ${PROJECT_SOURCE_DIR}/thirdparty) # 调用函数，指定参数

message(STATUS "INCLUDE_DIR_LIST_1 = ${INCLUDE_DIR_LIST_1}") # 打印INCLUDE_DIR_LIST_1中列表信息
message(STATUS "INCLUDE_DIR_LIST_2 = ${INCLUDE_DIR_LIST_2}") # 打印INCLUDE_DIR_LIST_2中列表信息

# 将INCLUDE_DIR_LIST中路径列表加入工程
include_directories( # INCLUDE_DIR_LIST路径列表加入工程
    ${INCLUDE_DIR_LIST_1}
    ${INCLUDE_DIR_LIST_2}
)

# # 设置编译选项
# # 允许c++11标准、O3优化、多线程。match选项可避免一些cpu上的问题
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")

# 设定可执行二进制文件的目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设定存放编译出来的库文件的目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

message(STATUS "Build documentation open")

# 引入 build_doxygen
include(cmake/build_doxygen.cmake)
build_doxygen()

# # 时间戳
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")
message("Build timestamp is ${BUILD_TIMESTAMP}")

# # 版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
message("Version is ${VERSION_MAJOR} ${VERSION_MINOR} ${VERSION_PATCH}")

# 加入版本号
# 需要重新执行cmake才会生效。
configure_file(cmake/version.h.in version.h)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

# 添加多个源文件  生成可以执行的文件
add_executable(main ${SRC_DIR_LIST_1} ${SRC_DIR_LIST_2})
