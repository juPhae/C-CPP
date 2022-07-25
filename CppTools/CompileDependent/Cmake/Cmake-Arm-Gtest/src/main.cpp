
/******************************************************************************* 
  *FileName: main.cpp
  *version: 
  *Author: phae
  *Date: 2022-07-25 11:13:54
  *Description: 单元测试，需要链接 libgtest.so
  *LastEditTime: 2022-07-25 14:17:15
 ********************************************************************************/

#include <iostream>
#include "gtest/gtest.h"  //这里修改为双引号，不在使用<>了，意味着不会从系统库里面搜索

int add(int n1, int n2) { return n1 + n2; }

TEST(TestCase, test1) { ASSERT_EQ(12, add(4, 8)); }

TEST(TestCase, test2) { EXPECT_EQ(5, add(2, 3)); }

TEST(TestCase, test3) { EXPECT_EQ(3, add(1, 2)); }

int main(int argc, char **argv) {
  std::cout << "Hello, World!" << std::endl;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
