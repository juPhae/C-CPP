
/*******************************************************************************
 *FileName: main.cpp
 *version:
 *Author: phae
 *Date: 2022-06-19 10:55:29
 *Description:
 *LastEditors: phae
 *LastEditTime: 2022-10-31 12:47:34
 ********************************************************************************/

/**
 * @file main.cpp
 * @brief 测试头文件
 * @details 这个是测试Doxygen
 * @mainpage 工程概览
 * @author phae
 * @email phae@lhlover.com
 * @version 1.0.0
 * @date 2022-06-20
 */

#include "Message.hpp"
#include "version.h"
#include <iostream>
#define VERSIONS                                                               \
  VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH " " BUILD_TIMESTAMP

/*
版本格式：主版本号.次版本号.修订号 时间戳

版本号递增规则如下：
      主版本号：当你做了不兼容的 API 修改，
      次版本号：当你做了向下兼容的功能性新增，
      修订号：当你做了向下兼容的问题修正。
      先行版本号及版本编译元数据可以加到“主版本号.次版本号.修订号”的后面，作为延伸。

*/

int main() {
  std::cout << "System V" << VERSIONS << std::endl;

  Message say_hello("Hello, CMake World!");
  std::cout << say_hello << std::endl;
  Message say_goodbye("Goodbye, CMake World");
  std::cout << say_goodbye << std::endl;
  return 0;
}