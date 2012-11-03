/*
 * tools.h
 *
 *  Created on: 03.08.2012
 *      Author: savop
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <vector>
#include <string>
#include <iostream>
#include <string.h>
#include <vdr/tools.h>
#include <map>
#include <list>

using namespace std;

#define KB(s)         (s * 1024)
#define MB(s)         (s * 1024 * 1024)

#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))

#define CRLF          "\r\n"

#define NL            "\n"

#define MAX_METADATA_LENGTH_L     1024
#define MAX_METADATA_LENGTH_S     256

namespace upnp {

  typedef std::list<std::string> StringList;
  typedef std::vector<std::string> StringVector;
  typedef std::map<std::string, std::string> StringMap;
  typedef std::map<std::string, uint32_t> IdList;

}

#include "tools/error.h"
#include "tools/ixml.h"
#include "tools/net.h"
#include "tools/string.h"
#include "tools/uuid.h"

#endif /* TOOLS_H_ */
