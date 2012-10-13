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

public:

  RecProvider()
  {}

  virtual ~RecProvider(){
    Cancel(2);
  }

  virtual string ProvidesSchema(){ return "rec"; }

  virtual string GetRootContainer(){
    return ProvidesSchema() + "://";
  }

  virtual StringList GetContainerEntries(const string& uri){
    cerr << "Getting container entries for recordings in " << uri << endl;

    if(!IsRootContainer(uri)) return StringList();

    StringList list;

    stringstream filename; string recFilename, p;
    filename << VideoDirectory << "/" << uri.substr(6);
    for(cRecording* rec = Recordings.First(); rec; rec = Recordings.Next(rec)){
      recFilename = rec->FileName();
      if(recFilename.find(filename.str()) == 0){
        p = recFilename.substr(filename.str().length());
        if(p.find_first_of('/')){
          p = p.substr(0, p.find_first_of('/'));
          list.push_back(p);
        }
      }
    }

    return list;
  }

  virtual bool IsContainer(const string& uri){
    if(GetRootContainer().compare(uri) == 0) return true;

    if(!Recordings.GetByName(uri.substr(6).c_str())) return true;
    else return false;
  }

  virtual bool IsLink(const string& uri, string& target){
    return false;
  }

  virtual long GetContainerUpdateId(const string& uri){
    struct stat fileStat;
    if(GetFileStat(uri.substr(0,uri.find_last_of('/')), fileStat)){
      return std::max<time_t>(fileStat.st_ctim.tv_sec, fileStat.st_mtim.tv_sec);
    }

    return 0;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(GetRootContainer().compare(uri) == 0){
      if(!cUPnPResourceProvider::GetMetadata(uri, metadata)) return false;
      metadata.SetProperty(cMetadata::Property(property::object::KEY_PARENTID, string("0")));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string("VDR Recordings")));

      struct passwd *pw;
      if((pw = getpwuid(getuid())) == NULL){
        metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, string("Klaus Schmidinger")));
      } else {
        string name(pw->pw_gecos); name = name.substr(0,name.find(",,,",0));
        metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, name));
      }

      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string("Watch TV recordings")));

      return true;
    }

    return false;
  }

  virtual void Action(){
    while(Running()){
      int state = 0;
      if(Recordings.NeedsUpdate() || Recordings.StateChanged(state)){
        OnContainerUpdate(GetRootContainer(), GetContainerUpdateId(GetRootContainer()));
      }
      sleep(5);
    }
  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(RecProvider);

}  // namespace upnp

