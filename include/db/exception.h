/*
 * exception.h
 *
 *  Created on: 28.10.2012
 *      Author: savop
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <string>

namespace db {

class SQLiteException : public std::exception {
public:
  SQLiteException(std::string msg) throw()
  : message(msg)
  {}
  virtual ~SQLiteException() throw() {}
  virtual const char* what() const throw()
  {
    return message.c_str();
  }
private:
  std::string message;
};

}  // namespace db


#endif /* EXCEPTION_H_ */
