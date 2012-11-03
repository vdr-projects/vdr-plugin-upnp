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
#include <string>
#include <sstream>
#include <algorithm>
#include <tools.h>
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

  int GetGroupByName(string name)
  {
    if(name.empty()) return -1;
    int Idx = -1;
    cChannel *channel = Channels.Get(++Idx);
    while (channel && !(channel->GroupSep() && name.compare(channel->Name()) == 0))
      channel = Channels.Get(++Idx);
    return channel ? Idx : -1;
  }

  string GetContainerName(string uri){
    return uri.substr(6,uri.size()-7);
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
    int index;
    cChannel* channel = NULL;
    // Check if this is the root and we have no groups in the channels.conf:
    if(uri.compare(GetRootContainer()) == 0 && Channels.GetNextGroup(0) == -1){
      for(index = Channels.GetNextNormal(-1); (channel = Channels.Get(index)); index = Channels.GetNextNormal(index)){
        list.push_back(*channel->GetChannelID().ToString());
      }
    } else {
      string u = GetContainerName(uri);
      if((index = GetGroupByName(u)) != -1){
        while((channel = Channels.Get(++index)) != NULL){
          if(channel->GroupSep()){
            // We reached the next group. So, stop here.
            if(*channel->Name())
              break;
          } else {
            list.push_back(*channel->GetChannelID().ToString());
          }
        }
      } else {
        for(index = Channels.GetNextGroup(-1); (channel = Channels.Get(index)); index = Channels.GetNextGroup(index)){
          string group = string(channel->Name()) + '/';
          cerr << group << endl;
          list.push_back(group);
        }
      }
    }

    return list;
  }

  virtual bool IsContainer(const string& uri){
    return uri.compare(GetRootContainer()) == 0 || GetGroupByName(GetContainerName(uri)) != -1;
  }

  virtual bool IsLink(const string& uri, string& target){
    // TODO: what are Channel::RefChannel or LinkChannels ?
    return false;
  }

  virtual long GetContainerUpdateId(const string& uri){
    if(!IsRootContainer(uri)) return 0;

    // We now have containers. However, they do not support containerUpdateIDs separately.
    return (long)lastModified;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(!IsRootContainer(uri)) return false;

    if(!cUPnPResourceProvider::GetMetadata(uri, metadata)) return false;

    int index = 0;
    cChannel* channel;
    if(uri.compare(GetRootContainer()) == 0){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string("VDR Live-TV")));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string("Watch Live-TV")));
    } else if((index = GetGroupByName(GetContainerName(uri))) != -1 && (channel = Channels.Get(index)) != NULL){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(channel->Name())));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string(channel->Name())));
    } else {
      return false;
    }

    metadata.SetProperty(cMetadata::Property("dlna:containerType", string("Tuner_1_0")));
    struct passwd *pw;
    if((pw = getpwuid(getuid())) == NULL){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, string("Klaus Schmidinger")));
    } else {
      string name(pw->pw_gecos); name = name.substr(0,name.find(",,,",0));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_CREATOR, name));
    }

    return true;
  }

  virtual string GetHTTPUri(const string& uri, const string& currentIP, const string& pInfo){
    if(!IsRootContainer(uri)) return string();

    int port = 3000;

    stringstream ss;

    string protocolInfo = pInfo.substr(pInfo.find_last_of(':'));

    std::replace(protocolInfo.begin(), protocolInfo.end(), ';','+');

    ss << "http://" << currentIP << ":" << port
       << "/"
       << "EXT;"
       << "PROG=cat;"
       << "DLNA_contentFeatures=" << protocolInfo
       << "/"
       << uri.substr(uri.find_last_of('/')+1);

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

