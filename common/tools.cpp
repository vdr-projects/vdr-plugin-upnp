/*
 * tools.cpp
 *
 *  Created on: 05.08.2012
 *      Author: savop
 */

#include "../include/tools.h"
#include <sstream>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;

#ifdef DEBUG

#define MESSAGE_SIZE 256
namespace upnp {
  void log_debug_msg(const char* file, const char* func, int line, int level, const char* msg, ...){
    char* lvlenv = getenv("UPNP_DEBUG_LEVEL");
    int debug_level = lvlenv ? atoi(lvlenv) : 0;
    if(level && debug_level >= level){
      va_list ap;
      char message[MESSAGE_SIZE];

      snprintf(message, sizeof(message), "[%s:%d] %s - %s", file, line, func, msg);
      va_start(ap, msg);
      vsyslog(LOG_NOTICE, message, ap);
      va_end(ap);
    }
  }
}
#endif

namespace upnp {

namespace tools {

string GetAddressByInterface(string Interface){
  string address;

  if(!Interface.empty()){
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, Interface.c_str(), IFNAMSIZ-1);
    int ret = ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    if(ret==0){
      sockaddr_in* inAddr = (sockaddr_in*)&ifr.ifr_addr;
      address = inet_ntoa(inAddr->sin_addr);
    }
  }

  return address;
}
string GetNetworkInterfaceByIndex(int Index, bool skipLoop){
  return GetNetworkInterfaces(skipLoop)[Index];
}

StringVector GetNetworkInterfaces(bool skipLoop){
  StringVector interfaces;

  int fd;
  struct ifconf ifc;
  struct ifreq ifr[10];
  int nifaces, i;

  memset(&ifc,0,sizeof(ifc));
  ifc.ifc_buf = (char*) (ifr);
  ifc.ifc_len = sizeof(ifr);

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  int ret = ioctl(fd, SIOCGIFCONF, &ifc);
  close(fd);
  if(ret==0){
    nifaces = ifc.ifc_len/sizeof(struct ifreq);
    for(i = 0; i < nifaces; i++){
      if(skipLoop && strcmp("lo", ifr[i].ifr_name)==0)
        continue;
      else
        interfaces.push_back(ifr[i].ifr_name);
    }
  }

  return interfaces;
}

string Trim(const std::string& str, const std::string& whitespace)
{
  const unsigned long int strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return ""; // no content

  const unsigned long int strEnd = str.find_last_not_of(whitespace);
  const unsigned long int strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

string ToString(long number){
  stringstream ss;
  ss << number;
  return ss.str();
}

string StringListToCSV(StringList list){
  stringstream ss;

  if(list.empty()) return string();

  StringList::iterator it = list.begin();

  ss << (*it);
  for(++it; it != list.end(); ++it){
    ss << "," << (*it);
  }

  return ss.str();
}

string IdListToCSV(IdList list){
  stringstream ss;

  if(list.empty()) return string();

  IdList::iterator it = list.begin();

  ss << (*it).first << "," << (*it).second;
  for(++it; it != list.end(); ++it){
    ss << "," << (*it).first << "," << (*it).second;
  }

  return ss.str();
}

string GenerateUUIDFromURL(string url){
  boost::uuids::string_generator gen;
  boost::uuids::uuid urlNamespace = gen(L"6ba7b811-9dad-11d1-80b4-00c04fd430c8");
  stringstream uuid;

  uuid << boost::uuids::name_generator(urlNamespace)(url);

  return uuid.str();
}

string GenerateUUIDRandomly(){
  stringstream uuid;

  uuid << boost::uuids::random_generator()();

  return uuid.str();
}

void StringExplode(string str, string separator, StringVector& results) {
  string::size_type found;
  found = str.find_first_of(separator);
  while(found != string::npos){
      if(found > 0){
          results.push_back(str.substr(0,found));
      }
      str = str.substr(found+1);
      found = str.find_first_of(separator);
  }
  if(str.length() > 0){
      results.push_back(str);
  }
}

}  // namespace tools

}  // namespace upnp

