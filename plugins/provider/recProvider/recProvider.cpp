/*
 * vdrProvider.cpp
 *
 *  Created on: 01.10.2012
 *      Author: savop
 */

#include <plugin.h>
#include <vdr/recording.h>
#include <vdr/channels.h>
#include <vdr/tools.h>
#include <vdr/config.h>
#include <vdr/videodir.h>
#include <string>
#include <sstream>
#include <tools.h>
#include <pwd.h>
#include <unistd.h>

using namespace std;

namespace upnp {

class RecProvider : public cUPnPResourceProvider {
private:

  bool IsRootContainer(const string& uri){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("RecProvider\tUri does not contain the root.");
      return false;
    } else {
      return true;
    }
  }

  bool GetFileStat(const string& uri, struct stat& fileStat){
    stringstream filename;
    filename << VideoDirectory << "/" << uri.substr(6);
    struct stat s;
    if(stat(filename.str().c_str(), &s) == 0){
      fileStat = s;
      return true;
    }

    return false;
  }

  FILE* fileFD;
  int currentFileNumber;
  int lastFileNumber;
  string filename;
  vector<size_t> offsets;

  void CloseFile(){
    fclose(fileFD);
    fileFD = NULL;
  }

  bool OpenFile(int i = 1){
    if(currentFileNumber != i && fileFD){
      CloseFile();
    }

    char fileBuf[1024]; strncpy(fileBuf, filename.c_str(), 1024);
    char* pFileNumber = fileBuf + strlen(fileBuf);
    sprintf(pFileNumber, "/%05d.ts", i);

    fileFD = fopen(fileBuf, "r");
    if(!fileFD) return false;
    currentFileNumber = i;

    return true;
  }

  bool OpenNext(){
    return OpenFile(currentFileNumber+1);
  }

  bool ScanFiles(){
    if(!OpenFile(1)) return false;
    do {
      fseek(fileFD, 0, SEEK_END);
      offsets[currentFileNumber-1] = ftell(fileFD);
    } while(OpenNext());
    lastFileNumber = currentFileNumber;
    return OpenFile(1);
  }

public:

  RecProvider()
  : fileFD(NULL)
  , currentFileNumber(1)
  , lastFileNumber(1)
  , offsets(65536)
  {
  }

  virtual ~RecProvider(){
    Cancel(2);
  }

  virtual string ProvidesSchema(){ return "rec"; }

  virtual string GetRootContainer(){
    return ProvidesSchema() + "://";
  }

  virtual StringList GetContainerEntries(const string& u){
    if(!IsRootContainer(u)) return StringList();

    StringList list;
    string videoDir(VideoDirectory), fs, uri = u.substr(6);
    int pos = 0, vl = videoDir.length(), ul = uri.length(), vul = vl + ul + 1;

    for(cRecording* rec = Recordings.First(); rec; rec = Recordings.Next(rec)){
      char* file = strdup(rec->Name());
      file = ExchangeChars(file, true);
      fs = file;
      free(file);
      if(fs.find(uri) != string::npos){
        fs = fs.substr(ul);
        if((pos = fs.find_first_of('/')) != string::npos){
          fs = fs.substr(0, pos+1);
        } else {
          fs = string(rec->FileName()).substr(vul);
        }
        list.push_back(fs);
      }
    }

    return list;
  }

  virtual bool IsContainer(const string& uri){
    if(GetRootContainer().compare(uri) == 0) return true;

    stringstream filename;
    filename << VideoDirectory << "/" << uri.substr(6);

    if(!Recordings.GetByName(filename.str().c_str())){
      return true;
    }
    else {
      return false;
    }
  }

  virtual bool IsLink(const string& uri, string& target){
    return false;
  }

  virtual long GetContainerUpdateId(const string& uri){
    struct stat fileStat;
    if(GetFileStat(uri.substr(0,uri.find_last_of('/')+1), fileStat)){
      return std::max<time_t>(fileStat.st_ctim.tv_sec, fileStat.st_mtim.tv_sec);
    }

    return 0;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(!IsRootContainer(uri)) return false;

    if(!cUPnPResourceProvider::GetMetadata(uri, metadata)) return false;

    if(GetRootContainer().compare(uri) == 0){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string("VDR Recordings")));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string("Watch TV recordings")));

    } else {
      int ul = uri.length();
      string folder = uri.substr(uri.find_last_of('/', ul-2)+1, ul - 7);
      char * str = strdup(folder.c_str());
      str = ExchangeChars(str, false);
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(str)));
      free(str);
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

  virtual bool Open(const string& uri){
    filename = string(VideoDirectory) + "/" + uri.substr(6);
    currentFileNumber = 1;
    return ScanFiles();
  }

  virtual size_t Read(char* buf, size_t bufLen){
    if(!fileFD) return -1;

    size_t bytesRead = 0;

    bytesRead += fread(buf, 1, bufLen, fileFD);
    if(bytesRead != bufLen && OpenNext()){
      bytesRead += fread(buf + bytesRead, 1, bufLen - bytesRead, fileFD);
    }

    return bytesRead;
  }

  virtual bool Seek(size_t offset, int origin){
    if(!fileFD) return false;

    off_t curpos = fseek(fileFD, 0, SEEK_CUR); // this should not change anything
    switch(origin){
    case SEEK_END:
        offset = offsets[lastFileNumber] + offset;
        break;
    case SEEK_CUR:
        offset = offsets[currentFileNumber-1] + curpos +  offset;
        break;
    case SEEK_SET:
        // Nothing to change
        break;
    default:
      return false;
    }

    if(!OpenFile(1)) return false;
    size_t size = 0;
    while(true){
      fseek(fileFD, 0, SEEK_END);
      size = ftell(fileFD);
      if(offset < size){
        return fseek(fileFD, offset, SEEK_SET) == 0;
      } else {
        offset -= size;
        if(OpenNext()) return false;
      }
    }

    return false;

  }

  virtual void Close(){
    currentFileNumber = 1;
    CloseFile();
  }

  virtual void Action(){
    while(Running()){
      int state = 0;
      if(Recordings.NeedsUpdate() || Recordings.StateChanged(state)){
        OnContainerUpdate(GetRootContainer(), GetContainerUpdateId(GetRootContainer()));
      }
      sleep(100);
    }
  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(RecProvider);

}  // namespace upnp

