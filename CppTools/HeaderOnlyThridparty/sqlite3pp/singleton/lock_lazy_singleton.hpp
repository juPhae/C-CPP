
/*******************************************************************************
 *FileName: lazy_singleton.hpp
 *version:
 *Author: Phae
 *Date: 2022-10-25 15:07:48
 *Description:
 *LastEditors: Phae
 *LastEditTime: 2022-10-25 15:08:48
 ********************************************************************************/
#pragma once

#ifndef __LOCK_LAZY_SINGLETON_HPP__
#define __LOCK_LAZY_SINGLETON_HPP__

#include "../sqlite3pp.hpp"

#include <mutex> // std::mutex

///////////////////  加锁的懒汉式实现  //////////////////
//懒汉式是以时间换空间，适应于访问量较小时

class SqlSingleInstance : public Sqlite3pp {

public:
  // 获取单实例对象
  static SqlSingleInstance *&GetInstance() {

    //  这里使用了两个
    //  if判断语句的技术称为双检锁；好处是，只有判断指针为空的时候才加锁，
    //  避免每次调用 GetInstance的方法都加锁，锁的开销毕竟还是有点大的。
    if (m_SqlSingleInstance == nullptr) {
      std::unique_lock<std::mutex> lock(m_Mutex); // 加锁
      if (m_SqlSingleInstance == nullptr) {
        m_SqlSingleInstance = new (std::nothrow) SqlSingleInstance;
      }
    }

    return m_SqlSingleInstance;
  }

  //释放单实例，进程退出时调用
  static void deleteInstance() {
    std::unique_lock<std::mutex> lock(m_Mutex); // 加锁
    if (m_SqlSingleInstance) {
      delete m_SqlSingleInstance;
      m_SqlSingleInstance = nullptr;
    }
  }

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
  SqlSingleInstance() : Sqlite3pp("/test.db"); // 使用绝对路径
  ~SqlSingleInstance() = default;

  // 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
  SqlSingleInstance(const SqlSingleInstance &signal);
  const SqlSingleInstance &operator=(const SqlSingleInstance &signal);

private:
  // 唯一单实例对象指针
  static SqlSingleInstance *m_SqlSingleInstance = nullptr;
  static std::mutex m_Mutex;
};

///////////////////  加锁的懒汉式实现  //////////////////
#endif //__LOCK_LAZY_SINGLETON_HPP__