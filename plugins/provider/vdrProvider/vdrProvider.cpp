/*
 * vdrProvider.cpp
 *
 *  Created on: 01.10.2012
 *      Author: savop
 */

#include <plugin.h>
#include <vdr/epg.h>
#include <vdr/channels.h>
#include <vdr/tools.h>
#include <vdr/config.h>
#include <string>
#include <sstream>
#include <tools.h>
#include <vdr/thread.h>
#include <iostream>
#include <pwd.h>
#include <unistd.h>

using namespace std;

namespace upnp {

class VdrProvider : public cUPnPResourceProvider {
private:
  time_t     lastModified;

  bool IsRootContainer(const string& uri){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("VdrProvider\tUri does not contain the root.");
      return false;
    } else {
      return true;
    }
  }

public:

  VdrProvider()
  : lastModified(0)
  {}

  virtual ~VdrProvider(){
    Cancel(2);
  }

  virtual string ProvidesSchema(){ return "vdr"; }

  virtual string GetRootContainer(){
    return ProvidesSchema() + "://";
  }

  virtual StringList GetContainerEntries(const string& uri){
    if(!IsRootContainer(uri)) return StringList();

    StringList list;

    // Check if this is the root:
    if(uri.compare(GetRootContainer()) == 0){
      cChannel* channel = NULL;
      for(int index = 0; (channel = Channels.Get(index)); index = Channels.GetNextNormal(index)){
        if(!channel->GroupSep()){
          list.push_back(*channel->GetChannelID().ToString());
        }
      }
    }

    return list;
  }

  virtual bool IsContainer(const string& uri){
    return uri.compare(GetRootContainer()) == 0;
  }

  virtual bool IsLink(const string& uri, string& target){
    // TODO: what are Channel::RefChannel or LinkChannels ?
    return false;
  }

  virtual long GetContainerUpdateId(const string& uri){
    if(IsRootContainer(uri)) return 0;

    // We have no containers. So just return the last modification date.
    // Containers like groups are about to come soon.
    return (long)lastModified;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(!IsRootContainer(uri)) return false;

    if(!cUPnPResourceProvider::GetMetadata(uri, metadata)) return false;

    metadata.SetProperty(cMetadata::Property(property::object::KEY_PARENTID, string("0")));
    metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string("VDR Live-TV")));
    metadata.SetProperty(cMetadata::Property("dlna:containerType", string("Tuner_1_0")));

    struct passwd *pw;
    if((pw = getpwuid(getuid())) == NULL){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, string("Klaus Schmidinger")));
    } else {
      string name(pw->pw_gecos); name = name.substr(0,name.find(",,,",0));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, name));
    }

    metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string("Watch Live-TV")));

    return true;
  }

  virtual string GetHTTPUri(const string& uri, const string& currentIP){
    if(!IsRootContainer(uri)) return string();

    int port = 3000;

    stringstream ss;

    ss << "http://" << currentIP << ":" << port
       << "/"
       << "EXT;"
       << "PROG=cat;"
       << "DLNA_contentFeatures=DLNA.ORG_PN=MPEG_TS_SD_EU_ISO+DLNA.ORG_OP=00+DLNA.ORG_CI=0+DLNA.ORG_FLAGS=ED100000000000000000000000000000"
       << "/"
       << uri.substr(6);

    return ss.str();
  }

  virtual void Action(){

    const cSchedules* Schedules;
    long now = 0;
    bool modified = false;
    while(Running()){
      now = time(NULL);

      if(!Channels.BeingEdited() && Channels.Modified() > 0){
        OnContainerUpdate(GetRootContainer(), now);
        modified = true;
      }

      { // Reduce Scope of Schedules lock.
        cSchedulesLock lock;
        Schedules = cSchedules::Schedules(lock);
        // iterate over all the schedules, find those, which were modified and tell
        // it to the media manager
        for(cSchedule* Schedule = Schedules->First(); Schedule; Schedule = Schedules->Next(Schedule))
        {
          if(Schedule->Modified() > lastModified && Schedule->PresentSeenWithin(30)){
            OnContainerUpdate(GetRootContainer(), now, *Schedule->ChannelID().ToString());
            modified = true;
          }
        }
      }

      if(modified){
        modified = false;
        lastModified = now;
      }

      sleep(2);
    }

  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(VdrProvider);

}  // namespace upnp

