/*
 * dvbProvider.cpp
 *
 *  Created on: 12.10.2012
 *      Author: savop
 */

#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/tools.h>
#include <plugin.h>
#include <tools.h>
#include <string>
#include <sstream>

using namespace std;

namespace upnp {

class DVBProfiler : public cMediaProfiler {
public:
  virtual bool CanHandleSchema(const string& schema){
    if(schema.find("vdr",0) == 0 || schema.find("rec",0) == 0){
      return true;
    } else {
      return false;
    }
  }

  virtual bool GetMetadata(const string& uri, cMetadata& metadata){
    if        (uri.find("vdr",0) == 0){
      return GetChannelMetadata(uri, metadata);
    } else if (uri.find("rec",0) == 0){
      return GetRecordingMetadata(uri, metadata);
    } else {
      return false;
    }
  }

private:

  virtual bool GetRecordingMetadata(const string& uri, cMetadata& metadata){
    dsyslog("DvbProfiler\tGetting recording metadata for '%s'", uri.c_str());
    return false;
  }

  virtual bool GetChannelMetadata(const string& uri, cMetadata& metadata){
    dsyslog("DvbProfiler\tGetting channel metadata for '%s'", uri.c_str());
    return false;
  }
};

UPNP_REGISTER_MEDIA_PLUGIN(DVBProfiler);

}
