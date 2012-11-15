/*
 * fileProvider.cpp
 *
 *  Created on: 15.11.2012
 *      Author: savop
 */

#include <plugin.h>

using namespace std;

namespace upnp {

class FileProvider : public cUPnPResourceProvider {
private:

  StringMap directoryMap;
  FILE* fileFD;

  string GetFile(const string& uri){
    string mountPoint = uri.substr(6, uri.find_first_of('/',6) - 7);

    string file;
    if(!mountPoint.empty() && !directoryMap[mountPoint].empty()){
      file = directoryMap[mountPoint] + uri.substr(uri.find_first_of('/',6));
    }

    return file;
  }

public:

  virtual string ProvidesSchema() { return "file"; }

  virtual string GetRootContainer() {
    return ProvidesSchema() + "://";
  }

  virtual StringList GetContainerEntries(const string& uri) {
    StringList list;

    return list;
  }

  virtual bool IsContainer(const string& uri) {
    return false;
  }

  virtual bool IsLink(const string& uri, string& target) {
    return false;
  }

  virtual long GetContainerUpdateId(const string& uri) {
    return 0;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata) {
    return false;
  }

  virtual bool Seekable() const {
    return true;
  }

  virtual bool Open(const string& uri) {
    if(fileFD)
      Close();

    string file = GetFile(uri);

    fileFD = fopen(file.c_str(), "r");

    return (fileFD) ? true : false;
  }

  virtual size_t Read(char* buf, size_t bufLen) {
    if(!fileFD) return 0;

    return fread(buf, 1, bufLen, fileFD);
  }

  virtual bool Seek(size_t offset, int origin) {
    if(!fileFD) return 0;

    return fseek(fileFD, offset, origin) == 0;
  }

  virtual void Close() {
    if(fileFD){
      fclose(fileFD);
      fileFD = NULL;
    }
  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(FileProvider);

}

