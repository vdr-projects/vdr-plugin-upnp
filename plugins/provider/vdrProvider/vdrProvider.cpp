/*
 * vdrProvider.cpp
 *
 *  Created on: 01.10.2012
 *      Author: savop
 */

#include <plugin.h>
#include <vdr/channels.h>
#include <vdr/tools.h>
#include <string>

namespace upnp {

#define GROUP_CHANNELS

class VdrProvider : cUPnPResourceProvider {

  virtual string ProvidesSchema(){ return "vdr"; }

  virtual string GetRootContainer(){
    return ProvidesSchema() + "://";
  }

  virtual EntryList GetContainerEntries(string uri){
    if(uri.find(GetRootContainer(), 0) != 0){
      isyslog("VdrProvider\tUri does not contain the root.");
      return EntryList;
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

  virtual bool IsContainer(string uri){
    return uri.compare(GetRootContainer()) == 0;
  }

  virtual bool IsLink(string uri, string& target){
    // TODO: what are Channel::RefChannel or LinkChannels ?
    return false;
  }

  virtual long GetContainerUpdateId(string uri){
    // TODO: provide a container update id
    return 0;
  }

  virtual cMetadata GetMetadata(string uri);

  virtual string GetHTTPUri(string uri){
    // TODO: get streamdev settings from configuration
    return string();
  }

};

UPNP_REGISTER_RESOURCE_PROVIDER(VdrProvider);

}  // namespace upnp

