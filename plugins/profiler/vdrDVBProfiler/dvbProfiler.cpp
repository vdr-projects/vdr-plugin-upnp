/*
 * dvbProvider.cpp
 *
 *  Created on: 12.10.2012
 *      Author: savop
 */

#include <server.h>
#include <webserver.h>
#include <vdr/videodir.h>
#include <vdr/recording.h>
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <vdr/tools.h>
#include <vdr/remux.h>
#include <plugin.h>
#include <tools.h>
#include <string>
#include <sstream>
#include <media/profile.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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

  bool GetRecordingMetadata(const string& u, cMetadata& metadata){
    string videoDir = string(VideoDirectory), uri = u.substr(6), recStr = videoDir + "/" + uri;
    cRecording* recording = Recordings.GetByName(recStr.c_str());

    if(!recording) return false;

    // Sorry, currently unsupported.
    if(recording->IsPesRecording()) return false;

    char* fileBuf = strdup(recording->Name());
    fileBuf = ExchangeChars(fileBuf, true);
    string fs = fileBuf;
    free(fileBuf);

    int pos = 0;
    if((pos = fs.find_last_of('/')) != string::npos){
      fs = fs.substr(0,pos+1);
    } else {
      fs = "";
    }
    fs = u.substr(0,6) + fs;

    const cRecordingInfo* info = recording->Info();

    metadata.SetObjectIDByUri(uri);
    metadata.SetParentIDByUri(fs);
    metadata.SetProperty(cMetadata::Property(property::object::KEY_RESTRICTED, true));

    metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, string(info->Title()?info->Title():recording->Title())));
    metadata.SetProperty(cMetadata::Property(property::object::KEY_DESCRIPTION, string(info->ShortText()?info->ShortText():"")));
    metadata.SetProperty(cMetadata::Property(property::object::KEY_LONG_DESCRIPTION, string(info->Description()?info->Description():"")));

    boost::posix_time::ptime date = boost::posix_time::from_time_t(info->GetEvent()->StartTime());
    metadata.SetProperty(cMetadata::Property(property::object::KEY_DATE, boost::gregorian::to_iso_extended_string(date.date())));

    cMetadata::Resource resource;

    char filename[1024]; strncpy(filename, recording->FileName(), 1024);
    char* pFileNumber = filename + strlen(filename);
    sprintf(pFileNumber, "/%05d.ts", 1);
    FILE *fd = fopen(filename, "r");
    if(!fd) return false;

    int pmtV, patV, pid;
    unsigned char buf[TS_SIZE];
    cPatPmtParser parser;
    while(fread(buf, 1, TS_SIZE, fd) == TS_SIZE){
      if(buf[0] == TS_SYNC_BYTE){
        pid = TsPid(buf);
        if(pid == 0){
          parser.ParsePat(buf, TS_SIZE);
        } else if (pid == parser.PmtPid()) {
          parser.ParsePmt(buf, TS_SIZE);
          if(parser.GetVersions(patV, pmtV))
            break;
        }
      }
    }
    fclose(fd);

    int i = 1;
    size_t size = 0;

    sprintf(pFileNumber, "/%05d.ts", i);
    while(access(filename, F_OK) == 0){
      fd = fopen(filename, "r");
      fseek(fd, 0, SEEK_END);
      size += ftell(fd);
      sprintf(pFileNumber, "/%05d.ts", ++i);
      fclose(fd);
    }

    // TODO: implement check for radio stations
    DLNA4thField fourthfield;
    string contentType, upnpclass;
    if(parser.Vtype() != 0){
      switch (parser.Vtype()) {
        case 0x02:
          fourthfield = DLNA4thField("MPEG_TS_SD_EU_ISO", DLNA_OPERATION_RANGE,
                                     DLNA_PLAYSPEEDS_NONE, DLNA_CONVERSION_NONE,
                                     DLNA_FLAG_STREAMING_TRANSFER |
                                     DLNA_FLAG_BYTE_BASED_SEEK |
                                     DLNA_FLAG_BACKGROUND_TRANSFER |
                                     DLNA_FLAG_CONNECTION_STALLING |
                                     DLNA_FLAG_VERSION_1_5 );
          break;
        case 0x1B:
          fourthfield = DLNA4thField("AVC_TS_HD_EU_ISO", DLNA_OPERATION_RANGE,
                                     DLNA_PLAYSPEEDS_NONE, DLNA_CONVERSION_NONE,
                                     DLNA_FLAG_STREAMING_TRANSFER |
                                     DLNA_FLAG_BYTE_BASED_SEEK |
                                     DLNA_FLAG_BACKGROUND_TRANSFER |
                                     DLNA_FLAG_CONNECTION_STALLING |
                                     DLNA_FLAG_VERSION_1_5 );
          break;
        default:
          return false;
      }
      contentType = "video/mpeg";
      upnpclass = "object.item.videoItem.videoBroadcast";
    } else {
      int Atype = 0;
      for(int i = 0; (Atype = parser.Atype(i)) != 0; ++i){
        switch(Atype){
        case 0x03:
        case 0x04:
#ifdef DLNA_STRICT
          fourthfield = DLNA4thField("MP2_MPS", DLNA_OPERATION_RANGE,
                                     DLNA_PLAYSPEEDS_NONE, DLNA_CONVERSION_NONE,
                                     DLNA_FLAG_STREAMING_TRANSFER |
                                     DLNA_FLAG_BYTE_BASED_SEEK |
                                     DLNA_FLAG_BACKGROUND_TRANSFER |
                                     DLNA_FLAG_CONNECTION_STALLING |
                                     DLNA_FLAG_VERSION_1_5 );
          contentType = "audio/mpeg";
          upnpclass = "object.item.audioItem.audioBroadcast";
#else
          fourthfield = DLNA4thField("MPEG_TS_SD_EU_ISO", DLNA_OPERATION_RANGE,
                                     DLNA_PLAYSPEEDS_NONE, DLNA_CONVERSION_NONE,
                                     DLNA_FLAG_STREAMING_TRANSFER |
                                     DLNA_FLAG_BYTE_BASED_SEEK |
                                     DLNA_FLAG_BACKGROUND_TRANSFER |
                                     DLNA_FLAG_CONNECTION_STALLING |
                                     DLNA_FLAG_VERSION_1_5 );
          contentType = "video/mpeg";
          upnpclass = "object.item.videoItem.videoBroadcast";
#endif
          goto validProfile;
        default:
          break;
        }
      }

      // No compatible audio codec found.
      return false;
    }
    // Found a valid profile
    validProfile:

    metadata.SetProperty(cMetadata::Property(property::object::KEY_CLASS, upnpclass));
    resource.SetProtocolInfo(ProtocolInfo(contentType, fourthfield).ToString());
    resource.SetSize(size);
    resource.SetResourceUri(u);

    int seconds = 0;
    const cEvent* event = info->GetEvent();
    if(event && event->Duration() > 0){
      seconds = event->Duration();
    }
#if VDRVERSNUM > 10723
    else if(recording->LengthInSeconds() > 0){
      seconds = recording->LengthInSeconds();
    }
#endif

    if(seconds){
      boost::posix_time::time_duration duration = boost::posix_time::seconds(seconds);
      resource.SetDuration(boost::posix_time::to_simple_string(duration));
    }

    metadata.AddResource(resource);

    return true;
  }

  bool GetChannelMetadata(const string& uri, cMetadata& metadata){

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
