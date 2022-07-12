# 目录结构

```
Cmake+Doxygen
├── bin
│   └── main
├── build
├── build.sh
├── CMakeLists.txt
├── cmake
│   └── build_doxygen.cmake
├── docs
│   ├── install.sh
│   ├── Doxygen.in
│   └── doxygen
│   │   ├── html
│   │   └── latex
├── Doxyfile
├── doxygen.md
├── src
│   ├── main.cpp
│   ├── Message.cpp
│   └── Message.hpp
├── thirdparty
│   └── dbg
│   │   └── dbg.hpp
└── README.md
```

- **bin:** 用于存放生成的可执行文件

- **bin/main:** 生成的可执行文件

- **build :** 用于存放build时cmake产生的中间文件

- **build.sh :** build脚本文件。

- **CMakeLists.txt :** cmake文件。

- **cmake :** 用于存放其他的cmake文件。

- **cmake/build_doxygen.cmake :** 生成doxygen的cmake文件。

- **Doxyfile :** 由cmake构建生成的Doxygen文件

- **doxygen.md :** doxygen相关的说明文件。

- **docs :** 用于存放项目的相关文档。

- **docs/install.sh :** 安装doxygen的脚本文件。

- **docs/Doxygen.in :** build_doxygen.cmake中引用的Doxygen.in，供cmake生成Doxygen文件。

- **src:** 用于存放源文件。

- **thirdparty :** 用于存放第三方库，每个第三库以单独目录的形式组织在thirdparty 目录下。

- **README.md :** 工程说明文件。

  

