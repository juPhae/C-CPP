
/*******************************************************************************
 *FileName: sqlite3pp.hpp
 *version:
 *Author: Phae
 *Date: 2022-06-30 19:39:41
 *Description:
 *
 *     Rely on JSON library files :   https://github.com/nlohmann/json
 *
 *LastEditors: Phae
 *LastEditTime: 2022-06-30 19:59:12
 ********************************************************************************/

#ifndef __SQLITE3PP_HPP__
#define __SQLITE3PP_HPP__

#include <assert.h>
// #include <glib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>
// #include <sstream>
#include <string>

#include "dbg.hpp"
#include "json.hpp"

using std::string;
// using std::stringstream;

static sqlite3 *database = nullptr;

class Sqlite3pp {
   private:
    static int callback(void *data, int argc, char **argv, char **col_name);

   public:
    Sqlite3pp(string file);
    ~Sqlite3pp();
    string db_sql(string sql);
    string db_drop(string table);
    string db_delete(string table, string where);
    string db_create(string table, string col_para);
    string db_insert(string table, string cols, string vals);
    string db_update(string table, string set, string where);
    string db_select(string table, string colname, string where, string order, string limit);
};

Sqlite3pp::Sqlite3pp(string file) {
    int rc;
    rc = sqlite3_open(file.c_str(), &database);
    dbg(rc);
    assert(!rc);
}

Sqlite3pp::~Sqlite3pp() {
    if (database) sqlite3_close(database);

    database = nullptr;
}

int Sqlite3pp::callback(void *data, int argc, char **argv, char **col_name) {
    int i;
    const char *j_array = (const char *)data;
    const char **j_cfg;
    dbg(j_array);
    for (i = 0; i < argc; i++) {
        if (col_name[i][0] == 'i') {
            dbg(col_name[i]);
            dbg((argv[i]));
        } else {
            argv[i] ? argv[i] : "";
            dbg(col_name[i]);
            dbg((argv[i]));
        }
    }
    return 0;
}

string Sqlite3pp::db_sql(string sql) {
    int rc;
    nlohmann::json j_cfg;
    char *error_msg = 0;

    const char *j_array;

    rc = sqlite3_exec(database, sql.c_str(), callback, (void *)j_array, &error_msg);
    dbg(rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
    } else {
        fprintf(stdout, "Records created successfully\n");
    }

    string ret = "";
    return ret;
}

string Sqlite3pp::db_select(string table, string colname, string where, string order, string limit) {
    string sql_cmd;
    string tmp;
    string end = ";";
    if (!colname.empty())
        sql_cmd = "SELECT" + colname + "FROM " + table;

    else

        sql_cmd = "SELECT * FROM " + table;

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
    dbg(sql_cmd);

    string ret = db_sql(sql_cmd);

    return ret;
}

string Sqlite3pp::db_drop(string table) {
    string sql_cmd = "DROP TABLE " + table + ";";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
}

string Sqlite3pp::db_delete(string table, string where) {
    string sql_cmd;
    if (!where.empty())

        sql_cmd = "DELETE from " + table + " WHERE " + where + ";";
    else

        sql_cmd = "DELETE from " + table + ";";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
}

string Sqlite3pp::db_update(string table, string set, string where) {
    string sql_cmd;

    sql_cmd = "UPDATE " + table + " SET " + set + " WHERE " + ";";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
}
string Sqlite3pp::db_insert(string table, string cols, string vals) {
    string sql_cmd;

    sql_cmd = "INSERT INTO " + table + " (" + cols + ") VALUES (" + vals + " );";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
}

string Sqlite3pp::db_create(string table, string col_para) {
    string sql_cmd;

    sql_cmd = "CREATE TABLE " + table + "(" + col_para + ");";
    string ret = db_sql(sql_cmd.c_str());

    return ret;
}

#endif  //__SQLITE3PP_HPP__