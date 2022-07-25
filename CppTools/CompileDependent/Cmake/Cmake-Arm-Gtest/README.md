
[参考链接1](https://cloud.tencent.com/developer/article/1393469#:~:text=%E5%9F%BA%E6%9C%AC%E6%80%9D%E8%B7%AF%3A,%E4%BD%BF%E7%94%A8arm%E4%BA%A4%E5%8F%89%E7%BC%96%E8%AF%91%E5%99%A8%E7%BC%96%E8%AF%91GoogleTest%2C%20%E7%94%9F%E6%88%90%E5%8F%AF%E8%B0%83%E7%94%A8%E7%9A%84%E5%8A%A8%E6%80%81%E5%BA%93%EF%BC%8C%E5%BA%94%E7%94%A8%E5%B1%82%E9%80%9A%E8%BF%87%E8%B0%83%E7%94%A8GoogleTest%E5%8A%A8%E6%80%81%E5%BA%93%E5%AE%8C%E6%88%90%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E7%9A%84%E7%BC%96%E5%86%99) 

[参考链接2](https://github.com/TechNotesDoc/Notes/blob/master/Doc/01.Linux%E7%9B%B8%E5%85%B3/05.%E5%BC%80%E6%BA%90%E7%A8%8B%E5%BA%8F/02.gtest/gtest.md)

** 由于用1.12.1版本编译出错 ，所以降级到1.10.0版本 **

### 第一步
 下载googletest源码，交叉编译成动态链接库

### 第二步
把编译好的 gtest 动态库和头文件添加到工程项目中

### 第三步
把编译的工程项目中的可执行文件和libgtest.so（gtest动态链接库文件）一起上传到arm开放上。

### 第四步
运行`./arm-gtest`

    
    Hello, World!
    [==========] Running 3 tests from 1 test suite.
    [----------] Global test environment set-up.
    [----------] 3 tests from TestCase
    [ RUN      ] TestCase.test1
    [       OK ] TestCase.test1 (0 ms)
    [ RUN      ] TestCase.test2
    [       OK ] TestCase.test2 (0 ms)
    [ RUN      ] TestCase.test3
    [       OK ] TestCase.test3 (0 ms)
    [----------] 3 tests from TestCase (0 ms total)

    [----------] Global test environment tear-down
    [==========] 3 tests from 1 test suite ran. (2 ms total)
    [  PASSED  ] 3 tests.

