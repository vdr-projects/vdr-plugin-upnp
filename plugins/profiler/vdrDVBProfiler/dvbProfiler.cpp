/*
 * dvbProvider.cpp
 *
 *  Created on: 12.10.2012
 *      Author: savop
 */

#include <tools/codec.h>
#include <server.h>
#include <webserver.h>
#include <vdr/recording.h>
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/tools.h>
#include <plugin.h>
#include <tools.h>
#include <string>
#include <sstream>
#include <media/profile.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

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
    cRecording* recording = Recordings.GetByName(uri.substr(6).c_str());

    if(!recording) return false;

    string parentUri = recording->FileName();

    const cRecordingInfo* info = recording->Info();

    metadata.SetObjectIDByUri(uri);
    metadata.SetParentIDByUri(parentUri);
    metadata.SetProperty(cMetadata::Property(property::object::KEY_CLASS, string("object.item.videoItem.videoBroadcast")));
    metadata.SetProperty(cMetadata::Property(property::object::KEY_RESTRICTED, true));

    metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(info->Title())));
    metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string(info->ShortText())));
    metadata.SetProperty(cMetadata::Property(property::object::KEY_LONG_DESCRIPTION, string(info->Description())));

    boost::posix_time::ptime date = boost::posix_time::from_time_t(info->GetEvent()->StartTime());
    metadata.SetProperty(cMetadata::Property(property::object::KEY_DATE, boost::gregorian::to_iso_extended_string(date.date())));

//    cMetadata::Resource resource;
//
//    codec::cFormatContext formatContext;
//
//    if(formatContext.Open(recording->FileName())){
//
//    }
//
//    metadata.AddResource(resource);

    return true;
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

	  // TODO: implement check for radio stations (this is ...audioItem.audioBroadcast)
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_CLASS, string("object.item.videoItem.videoBroadcast")));
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_RESTRICTED, true));
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_CHANNEL_NAME, string(channel->Name())));
	  metadata.SetProperty(cMetadata::Property(property::object::KEY_CHANNEL_NR, (long int)channel->Number()));

	  // Now, we try to get the present event of the schedule
	  {
	    cSchedulesLock lock;
	    const cSchedules* schedules = cSchedules::Schedules(lock);
	    const cSchedule* schedule = (schedules) ? schedules->GetSchedule(channelID) : NULL;
	    const cEvent* event = (schedule) ? schedule->GetPresentEvent() : NULL;

	    if(event){
	      stringstream title;
	      title << channel->Name() << ": " << event->Title();
	      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, title.str()));

	      boost::posix_time::ptime startTime, endTime;
	      startTime = boost::posix_time::from_time_t(event->StartTime());
	      endTime = boost::posix_time::from_time_t(event->EndTime());

	      metadata.SetProperty(cMetadata::Property(property::object::KEY_DATE, boost::gregorian::to_iso_extended_string(startTime.date())));
	      metadata.SetProperty(cMetadata::Property(property::object::KEY_SCHEDULED_START, boost::posix_time::to_iso_extended_string(startTime)));
	      metadata.SetProperty(cMetadata::Property(property::object::KEY_SCHEDULED_END, boost::posix_time::to_iso_extended_string(endTime)));
	      metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string(event->ShortText()?event->ShortText():"")));
	      metadata.SetProperty(cMetadata::Property(property::object::KEY_LONG_DESCRIPTION, string(event->Description()?event->Description():"")));
	    } else {
	      metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(channel->Name())));
	    }

      cMetadata::Resource resource;

      resource.SetResourceUri(uri);

      // TODO: implement check for radio stations
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

      resource.SetProtocolInfo(ProtocolInfo("video/mpeg", fourthfield).ToString());

      if(event){
        boost::posix_time::time_duration duration = boost::posix_time::seconds(event->Duration());
        resource.SetDuration(boost::posix_time::to_simple_string(duration));
      }

      metadata.AddResource(resource);

    }

	  stringstream ss;
	  cMetadata::Resource thumbnail;
	  ss.str(string());
	  ss << "channelIcons/" << channel->Name() << ".jpg";

	  stringstream filename, uriStrm;
	  filename << cMediaServer::GetInstance()->GetWebserver().GetThumbnailDir() << ss.str();
	  uriStrm << "thumb://" << ss.str();

	  struct stat fileStat;

	  if(stat(filename.str().c_str(), &fileStat) == 0){
	    thumbnail.SetResourceUri(uriStrm.str());
	    thumbnail.SetProtocolInfo(ProtocolInfo("image/jpeg", DLNA4thField("JPEG_TN")).ToString());
	    thumbnail.SetSize(fileStat.st_size);
	    metadata.AddResource(thumbnail);
	  }

    return true;
  }
};

UPNP_REGISTER_MEDIA_PLUGIN(DVBProfiler);

}
