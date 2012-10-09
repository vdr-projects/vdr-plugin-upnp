/*
 * vdrProvider.cpp
 *
 *  Created on: 01.10.2012
 *      Author: savop
 */

#include <plugin.h>
#include <vdr/channels.h>
#include <vdr/tools.h>
#include <vdr/config.h>
#include <string>
#include <sstream>
#include <tools.h>
#include <vdr/thread.h>

using namespace std;

namespace upnp {

class VdrProvider : public cUPnPResourceProvider, cThread {
private:
  int     lastUpdateID;

public:

  VdrProvider()
  : lastUpdateID(0)
  {}

  virtual string ProvidesSchema(){ return "vdr"; }

  virtual string GetRootContainer(){
    return ProvidesSchema() + "://";
  }

  virtual cUPnPResourceProvider::EntryList GetContainerEntries(const string& uri){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("VdrProvider\tUri does not contain the root.");
      return cUPnPResourceProvider::EntryList();
    }

    EntryList list;

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

  virtual long GetContainerUpdateId(const string&){
    // TODO: provide a container update id
    return lastUpdateID;
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("VdrProvider\tUri does not contain the root.");
      return false;
    }

    return false;
  }

  virtual string GetHTTPUri(const string& uri, const string& currentIP){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("VdrProvider\tUri does not contain the root.");
      return string();
    }

    int port = 3000;

    stringstream ss;

    ss << "http://" << currentIP << ":" << port << "/TS/" << uri.substr(6);

    return ss.str();
  }

  virtual void Action(){

  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(VdrProvider);

}  // namespace upnp

