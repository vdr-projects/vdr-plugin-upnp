/*
 * connection.h
 *
 *  Created on: 28.10.2012
 *      Author: savop
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <sqlite3.h>

#include "exception.h"
#include "statement.h"

namespace db {

class Connection {

private:

  boost::shared_ptr<sqlite3> m_connection;

public:

  Connection() {}

  Connection(boost::shared_ptr<sqlite3>& conn)
  : m_connection(conn)
  {}

  virtual ~Connection(){}

  void Close(){
    m_connection.reset();
  }

  Statement Prepare(const std::string& stmt){
    sqlite3_stmt* _stmt;
    if(sqlite3_prepare(m_connection.get(), stmt.c_str(), stmt.length(), &_stmt, NULL) != SQLITE_OK)
      throw SQLiteException(sqlite3_errmsg(m_connection.get()));

    boost::shared_ptr<sqlite3_stmt> statement(_stmt, std::ptr_fun(sqlite3_finalize));
    return Statement(m_connection, statement);
  }

  void BeginTransaction(){
    Prepare("BEGIN TRANSACTION").execute();
  }

  void CommitTransaction(){
    Prepare("COMMIT TRANSACTION").execute();
  }

  void RollbackTransaction(){
    Prepare("ROLLBACK TRANSACTION").execute();
  }

};

Connection Connect(const std::string url){
  sqlite3* _conn;
  if(sqlite3_open_v2(url.c_str(), &_conn, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK){
    sqlite3_close(_conn);
    throw SQLiteException(std::string("Failed to open ") + url);
  }
  boost::shared_ptr<sqlite3> connection(_conn, std::ptr_fun(sqlite3_close));

  return Connection(connection);
}

}  // namespace db


#endif /* CONNECTION_H_ */
