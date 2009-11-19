/*
 * File:   common.cpp
 * Author: savop
 *
 * Created on 17. April 2009, 20:53
 */

#include <stdarg.h>
#include "common.h"
#include "misc/config.h"

DLNAProfile DLNA_PROFILE_MPEG_TS_SD_EU = { "MPEG_TS_SD_EU", "video/vnd.dlna.mpeg-tts" };
DLNAProfile DLNA_PROFILE_AVC_TS_HD_EU = { "AVC_TS_HD_EU", "video/vnd.dlna.mpeg-tts" };
DLNAProfile DLNA_PROFILE_MPEG_TS_SD_EU_ISO = { "MPEG_TS_SD_EU_ISO", "video/mpeg" };
DLNAProfile DLNA_PROFILE_AVC_TS_HD_EU_ISO = { "AVC_TS_HD_EU_ISO", "video/mpeg" };
DLNAProfile DLNA_PROFILE_MPEG1_L3 = { "MP3", "audio/mpeg" };

DLNAIconProfile DLNA_ICON_JPEG_SM_24 = { "image/jpeg", 48, 48, 24 };
DLNAIconProfile DLNA_ICON_JPEG_LRG_24 = { "image/jpeg", 120, 120, 24 };
DLNAIconProfile DLNA_ICON_PNG_SM_24A = { "image/png", 48, 48, 24 };
DLNAIconProfile DLNA_ICON_PNG_LRG_24A = { "image/png", 120, 120, 24 };

#define MESSAGE_SIZE 256

void message(int Level, const char* , int , const char* Format, ...){
    if(Level && cUPnPConfig::verbosity >= Level){
        va_list ap;
        char Message[MESSAGE_SIZE];

        snprintf(Message, sizeof(Message), "[%d] %s", cThread::ThreadId(), Format);
        va_start(ap, Format);
        vsyslog(LOG_NOTICE, Message, ap);
        va_end(ap);
    }
}