/*
 * vdrProvider.cpp
 *
 *  Created on: 01.10.2012
 *      Author: savop
 */

#include <plugin.h>
#include <server.h>
#include <vdr/epg.h>
#include <vdr/channels.h>
#include <vdr/tools.h>
#include <vdr/plugin.h>
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
  time_t      lastModified;
  int         from;
  int         to;

  bool Parse(const string& line)
  {
    int pos = 0;
    if((pos = line.find_first_of('-')) != string::npos){
      from = atoi(line.substr(0, pos).c_str());
      to = atoi(line.substr(pos+1).c_str());
      return true;
    }
    return false;
  }

  int GetGroupByName(const string name)
  {
    if(name.empty()) return -1;
    int Idx = -1;
    cChannel *group = Channels.Get(++Idx);
    while (group && !(group->GroupSep() && name.compare(group->Name()) == 0))
      group = Channels.Get(++Idx);
    return group ? Idx : -1;
  }

  int GetGroupByChannel(const cChannel* channel)
  {
    if(!channel) return -1;
    int Idx = -1;
    cChannel* group = Channels.Get(++Idx);
    while(group && !(group->GroupSep() && group->Number() > channel->Number()))
      group = Channels.Get(++Idx);
    return group ? Idx : -1;

  }

  string GetContainerName(string uri){
    return uri.substr(6,uri.size()-7);
  }

  cCondWait sleep;

  cCharSetConv conv;
#define TO_UTF8(from, to, length) \
  char to[length]; conv.Convert(from, to, length);

public:

  VdrProvider()
  : lastModified(0)
  , from(0)
  , to(INT_MAX)
  {
    LoadConfigFile("vdrProvider.conf");
  }

  virtual ~VdrProvider(){
    sleep.Signal();
    Cancel(5);
  }

  virtual string ProvidesSchema(){ return "vdr"; }

  virtual string GetRootContainer(){
    return ProvidesSchema() + "://";
  }

  virtual StringList GetContainerEntries(const string& uri){
    if(!HasRootContainer(uri)) return StringList();

    StringList list;
    int index;
    cChannel* channel = NULL;
    if(to == 0) to = INT_MAX;
    // Check if this is the root and we have no groups in the channels.conf:
    if(uri.compare(GetRootContainer()) == 0 && Channels.GetNextGroup(0) == -1){
      for(index = Channels.GetNextNormal(from - 1); (channel = Channels.Get(index)) && index < to; index = Channels.GetNextNormal(index)){
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
        for(index = Channels.GetNextGroup(from - 1); (channel = Channels.Get(index)) && index < to; index = Channels.GetNextGroup(index)){
          TO_UTF8(channel->Name(), chanName, 1024);
          string group = string(chanName) + '/';
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
    if(!HasRootContainer(uri)) return 0;

    // We now have containers. However, they do not support containerUpdateIDs separately.
    return (long)lastModified;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(!HasRootContainer(uri)) return false;

    if(!cUPnPResourceProvider::GetMetadata(uri, metadata)) return false;

    int index = 0;
    cChannel* channel;
    if(uri.compare(GetRootContainer()) == 0){
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string("VDR Live-TV")));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string("Watch Live-TV")));
    } else if((index = GetGroupByName(GetContainerName(uri))) != -1 && (channel = Channels.Get(index)) != NULL){
      TO_UTF8(channel->Name(), chanName, 1024);
      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(chanName)));
      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string(chanName)));
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
    if(!HasRootContainer(uri)) return string();

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

#define SLEEP_TIMEOUT 120

    const cSchedules* Schedules;
    cSchedule* Schedule;

    const cEvent* event;

    long now = 0;
    bool modified = false;

    while(Running()){

      event = NULL;
      now = time(NULL);

      if(!Channels.BeingEdited() && Channels.Modified() > 0){
        modified = true;
      }

      { // Reduce Scope of Schedules lock.
        cSchedulesLock lock;
        Schedules = cSchedules::Schedules(lock);
        // iterate over all the schedules, find those, which were modified and tell
        // it to the media manager. If we found an entry, there's no need to continue searching.
        for(Schedule = Schedules->First(); Schedule; Schedule = Schedules->Next(Schedule))
        {
          // Get the next event of the schedule...
          event = Schedule->GetFollowingEvent();
          if(event){
            // and check if it starts after the last modification and in two minutes from now.
            // This causes an update only if there is at least one element in the schedule
            // which starts within the next two minutes. Other elements, which will start later,
            // will be skipped.
            if(event->StartTime() > lastModified && event->StartTime() < now + SLEEP_TIMEOUT){
              modified = true;
              break;
            }
          } else if(Schedule->Modified() > lastModified){
            modified = true;
            break;
          }
        }

      }

      if(modified){
        OnContainerUpdate(GetRootContainer(), now);
        modified = false;
        lastModified = now;
      }

      // Sleep 2 minutes
      sleep.Wait(SLEEP_TIMEOUT * 1000);
    }

  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(VdrProvider);

}  // namespace upnp

