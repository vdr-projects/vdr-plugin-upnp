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
  stringstream ss;

  if(profile.empty()) return "*";

  ss << "DLNA.ORG_PN=" << profile << ";";

  ss << "DLNA.ORG_OP=" << bitset<2>(operations) << ";";

  ss << "DLNA.ORG_CI=" << bitset<1>(conversionIndicator) << ";";

  ss << "DLNA.ORG_FLAGS=" << hex << primaryFlags << "000000000000000000000000" << ";";

  return ss.str();
}

image::cIcon image::DLNA_ICON_PNG_SM_24A = { "image/png", 48, 48, 24 };
image::cIcon image::DLNA_ICON_PNG_LRG_24A = { "image/png", 120, 120, 24 };

image::cIcon image::DLNA_ICON_JPEG_SM_24 = { "image/jpeg", 48, 48, 24 };
image::cIcon image::DLNA_ICON_JPEG_LRG_24 = { "image/jpeg", 120, 120, 24 };


