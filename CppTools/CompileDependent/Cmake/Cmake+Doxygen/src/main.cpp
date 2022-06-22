
/******************************************************************************* 
  *FileName: main.cpp
  *version: 
  *Author: LZH
  *Date: 2022-06-19 10:55:29
  *Description: 
  *LastEditors: LZH
  *LastEditTime: 2022-06-22 17:45:20
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
#include <iostream>
int main() {
  Message say_hello("Hello, CMake World!");
  std::cout << say_hello << std::endl;
  Message say_goodbye("Goodbye, CMake World");
  std::cout << say_goodbye << std::endl;
  return 0;
}