
/*******************************************************************************
 *FileName: hungry_singleton.hpp
 *version:
 *Author: Phae
 *Date: 2022-10-25 15:08:18
 *Description:
 *LastEditors: LZH
 *LastEditTime: 2022-10-25 15:12:29
 ********************************************************************************/

#pragma once

#ifndef __HUNGRY_SINGLETON_HPP__
#define __HUNGRY_SINGLETON_HPP__

#include "sqlite3pp.hpp"

////////////////////////// 饿汉实现 /////////////////////
// 饿汉式是以空间换时间，适应于访问量较大时，或者线程比较多的的情况

class SqlSingleton : public Sqlite3pp {
public:
  // 获取单实例
  static SqlSingleton *GetInstance() { return g_pSqlSingleton; };

  // 释放单实例，进程退出时调用
  static void deleteInstance() {
    if (g_pSqlSingleton) {
      delete g_pSqlSingleton;
      g_pSqlSingleton = nullptr;
    }
  };

  // ////////////////////////// 接口部分 //////////////////////////

  //  根据表名查询一张表
  std::string queryTable(string table) {
    std::string retsult = Sqlite3pp::db_select(table, "", "", "", "");
    return retsult;
  }

  //  根据表名列名查询指定的表中的列
  std::string queryTableByColConditions(string table,
                                        std::vector<std::string> conditions) {
    std::string str;
    for (auto p = conditions.begin(); p != conditions.end(); ++p) {
      str += *p + " and ";
    }
    str = str.substr(0, str.size() - 4);
    std::string retsult = Sqlite3pp::db_select(table, "", str, "", "");
    return retsult;
  }
  //  根据列的条件查询一张表
  std::string queryColumnFromTable(string table,
                                   std::vector<std::string> column) {
    std::string str;
    for (auto p = column.begin(); p != column.end(); ++p) {
      str += *p + ",";
    }
    str = str.substr(0, str.size() - 1);
    std::string retsult = Sqlite3pp::db_select(table, str, "", "", "");
    return retsult;
  }

  //  根据排序的条件查询一张表
  std::string queryTableBySort(string table,
                               std::vector<std::string> conditions) {
    std::string str;
    for (auto p = conditions.begin(); p != conditions.end(); ++p) {
      str += *p + ",";
    }
    str = str.substr(0, str.size() - 1);
    std::string retsult = Sqlite3pp::db_select(table, "", "", str, "");
    return retsult;
  }

  //  根据 分页条件查询一张表， index 表示第 i 页， size表示s条
  std::string queryTableByPage(string table, int index, int size) {
    std::string str =
        std::to_string((index - 1) * size) + "," + std::to_string(size);
    // std::string str = std::to_string(size) + " OFFSET " +
    // std::to_string(index);

    std::string retsult = Sqlite3pp::db_select(table, "", "", "", str);
    return retsult;
  }

  //  根据 分页条件查询一张表 // num 表示 n行
  std::string queryTableByNumbers(string table, int num) {
    std::string str = std::to_string(num);
    std::string retsult = Sqlite3pp::db_select(table, "", "", "", str);
    return retsult;
  }

  // ////////////////////////// 接口部分 //////////////////////////

private:
  // 将其构造和析构成为私有的, 禁止外部构造和析构
  SqlSingleton() : Sqlite3pp("/test.db"); // 使用绝对路径
  ~SqlSingleton() = default;

  // 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
  SqlSingleton(const SqlSingleton &signal);
  const SqlSingleton &operator=(const SqlSingleton &signal);

private:
  // 唯一单实例对象指针
  // 代码一运行就初始化创建实例 ，本身就线程安全
  static SqlSingleton *g_pSqlSingleton = new (std::nothrow) SqlSingleton;
};

////////////////////////// 饿汉实现 /////////////////////

#endif //__HUNGRY_SINGLETON_HPP__