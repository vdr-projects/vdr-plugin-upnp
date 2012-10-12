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
#include <media/profile.h>

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
    //TODO
    return false;
  }

  virtual bool GetChannelMetadata(const string& uri, cMetadata& metadata){

	  tChannelID channelID = tChannelID::FromString(uri.substr(6).c_str());
	  if(!channelID.Valid()) return false;

	  cChannel* channel = Channels.GetByChannelID(channelID);
	  if(channel == NULL) return false;

	  // First of all we can set the default information.

	  string parentUri = uri.substr(0, uri.find_last_of('/')+1);

	  metadata.SetObjectIDByUri(uri);
	  metadata.SetParentIDByUri(parentUri);
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_CLASS, string("object.item.videoItem.videoBroadcast")));
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_RESTRICTED, true));
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_CHANNEL_NAME, string(channel->Name())));
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_CHANNEL_NR, (long int)channel->Number()));

	  // Now, we try to get the present event of the schedule
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(channel->Name())));

	  cMetadata::Resource resource;

	  stringstream protocolInfo;

	  protocolInfo << "http-get:*:video/mpeg:";

    DLNA4thField fourthfield;

	  switch (channel->Vtype()) {
      case 0x02:
        fourthfield = DLNA4thField("MPEG_TS_SD_EU_ISO", DLNA_OPERATION_NONE,
                                   DLNA_PLAYSPEEDS_NONE, DLNA_CONVERSION_NONE,
                                   DLNA_FLAG_STREAMING_TRANSFER |
                                   DLNA_FLAG_SN_INCREASE |
                                   DLNA_FLAG_VERSION_1_5 );
        break;
      case 0x1B:
        fourthfield = DLNA4thField("AVC_TS_HD_EU_ISO", DLNA_OPERATION_NONE,
                                   DLNA_PLAYSPEEDS_NONE, DLNA_CONVERSION_NONE,
                                   DLNA_FLAG_STREAMING_TRANSFER |
                                   DLNA_FLAG_SN_INCREASE |
                                   DLNA_FLAG_VERSION_1_5 );
        break;
      default:
        return false;
        break;
    }

	  protocolInfo << fourthfield.ToString();

	  resource.SetResourceUri(uri);
	  resource.SetProtocolInfo(protocolInfo.str());

	  metadata.AddResource(resource);

    return true;
  }
};

UPNP_REGISTER_MEDIA_PLUGIN(DVBProfiler);

}
