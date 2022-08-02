
/**
 * @file sqlite3pp.hpp
 * @version v1.0
 * @author LZH
 * @date 2022-06-30 16:39:41
 * @brief  数据库接口文件
 * @details 自定义Sqlite3pp类，封装sqlite3常用的接口，无需手动操作指针来开关文件，在类析构会自动关闭文件。
 */

#pragma once

#ifndef __SQLITE3PP_HPP__
#define __SQLITE3PP_HPP__

#include <assert.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <memory>
// #include <stdexcept>
#include <string>
#include "json.hpp"

using std::string;

static nlohmann::json j_cfg;
static sqlite3 *database = nullptr;

class Sqlite3pp {
 private:
  static int callback(void *data, int argc, char **argv, char **col_name) {
    int i;
    j_cfg.clear();

    for (i = 0; i < argc; i++) {
      if (col_name[i][0] == 'i') {
        j_cfg.emplace(col_name[i], argv[i]);
      } else {
        argv[i] ? argv[i] : "";

        j_cfg.emplace(col_name[i], argv[i]);
      }
    }
    return 0;
  }

  bool is_exists(const std::string &filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
  }

 public:
  Sqlite3pp(string file) {
    bool file_exists = Sqlite3pp::is_exists(file);
    if (file_exists) {
      int rc;
      rc = sqlite3_open(file.c_str(), &database);
      assert(!rc);
    } else {
      string error = file + " does not exist";
      throw std::invalid_argument(error);
    }
  }

  ~Sqlite3pp() {
    if (database) sqlite3_close(database);

    database = nullptr;
  }
  string db_sql(string sql) {
    int rc;
    char *error_msg = 0;
    nlohmann::json j_object;
    j_object.clear();
    string ErrorMsg = "";
    rc = sqlite3_exec(database, sql.c_str(), callback, 0, &error_msg);
    j_object.emplace("Return", rc);

    if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", error_msg);
      ErrorMsg = error_msg;

      sqlite3_free(error_msg);
    } else {
      // fprintf(stdout, "Records created successfully\n");
    }

    j_object.emplace("ErrorMsg", ErrorMsg);
    j_object.emplace("Data", j_cfg);

    string ret = j_object.dump();
    return ret;
  }
  string db_drop(string table);
  string db_delete(string table, string where) {
    string sql_cmd;
    if (!where.empty()) {
      sql_cmd = "DELETE from " + table + " WHERE " + where + ";";
    } else {
      sql_cmd = "DELETE from " + table + ";";
    }

    string ret = db_sql(sql_cmd.c_str());

    return ret;
  }
  string db_create(string table, string col_para) {
    string sql_cmd;

    sql_cmd = "CREATE TABLE " + table + "(" + col_para + ");";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
  }
  string db_update(string table, string set, string where) {
    string sql_cmd;

    sql_cmd = "UPDATE " + table + " SET " + set + " WHERE " + ";";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
  }
  string db_insert(string table, string cols, string vals) {
    string sql_cmd;

    sql_cmd = "INSERT INTO " + table + " (" + cols + ") VALUES (" + vals + " );";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
  }
  string db_select(string table, string colname, string where, string order, string limit) {
    string sql_cmd;
    string tmp;
    string end = ";";
    if (!colname.empty()) {
      sql_cmd = "SELECT" + colname + "FROM " + table;
    } else {
      sql_cmd = "SELECT * FROM " + table;
    }

    if (!where.empty()) {
      tmp = sql_cmd;
      sql_cmd += tmp + " WHERE " + where;
    }

    if (!order.empty()) {
      tmp = sql_cmd;
      sql_cmd += tmp + "  ORDER BY " + order;
    }

    if (!limit.empty()) {
      tmp = sql_cmd;
      sql_cmd += tmp + "  LIMIT " + order;
    }
    sql_cmd += end;
    string ret = db_sql(sql_cmd);

    return ret;
  }
};
#endif
