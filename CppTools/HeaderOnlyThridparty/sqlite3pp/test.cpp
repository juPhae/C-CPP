/*******************************************************************************
 *FileName: test.cpp
 *version:
 *Author: Phae
 *Date: 2022-06-30 20:30:10
 *Description:
 *LastEditors: Phae
 *LastEditTime: 2022-06-30 23:53:11
 ********************************************************************************/
#include "dbg.hpp"
#include "sqlite3pp.hpp"
int main(int argc, char** argv) {
  string path = "./test.db";
  Sqlite3pp db(path.c_str());
  string select_ = db.db_select("user", "", "", "", "");
  string cmd = "SELECT * FROM sqlite_master WHERE type='table' ";
  string sql_ = db.db_sql(cmd);

  // db.db_create("test", "id,name");
  // db.db_insert("test", "id", "1");
  dbg(select_);
  dbg(sql_);
  return 0;
}