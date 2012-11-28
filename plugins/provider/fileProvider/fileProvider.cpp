/*
 * fileProvider.cpp
 *
 *  Created on: 15.11.2012
 *      Author: savop
 */

#include <plugin.h>
#include <fstream>
#include <sstream>
#include <tools/string.h>
#include <vdr/plugin.h>
#include <pwd.h>
#include <unistd.h>

using namespace std;

namespace upnp {

class FileProvider : public cUPnPResourceProvider {
private:

  StringMap directoryMap;
  FILE* fileFD;

  bool IsRootContainer(const string& uri){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("RecProvider\tUri does not contain the root.");
      return false;
    } else {
      return true;
    }
  }

  bool Load(const string& filename)
  {
    if (access(filename.c_str(), F_OK) == 0) {
      isyslog("loading %s", filename.c_str());
      ifstream file;
      file.open(filename.c_str(), ifstream::in);
      if(!file.is_open())
        return false;
      string line; int pos;
      while(getline(file, line)){
        if(line.length() > 0 && line[0] != '#'){
          if((pos = line.find_first_of(':')) != string::npos){
            directoryMap[tools::Trim(line.substr(0,pos))] = tools::Trim(line.substr(pos+1));
          }
        }
      }
      return true;
    }
    return false;
  }

  string GetFile(const string& uri){
    string mountPoint = uri.substr(7, uri.find_first_of('/',7) - 7);

    string file;
    if(!mountPoint.empty() && !directoryMap[mountPoint].empty()){
      file = directoryMap[mountPoint];
      if(uri.find_first_of('/', 7) != string::npos)
        file += uri.substr(uri.find_first_of('/',7));
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

  FileProvider()
  : fileFD(NULL)
  {
    stringstream file;
    file << cPlugin::ConfigDirectory(PLUGIN_NAME_I18N) << "/directories.conf";
    Load(file.str());
  }

  virtual string ProvidesSchema() { return "file"; }

  virtual string GetRootContainer() {
    return ProvidesSchema() + "://";
  }

  virtual bool IsContainer(const string& uri) {
    struct stat fileStat;
    if(GetRootContainer().compare(uri) == 0 || (GetFileStat(uri, fileStat) && S_ISDIR(fileStat.st_mode)))
      return true;

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

    if(GetRootContainer().compare(uri) == 0){
      for( StringMap::iterator it = directoryMap.begin(); it != directoryMap.end(); ++it ) {
        list.push_back( it->first + "/" );
      }
    } else {
      if((dirHandle = opendir(GetFile(uri).c_str())) == NULL){
        return list;
      }

      string filename;
      while ((dirEntry = readdir(dirHandle)) != NULL) {
        filename = dirEntry->d_name;
        if( filename.compare(".") != 0 && filename.compare("..") != 0 ) {
          if(dirEntry->d_type == DT_DIR) {
            filename += "/";
          }
          list.push_back(filename);
        }
      }
      closedir(dirHandle);
    }

    return list;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(!IsRootContainer(uri)) return false;

    if(!cUPnPResourceProvider::GetMetadata(uri, metadata)) return false;

    if(GetRootContainer().compare(uri) == 0){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string("File system")));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string("Access files on the file system")));
    }

    struct passwd *pw;
    if((pw = getpwuid(getuid())) == NULL){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, string("Klaus Schmidinger")));
    } else {
      string name(pw->pw_gecos); name = name.substr(0,name.find(",,,",0));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, name));
    }

    return true;
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

