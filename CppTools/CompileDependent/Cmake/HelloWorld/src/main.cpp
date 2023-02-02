
/*******************************************************************************
 *FileName: main.cpp
 *version:
 *Author: LZH
 *Date: 2022-06-19 10:55:29
 *Description:
 *LastEditors: LZH
 *LastEditTime: 2022-12-19 08:40:45
 ********************************************************************************/

#include "Message.hpp"
#include "../build/version.h"
#include <cstdio>
#include <iostream>
int main() {
  printf("Version: %d\n", VERSION);
  Message say_hello("Hello, CMake World!");
  std::cout << say_hello << std::endl;
  Message say_goodbye("Goodbye, CMake World");
  std::cout << say_goodbye << std::endl;
  return 0;
}