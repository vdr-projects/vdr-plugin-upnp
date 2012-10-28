/*
 * test_database.cpp
 *
 *  Created on: 28.10.2012
 *      Author: savop
 */

#include <iostream>
#include "../include/db/connection.h"

int main(){

  try {
    db::Connection connection = db::Connect("/tmp/test_db.sqlite");

    connection.BeginTransaction();

    connection.RollbackTransaction();

  } catch (const db::SQLiteException& e){
    std::cerr << "Exception: "<< e.what() << std::endl;
  }

  return 0;
}
