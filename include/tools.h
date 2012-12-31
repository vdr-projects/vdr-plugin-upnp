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

#define _unused(x) ((void)x)

#ifdef DEBUG

#if __STDC_VERSION__ < 199901L
  #if __GNUC__ >= 2
  #define __func__ __PRETTY_FUNCTION__
  #else
  #define __func__ "<unknown>"
  #endif
#endif

#define LOG(level, msg...) upnp::log_debug_msg(__FILE__, __func__, __LINE__, level, msg);

namespace upnp {
  void log_debug_msg(const char* file, const char* func, int line, int level, const char* msg, ...) __attribute__ ((format (printf, 5, 6)));
}
#else
#define LOG(level, msg...)
#endif

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
