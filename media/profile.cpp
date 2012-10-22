/*
 * profile.cpp
 *
 *  Created on: 05.08.2012
 *      Author: savop
 */

#include "../include/media/profile.h"
#include <sstream>
#include <bitset>

using namespace upnp;

ProtocolInfo::ProtocolInfo()
: streamType(DLNA_STREAM_HTTP)
, contentType(string())
, fourthField(DLNA4thField())
{
}

ProtocolInfo::ProtocolInfo(string ct, DLNA4thField ffld, StreamType t)
: streamType(t)
, contentType(ct)
, fourthField(ffld)
{
}

string ProtocolInfo::ToString(){
  if(contentType.empty()) return string();

  stringstream ss;
  switch(streamType){
  case DLNA_STREAM_HTTP:
    ss << "http-get:";
    break;
  case DLNA_STREAM_RTP:
    ss << "rtsp-rtp-udp:";
    break;
  }

  ss << "*:";

  ss << contentType << ":";

  ss << fourthField.ToString();

  return ss.str();
}

DLNA4thField::DLNA4thField()
: profile(string())
, operations(DLNA_OPERATION_NONE)
, playSpeeds(DLNA_PLAYSPEEDS_NONE)
, conversionIndicator(DLNA_CONVERSION_NONE)
, primaryFlags(DLNA_FLAG_NONE)
{
}

DLNA4thField::DLNA4thField(string pn, uint8_t op, string ps, bool ci, uint32_t flags)
: profile(pn)
, operations(op)
, playSpeeds(ps)
, conversionIndicator(ci)
, primaryFlags(flags)
{
}

string DLNA4thField::ToString(){
  if(profile.empty()) return "*";

  stringstream ss;
  ss << "DLNA.ORG_PN=" << profile;

  if(primaryFlags){
    ss << ";";

#ifdef DLNA_STRICT
    // If the any of the flags lop-npt, lop-bytes or lop-cleartextbytes are set
    // the OP param must be omitted.
    if( !(primaryFlags & DLNA_FLAG_BYTE_BASED_SEEK) &&
        !(primaryFlags & DLNA_FLAG_TIME_BASED_SEEK) &&
        !(primaryFlags & DLNA_FLAG_CLEARTEXT_LIMITED_SEEK)){
#endif
      ss << "DLNA.ORG_OP=" << bitset<2>(operations) << ";";
#ifdef DLNA_STRICT
    }
#endif

    ss << "DLNA.ORG_CI=" << bitset<1>(conversionIndicator) << ";";

    ss << "DLNA.ORG_FLAGS=" << hex << primaryFlags << "000000000000000000000000";
  }

  return ss.str();
}

image::cIcon image::DLNA_ICON_PNG_SM_24A = { "image/png", 48, 48, 24 };
image::cIcon image::DLNA_ICON_PNG_LRG_24A = { "image/png", 120, 120, 24 };

image::cIcon image::DLNA_ICON_JPEG_SM_24 = { "image/jpeg", 48, 48, 24 };
image::cIcon image::DLNA_ICON_JPEG_LRG_24 = { "image/jpeg", 120, 120, 24 };


