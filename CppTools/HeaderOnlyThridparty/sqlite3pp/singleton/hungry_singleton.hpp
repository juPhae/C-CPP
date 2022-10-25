
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

  // string sql_cmd(string sql) {
  //   std::string string_array = Sqlite3pp::db_sql(sql);
  //   return string_array;
  // }

  // string sql_drop(string table) {
  //   std::string string_array = Sqlite3pp::db_drop(table);
  //   return string_array;
  // }

  // string sql_delete(string table, string where) {
  //   std::string string_array = Sqlite3pp::db_delete(table, where);
  //   return string_array;
  // }

  // string sql_create(string table, string col_para) {
  //   std::string string_array = Sqlite3pp::db_create(table, col_para);
  //   return string_array;
  // }

  // string sql_update(string table, string set, string where) {
  //   std::string string_array = Sqlite3pp::db_update(table, set, where);
  //   return string_array;
  // }

  // string sql_insert(string table, string cols, string vals) {
  //   std::string string_array = Sqlite3pp::db_insert(table, cols, vals);
  //   return string_array;
  // }
  
  // string sql_select(string table, string colname, string where, string order,
  //                   string limit) {
  //   std::string string_array =
  //       Sqlite3pp::db_select(table, colname, where, order, limit);
  //   return string_array;
  // }
  // ////////////////////////// 接口部分 //////////////////////////

private:
  // 将其构造和析构成为私有的, 禁止外部构造和析构
  SqlSingleton() : Sqlite3pp("/test.db");   // 使用绝对路径
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