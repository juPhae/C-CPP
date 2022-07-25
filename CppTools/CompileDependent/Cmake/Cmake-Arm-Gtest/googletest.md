# GoogleTest框架编译

## Linux编译
### Ubuntu编译官方 GoogleTest
下载官方源码

	git clone  git@github.com:google/googletest.git
    
cmake 编译源码

    cd  googletest/googletest/

    #新建cmake 构建目录
    mkdir build  
    #cmake 构建
    cmake .. 
    #编译
    make 

如果需要构建Google Test的示例，使用以下命令替换cmake …

    cmake -Dgtest_build_samples=ON ${GTEST_DIR}
    
这里为止，完成了在Ubuntu 18.04平台上构建GoogleTest 的步骤，详细构建可查看 [官方文档](https://github.com/google/googletest)

## 交叉编译
基本思路: 使用arm交叉编译器编译GoogleTest, 生成可调用的动态库，应用层通过调用GoogleTest动态库完成测试用例的编写
### 指定arm交叉编译器

GoogleTest默认使用CMake构建，通过修改googletest/googletest/CMakeLists.txt,指定 arm32 平台使用的编译器, 如下

    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    #根据实际情况，指定交叉编译的路径
    #分别指定C和C++编译器 
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
CMake中如何指定编译器，[官方说明文档](https://cmake.org/cmake/help/git-master/manual/cmake-toolchains.7.html?highlight=cmake_c_compiler#id5)中有详细描述

生成动态库文件
方便应用层编写测试用例，这里需要编译出动态库。 还是修改`googletest/googletest/CMakeLists.txt `   

    option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
    #改为
    option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." ON)

    
    
如果出现

    CMake Error at CMakeLists.txt:xxx (set_target_properties):
      set_target_properties called with incorrect number of arguments.


    CMake Error at CMakeLists.txt:xxx (set_target_properties):
      set_target_properties called with incorrect number of arguments.


    -- Configuring incomplete, errors occurred!


手动加上版本号
	#与实际版本对应
    set(GOOGLETEST_VERSION 1.21.1)
    
最终再build/lib目录下出现以下四个文件则编译完成。
	
    libgtest.so 
    libgtest.so.1.21.1
    libgtest_main.so
    libgtest_main.so.1.21.1

#### gtest加入到项目

把 libgtest.so 、libgtest_main.so 以及 gtest头文件加入到项目中。

