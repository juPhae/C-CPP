
/******************************************************************************* 
  *FileName: main.cpp
  *version: 
  *Author: LZH
  *Date: 2022-06-19 10:55:29
  *Description: 
  *LastEditors: LZH
  *LastEditTime: 2022-06-19 11:25:45
 ********************************************************************************/

#include "Message.hpp"
#include <iostream>
int main() {
  Message say_hello("Hello, CMake World!");
  std::cout << say_hello << std::endl;
  Message say_goodbye("Goodbye, CMake World");
  std::cout << say_goodbye << std::endl;
  return 0;
}