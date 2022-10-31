



## oat++的交叉编译安装说明

下载：

```shell
git clone https://github.com/oatpp/oatpp.git
```

修改编译器选项：

在oatpp/CMakeLists.txt 中加入

```shell
# # 设置编译器路径
set(CMAKE_CXX_COMPILER "${ARM_CXX_APTH}arm-linux-gnueabihf-g++")
```



编译和安装：

```shell
cd oatpp/
mkdir build && cd build
cmake ..
make && sudo make install
```

