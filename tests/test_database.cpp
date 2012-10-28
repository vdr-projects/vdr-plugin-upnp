/*
 * test_database.cpp
 *
 *  Created on: 28.10.2012
 *      Author: savop
 */

#include <iostream>
#include <sstream>
#include "../include/db/connection.h"

int main(){

  try {
    db::Connection connection = db::Connect("/tmp/test_db.sqlite");

    connection.BeginTransaction();

    std::stringstream ss;

    ss << "CREATE TABLE details"
       << "("
       << "  `propertyID` INTEGER PRIMARY KEY,"
       << "  `@id` TEXT "
       << "  REFERENCES metadata (`@id`) ON DELETE CASCADE ON UPDATE CASCADE,"
       << "  `property`   TEXT,"
       << "  `value`      TEXT"
       << ")";

    db::Statement statement = connection.Prepare(ss.str());

    statement.Execute();

    connection.CommitTransaction();

  } catch (const db::SQLiteException& e){
    std::cerr << "Exception: "<< e.what() << std::endl;
  }

  return 0;
}
