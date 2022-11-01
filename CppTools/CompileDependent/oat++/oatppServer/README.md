



# C++的Web框架(OAT++)

## 通过脚本一键下载

```
cd scripts && ./get-oatpp-modules.sh
```

通过脚本会下载到tmp文件夹

## 手动下载：

下载oat++：

```shell
git clone https://github.com/oatpp/oatpp.git
```

下载oatpp-swagger：

```
git clone https://github.com/oatpp/oatpp-swagger.git
```

下载oatpp-sqlite：

```
git clone https://github.com/oatpp/oatpp-sqlite.git
```



## 安装说明：

## oat++的交叉编译安装说明

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



## oat++的swagger安装说明

修改编译器选项：

在oatpp-swagger/CMakeLists.txt 中加入

```shell
# # 设置编译器路径
set(CMAKE_CXX_COMPILER "${ARM_CXX_APTH}arm-linux-gnueabihf-g++")
```



编译和安装：

```shell
cd oatpp-swagger/
mkdir build && cd build
cmake ..
make && sudo make install
```



## oat++的sqlite安装说明



修改编译器选项：

在oatpp-sqlite/CMakeLists.txt 中加入

```shell
# # 设置编译器路径
set(CMAKE_CXX_COMPILER "${ARM_CXX_APTH}arm-linux-gnueabihf-g++")
```



编译和安装：

```shell
cd oatpp-sqlite/
mkdir build && cd build
cmake .. -DOATPP_SQLITE_AMALGAMATION=ON
make && sudo make install
```

如果出现了链接错误

```shell
[100%] Linking CXX executable module-tests
../src/libsqlite.a: 无法添加符号: 不可识别的文件格式
collect2: error: ld returned 1 exit status
test/CMakeFiles/module-tests.dir/build.make:182: recipe for target 'test/module-tests' failed
make[2]: *** [test/module-tests] Error 1
CMakeFiles/Makefile2:170: recipe for target 'test/CMakeFiles/module-tests.dir/all' failed
make[1]: *** [test/CMakeFiles/module-tests.dir/all] Error 2
Makefile:145: recipe for target 'all' failed
make: *** [all] Error 2
```

则手动编译sqlite成库文件，替换libsqlite.a

```
cd oatpp-sqlite/src/sqlite
arm-linux-gnueabihf-gcc -c sqlite3.c
ar -rv libsqlite.a sqlite3.o

cp libsqlite.a ../../build/src/
```

然后重新回到build目录重新执行 `make && sudo make install`

