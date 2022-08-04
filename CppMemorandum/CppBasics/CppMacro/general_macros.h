/*******************************************************************************
 *FileName: general_macros.h
 *Author: phae
 *Date: 2022-08-04 10:00:25
 *Description:  自定义一些常用的方法宏,适用与Linux
 ********************************************************************************/
#pragma once

#ifndef __GENERAL_MACROS_H__
#define __GENERAL_MACROS_H__

#include <fstream>
#include <iostream>
#include <cstring>
// 用于抛出异常，输入的参数类型为const char * ;
#define THROW_STOP(error) throw std::invalid_argument(error);

//将cout输出到文件，会覆盖输出的文件,注意同一个cpp文件中多次使用会重定义
#define COUT_TO_FILE(filename, output)                                         \
  do {                                                                         \
    std::ofstream fout(filename);                                              \
    std::streambuf *p_o_id = std::cout.rdbuf(fout.rdbuf());                    \
    std::cout << output << std::flush;                                         \
    std::cout.rdbuf(p_o_id);                                                   \
  } while (0)
  
//去掉__FILE__宏的中的路径，只保留一个文件名称
#define _FILE_NMAE_                                                            \
  (strrchr(__FILE__, '/')                                                      \
       ? strrchr(__FILE__, '/') + 1                                            \
       : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

//  简单的log 宏
#ifndef __LOG__BY_SPRINTF__
#define __LOG__BY_SPRINTF__

#endif //__LOG__BY_SPRINTF__

#endif //__GENERAL_MACROS_H__