/*
 * string.h
 *
 *  Created on: 03.10.2012
 *      Author: savop
 */

#ifndef STRING_H_
#define STRING_H_

#include <string>
#include "../tools.h"

using namespace std;

namespace upnp {

namespace tools {

string ToString(long number);
string StringListToCSV(StringList list);
string IdListToCSV(IdList list);
void StringExplode(string str, string separator, StringVector& results);
string Trim(const std::string& str, const std::string& whitespace = " \t");

}

}  // namespace upnp

#endif /* STRING_H_ */
