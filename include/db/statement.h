/*
 * statement.h
 *
 *  Created on: 28.10.2012
 *      Author: savop
 */

#ifndef STATEMENT_H_
#define STATEMENT_H_

#include <sqlite3.h>

namespace db {

class Statement {
private:

  boost::shared_ptr<sqlite3_stmt> m_statement;
  boost::shared_ptr<sqlite3> m_connection;

public:

  Statement() {}

  Statement(boost::shared_ptr<sqlite3>& conn, boost::shared_ptr<sqlite3_stmt>& stmt)
  : m_connection(conn)
  , m_statement(stmt)
  {}

  virtual ~Statement(){}

  void Reset(){
    if(sqlite3_reset(m_statement.get()) != SQLITE_OK)
      throw SQLiteException(sqlite3_errmsg(m_connection.get()));
  }

  int Execute(){
    int ret = sqlite3_step(m_statement.get());

    if(ret != SQLITE_OK && ret != SQLITE_DONE )
      throw SQLiteException(sqlite3_errmsg(m_connection.get()));

    int numChanges = sqlite3_changes(m_connection.get());

    Reset();

    return numChanges;
  }

  int GetBindIndex(const std::string& col){
    int index = 0;

    return index;
  }

  Statement& SetNull(const std::string& c){
    int col = GetBindIndex(c);

    return *this;
  }

  Statement& SetString(const std::string& c, const std::string& value){
    int col = GetBindIndex(c);

    return *this;
  }

  Statement& SetLong(const std::string& c, long value){
    int col = GetBindIndex(c);

    return *this;
  }

  Statement& SetDouble(const std::string& c, double value){
    int col = GetBindIndex(c);

    return *this;
  }

  Statement& SetBlob(const std::string& c, const char* buf, size_t bufLen){
    int col = GetBindIndex(c);

    return *this;
  }

};

}  // namespace db

#endif /* STATEMENT_H_ */
