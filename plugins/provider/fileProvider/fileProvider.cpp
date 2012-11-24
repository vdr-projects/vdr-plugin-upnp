/*
 * fileProvider.cpp
 *
 *  Created on: 15.11.2012
 *      Author: savop
 */

#include <plugin.h>
#include <fstream>

using namespace std;

namespace upnp {

class FileProvider : public cUPnPResourceProvider {
private:

  StringMap directoryMap;
  FILE* fileFD;

  bool Load(const string& filename)
  {
    bool result = false;
    if (access(filename.c_str(), F_OK) == 0) {
      isyslog("loading %s", filename.c_str());
      ifstream file;
      file.open(filename.c_str(), ifstream::in);
      string line; int pos;
      while(getline(file, line)){
        if(line.length() > 0 && line[0] != '#'){
          if((pos = line.find_first_of(':')) != string::npos){
            directoryMap[line.substr(0,pos)] = line.substr(pos+1);
          }
        }
      }
    }
    return result;
  }

  string GetFile(const string& uri){
    string mountPoint = uri.substr(6, uri.find_first_of('/',6) - 7);

    string file;
    if(!mountPoint.empty() && !directoryMap[mountPoint].empty()){
      file = directoryMap[mountPoint] + uri.substr(uri.find_first_of('/',6));
    }

    return file;
  }

  bool GetFileStat(const string& uri, struct stat& fileStat){
    struct stat s;
    if(stat(GetFile(uri).c_str(), &s) == 0){
      fileStat = s;
      return true;
    }

    return false;
  }

public:

  virtual string ProvidesSchema() { return "file"; }

  virtual string GetRootContainer() {
    return ProvidesSchema() + "://";
  }

  virtual bool IsContainer(const string& uri) {
    struct stat fileStat;
    if(GetFileStat(uri, fileStat) && S_ISDIR(fileStat.st_mode)) return true;

    return false;
  }

  virtual bool IsLink(const string& uri, string& target) {
    struct stat fileStat;
    if(GetFileStat(uri, fileStat) && S_ISLNK(fileStat.st_mode)) return true;

    return false;
  }

  virtual long GetContainerUpdateId(const string& uri) {
    struct stat fileStat;
    if(GetFileStat(uri, fileStat)){
      return std::max<time_t>(fileStat.st_ctim.tv_sec, fileStat.st_mtim.tv_sec);
    }

    return 0;
  }

  virtual StringList GetContainerEntries(const string& uri) {
    StringList list;

    DIR* dirHandle;
    struct dirent* dirEntry;

    if((dirHandle = opendir(GetFile(uri).c_str())) == NULL){
      return list;
    }

    string filename;
    while ((dirEntry = readdir(dirHandle)) != NULL) {
      filename = dirEntry->d_name;
      if(filename.compare(".") || filename.compare("..")){
        list.push_back(filename);
      }
    }
    closedir(dirHandle);

    return list;
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

