# 设置CMake所需的最低版本 3.5。如果使用的CMake版本低于该版本，则会发出致命错误：
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# 声明了项目的名称和支持的编程语言(CXX代表C++)：
project(main LANGUAGES CXX)

# 设置编译器
set(CMAKE_CXX_COMPILER "g++")

# # 搜集所有在指定路径下的源文件的文件名，将输出结果列表储存在指定的变量中
# aux_source_directory(./ SRC_LIST)

# 查找所有*.cpp,*.c源文件,并将文件列表保存到 DIR_LIB_SRCS 变量
file(GLOB_RECURSE SRC_DIR_LIST_1 "src/*.cpp" "src/*.c") # 遍历获取src/路径下所有的*.cpp和*.c文件列表
file(GLOB_RECURSE SRC_DIR_LIST_2 "thirdparty/*.cpp" "thirdparty/*.c") # 遍历获取source/路径下所有的*.cpp和*.c文件列表

message(STATUS "SRC_DIR_LIST_1 = ${SRC_DIR_LIST_1}") # 打印SRC_DIR_LIST_1中列表信息
message(STATUS "SRC_DIR_LIST_2 = ${SRC_DIR_LIST_2}") # 打印SRC_DIR_LIST_2中列表信息

# 将 SRC_DIR_LIST_1 SRC_DIR_LIST_2 中文件列表以静态库的方式加入工程
# add_library(
# ${PROJECT_NAME} STATIC
# ${SRC_DIR_LIST_1}
# ${SRC_DIR_LIST_2} 
# )

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

# #并且把该目录设为连接目录
# LINK_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lib)
# #
# find_package (Threads REQUIRED)
# #设定头文件目录
# INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/include)

# #增加子文件夹(注意我们的src路径是通过子文件夹的形式添加的，在这里面也是可以些CMakeLists.txt的)
# ADD_SUBDIRECTORY( ${PROJECT_SOURCE_DIR}/src)
# #或是ADD_SUBDIRECTORY( src)

# #连接共享库
# target_link_libraries(main calculate_shared)

# 添加多个源文件  生成可以执行的文件
add_executable(main ${SRC_DIR_LIST_1} ${SRC_DIR_LIST_2})