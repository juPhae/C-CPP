
/*******************************************************************************
 *FileName: lazy_singleton.hpp
 *version:
 *Author: Phae
 *Date: 2022-10-25 15:58:17
 *Description:
 *LastEditors: Phae
 *LastEditTime: 2022-10-25 15:58:41
 ********************************************************************************/
#pragma once

#ifndef __LAZY_SINGLETON_HPP__
#define __LAZY_SINGLETON_HPP__

#include "../sqlite3pp.hpp"

///////////////////  内部静态变量的懒汉实现  //////////////////
//  //懒汉式是以时间换空间，适应于访问量较小时
//  在C++11内部静态变量的方式里是线程安全的，只创建了一次实例
class SqlSingle : public Sqlite3pp {

public:
  // 获取单实例对象
  static SqlSingle &GetInstance() {
    // 局部静态特性的方式实现单实例
    static SqlSingle signal;
    return signal;
  }

  ////////////////////////// 接口部分 //////////////////////////

  string sql_cmd(string sql) {
    std::string string_array = Sqlite3pp::db_sql(sql);
    return string_array;
  }

  string sql_drop(string table) {
    std::string string_array = Sqlite3pp::db_drop(table);
    return string_array;
  }

  string sql_delete(string table, string where) {
    std::string string_array = Sqlite3pp::db_delete(table, where);
    return string_array;
  }

  string sql_create(string table, string col_para) {
    std::string string_array = Sqlite3pp::db_create(table, col_para);
    return string_array;
  }

  string sql_update(string table, string set, string where) {
    std::string string_array = Sqlite3pp::db_update(table, set, where);
    return string_array;
  }

  string sql_insert(string table, string cols, string vals) {
    std::string string_array = Sqlite3pp::db_insert(table, cols, vals);
    return string_array;
  }

  string sql_select(string table, string colname, string where, string order,
                    string limit) {
    std::string string_array =
        Sqlite3pp::db_select(table, colname, where, order, limit);
    return string_array;
  }
  ////////////////////////// 接口部分 //////////////////////////

private:
  // 将其构造和析构成为私有的, 禁止外部构造和析构
  SqlSingle() : Sqlite3pp("/test.db"); // 使用绝对路径
  ~SqlSingle() = default;

  // 禁止外部复制构造
  SqlSingle(const SqlSingle &signal);

  // 禁止外部赋值操作
  const SqlSingle &operator=(const SqlSingle &signal);
};

///////////////////  内部静态变量的懒汉实现  //////////////////

#endif