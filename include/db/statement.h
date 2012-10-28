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

  Statement& execute(){
    if(sqlite3_step(m_statement.get()) != SQLITE_OK)
      throw SQLiteException(sqlite3_errmsg(m_connection.get()));

    return *this;
  }

};

}  // namespace db

#endif /* STATEMENT_H_ */
