/*
 * File:   common.cpp
 * Author: savop
 *
 * Created on 17. April 2009, 20:53
 */

#include <stdarg.h>
#include "common.h"

DLNAProfile DLNA_PROFILE_MPEG2_TS_SD_EU = { "MPEG2_TS_SD_EU", "video/mpeg" };
DLNAProfile DLNA_PROFILE_AVC_TS_HD_EU = { "AVC_TS_HD_EU", "video/vnd.dlna.mpeg-tts" };
DLNAProfile DLNA_PROFILE_MPEG1_L3 = { "MP3", "audio/mpeg" };

DLNAIconProfile DLNA_ICON_JPEG_SM_24 = { "image/jpeg", 48, 48, 24 };
DLNAIconProfile DLNA_ICON_JPEG_LRG_24 = { "image/jpeg", 120, 120, 24 };
DLNAIconProfile DLNA_ICON_PNG_SM_24A = { "image/png", 48, 48, 24 };
DLNAIconProfile DLNA_ICON_PNG_LRG_24A = { "image/png", 120, 120, 24 };

#define MESSAGE_SIZE 256

void message(const char* File, int Line, const char* Format, ...){
    va_list ap;
    char Message[MESSAGE_SIZE];
    snprintf(Message, MESSAGE_SIZE, "(%s:%d) %s\n", File, Line, Format);
    va_start(ap, Format);
    vprintf(Message, ap);
    va_end(ap);
}